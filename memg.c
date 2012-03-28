#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <search.h>
#include <pthread.h>

/* Arguments to our thread */
typedef struct {
    int conn;
    struct hsearch_data *htab;
} thdata;

/* Strip end \r and \n from string */
void strip(char *str) {

    int len = strlen(str);
    while (str[len - 1] == '\r' || str[len - 1] == '\n') {
        str[len - 1] = 0;
        len--;
    }
}

/*
* Connection handling. Implements 'get' and 'set'.
*/
void *handle_conn(void *void_param) {//int conn, struct hsearch_data *htab) {

    thdata *param = (thdata *)void_param;
    int conn = param->conn;
    struct hsearch_data *htab = param->htab;

    char *cmd, *key, *val, *msg, *line;
    int len, curr_len, space;
    ENTRY entry, *result;

    FILE *f = fdopen(conn, "rw");
    line = (char*)malloc(128);

    while (1) {
        memset(line, 0, 128);

        fgets(line, 128, f);

        if (strlen(line) == 0) { // Client has closed connection

            fclose(f);
            close(conn);
            free(line);
            break;
        }

        cmd = strtok(line, " ");

        if (strcmp(cmd, "get") == 0) {

            key = strtok(NULL, " ");
            strip(key);

            entry.key = key;
            hsearch_r(entry, FIND, &result, htab);
            if (result != NULL) {

                val = (char *) result->data;

                msg = (char*)malloc(strlen(key) + strlen(val) + 20);
                sprintf(msg, "VALUE %s 0 %zd\r\n%s\r\nEND\r\n",
                        key, strlen(val), val);
                send(conn, msg, strlen(msg), 0);
                free(msg);

            } else {

                send(conn, "END\r\n", 5, 0);
            }

        } else if (strcmp(cmd, "set") == 0) {

            key = strtok(NULL, " ");

            strtok(NULL, " "); // skip exp
            strtok(NULL, " "); // skip flags

            // Length of string to read
            len = atoi(strtok(NULL, " ")) + 2; // + 2 for \r\n

            // How much space to allocate. + 1 for \0
            space = len + 1;

            val = (char*)calloc(1, space);

            curr_len = 0;
            do {
                fgets(val + curr_len, space - curr_len, f);
                curr_len = strlen(val);
            } while (curr_len < len);

            strip(val);

            entry.key = strdup(key);
            entry.data = strdup(val);
            hsearch_r(entry, ENTER, &result, htab);

            send(conn, "STORED\r\n", 8, 0);
            free(val);
        }
    }

    return NULL;
}

/* Is --single in argv? */
int is_single(int argc, char *argv[]) {

    int i;

    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--single") == 0) {
            return 1;
        }
    }
    return 0;
}

/*
* MAIN
*/
int main(int argc, char *argv[]) {

    int sock_fd;
    int err;
    int optval;
    int conn;
    struct sockaddr_in *addr, *client_addr;
    socklen_t client_addr_size;
    struct hsearch_data *htab;
    pthread_t thread;
    thdata *thread_data = (thdata*)malloc(sizeof(thdata));

    // Create hashmap storage
    htab = (struct hsearch_data*)calloc(1, sizeof(struct hsearch_data));
    if (hcreate_r(10000, htab) == -1) {
        printf("Error on hcreate\n");
    }

    // Create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        printf("Error creating socket: %d\n", errno);
    }

    // Allow address reuse
    optval = 1;
    err = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    if (err == -1) {
        printf("Error setting SO_REUSEADDR on socket: %d\n", errno);
    }

    // bind
    addr = (struct sockaddr_in*)calloc(1, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(11211); // htons: Convert to network byte order
    addr->sin_addr.s_addr = INADDR_ANY;
    err = bind(sock_fd, (struct sockaddr *)addr, sizeof(struct sockaddr));
    free(addr);
    if (err == -1) {
        printf("bind error: %d\n", errno);
    }

    err = listen(sock_fd, 1);
    if (err == -1) {
        printf("listen error: %d\n", errno);
    }

    if (is_single(argc, argv)) {

        client_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
        client_addr_size = sizeof(struct sockaddr);
        conn = accept(sock_fd, (struct sockaddr *)client_addr, &client_addr_size);
        free(client_addr);

        thread_data->conn = conn;
        thread_data->htab = htab;
        handle_conn(thread_data);
        close(conn);

    } else {
        while (1) {

            client_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
            client_addr_size = sizeof(struct sockaddr);
            conn = accept(
                    sock_fd,
                    (struct sockaddr *)client_addr,
                    &client_addr_size);
            free(client_addr);

            thread_data->conn = conn;
            thread_data->htab = htab;
            pthread_create(
                    &thread,
                    NULL,
                    handle_conn,
                    thread_data);
        }
    }

    close(sock_fd);
    free(thread_data);
    hdestroy_r(htab);
    free(htab);

    return 0;
}
