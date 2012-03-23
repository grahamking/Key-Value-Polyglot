#!/usr/bin/env python
from gevent.server import StreamServer

CACHE = {}

def handle_con(conn, address):
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
                sockfile.write("VALUE %s 0 %d\r\n" % (key, len(val)))
                sockfile.write(val + "\r\n")
            except KeyError:
                pass
            sockfile.write("END\r\n")
            sockfile.flush()

        elif cmd == "set":
            key = parts[1]
            length = int(parts[4])
            val = sockfile.read(length + 2)[:length]
            CACHE[key] = val

            sockfile.write("STORED\r\n")
            sockfile.flush()


if __name__ == "__main__":
    server = StreamServer(("127.0.0.1", 11211), handle_con)
    server.serve_forever()
