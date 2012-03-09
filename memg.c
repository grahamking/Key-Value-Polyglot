
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

#define READ_SIZE 1024

// Arguments to our thread
typedef struct {
    int conn;
    struct hsearch_data *htab;
} thdata;

/*
 * Buffered IO on socket
 */

struct Buf {
    int conn;
    char *store;
    int start;
    int end;
};

struct Buf *new_buffer(int conn) {
    struct Buf *buf = malloc(sizeof(struct Buf));
    buf->conn = conn;
    buf->store = malloc(READ_SIZE + 1); // +1 for the \0
    return buf;
}

void buf_fill(struct Buf *buf) {
    memset(buf->store, '\0', READ_SIZE + 1);
    recv(buf->conn, buf->store, READ_SIZE, 0);

    //printf("* Buffer is now: '%s' (%d)\n", buf->store, strlen(buf->store));
}

/* Fetch until \n */
char *buf_line(struct Buf *buf) {

    size_t str_end;
    char *line;

    if (strlen(buf->store) == 0) {
        buf_fill(buf);
    }

    str_end = strcspn(buf->store, "\n");
    line = strndup(buf->store, str_end - 1);    // -1 because of \r

    memmove(buf->store, buf->store + str_end + 1, READ_SIZE - str_end);

    //printf("buf_line Found line: %s (%d)\n", line, strlen(line));
    //printf("buf_line Remain: %s (%d)\n", buf->store, strlen(buf->store));
    return line;
}

/* Fetch exactly 'num' bytes */
char *buf_bytes(struct Buf *buf, int num) {

    char *line;

    if (strlen(buf->store) == 0) {
        buf_fill(buf);
    }

    line = strndup(buf->store, num);
    memmove(buf->store, buf->store + num + 2, READ_SIZE - num - 1);

    //printf("buf_bytes Found line: %s (%d)\n", line, strlen(line));
    //printf("buf_bytes Remain: %s (%d)\n", buf->store, strlen(buf->store));
    return line;
}

/*
 * Connection handling. Implements 'get' and 'set'.
 */
void *handle_conn(void *void_param) {//int conn, struct hsearch_data *htab) {

    thdata *param = (thdata *)void_param;
    int conn = param->conn;
    struct hsearch_data *htab = param->htab;

    char *cmd, *key, *val, *msg, *line;
    int length;
    ENTRY entry, *result;
    struct Buf *buf;

    buf = new_buffer(conn);

    while (1) {
        line = buf_line(buf);

        if (strlen(line) == 0) {
            // Client has closed connection
            break;
        }

        cmd = strtok(line, " ");

        if (strcmp(cmd, "get") == 0) {
            key = strtok(NULL, " ");

            entry.key = key;
            hsearch_r(entry, FIND, &result, htab);
            if (result != NULL) {

                val = (char *) result->data;

                msg = malloc(strlen(key) + 13);
                sprintf(msg, "VALUE %s 0 %d\r\n", key, strlen(val));
                send(conn, msg, strlen(msg), 0);

                send(conn, val, strlen(val), 0);
                send(conn, "\r\n", 2, 0);
            }
            send(conn, "END\r\n", 5, 0);

        } else if (strcmp(cmd, "set") == 0) {
            key = strtok(NULL, " ");

            strtok(NULL, " ");
            strtok(NULL, " ");
            //exp = strtok(NULL, " ");
            //flags = strtok(NULL, " ");

            length = atoi(strtok(NULL, " "));
            val = buf_bytes(buf, length);

            entry.key = strdup(key);
            entry.data = strdup(val);
            hsearch_r(entry, ENTER, &result, htab);

            send(conn, "STORED\r\n", 8, 0);
        }
    }

    return NULL;
}

// Is --single in argv?
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
    thdata *thread_data = malloc(sizeof(thdata));

    // Create hashmap storage
    htab = calloc(1, sizeof(struct hsearch_data));
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
    addr = malloc(sizeof(struct sockaddr_in));
    memset(addr, 0, sizeof(struct sockaddr_in));    // Fill with zeros
    addr->sin_family = AF_INET;
    addr->sin_port = htons(11211);  // htons: Convert to network byte order
    addr->sin_addr.s_addr = INADDR_ANY;
    err = bind(sock_fd, (struct sockaddr *)addr, sizeof(struct sockaddr));
    if (err == -1) {
        printf("bind error: %d\n", errno);
    }

    err = listen(sock_fd, 1);
    if (err == -1) {
        printf("listen error: %d\n", errno);
    }

    if (is_single(argc, argv)) {

        client_addr = malloc(sizeof(struct sockaddr_in));
        client_addr_size = sizeof(struct sockaddr);
        conn = accept(sock_fd, (struct sockaddr *)client_addr, &client_addr_size);

        thread_data->conn = conn;
        thread_data->htab = htab;
        handle_conn(thread_data);
        close(conn);

    } else {
        while (1) {

            client_addr = malloc(sizeof(struct sockaddr_in));
            client_addr_size = sizeof(struct sockaddr);
            conn = accept(sock_fd, (struct sockaddr *)client_addr, &client_addr_size);

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

    return 0;
}
