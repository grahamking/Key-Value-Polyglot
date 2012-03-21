#!/usr/bin/env python
import socket
import threading
import sys

CACHE = {}


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("127.0.0.1", 11211))
    sock.listen(1)

    if '--single' in sys.argv:
        conn, _ = sock.accept()
        handle_con(conn)
    else:
        while 1:
            conn, _ = sock.accept()
            thread = threading.Thread(target=handle_con, args=(conn,))
            thread.start()


def handle_con(conn):

    try:
        # Disable universal new lines for python 2 compatibility
        sockfile = conn.makefile(newline="")
    except TypeError:
        # python 2
        sockfile = conn.makefile()

    while True:

        line = sockfile.readline()
        if line == "":
            break

        parts = line.split()
        cmd = parts[0]

        if cmd == "get":
            key = parts[1]

            try:
                val = CACHE[key]
                output(conn, "VALUE %s 0 %d\r\n" % (key, len(val)))
                output(conn, val + "\r\n")
            except KeyError:
                pass
            output(conn, "END\r\n")

        elif cmd == "set":
            key = parts[1]
            #exp = parts[2]
            #flags = parts[3]
            length = int(parts[4])
            val = sockfile.read(length + 2)[:length]
            CACHE[key] = val

            output(conn, "STORED\r\n")


def output(conn, string):
    """Actually write to socket"""
    conn.sendall(string.encode("utf8"))


if __name__ == "__main__":
    main()
