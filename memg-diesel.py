#!/usr/bin/env python
from diesel import Service, quickstart, until_eol, send, receive, logmod

CACHE = {}

def handle_con(unused):
    while True:
        line = until_eol()
        if line == "":
            break

        parts = line.split()
        cmd = parts[0]

        if cmd == "get":
            key = parts[1]

            try:
                val = CACHE[key]
                send("VALUE %s 0 %d\r\n" % (key, len(val)))
                send(val + "\r\n")
            except KeyError:
                pass
            send("END\r\n")

        elif cmd == "set":
            key = parts[1]
            #exp = parts[2]
            #flags = parts[3]
            length = int(parts[4])
            val = receive(length + 2)[:length]
            CACHE[key] = val

            send("STORED\r\n")


if __name__ == "__main__":
    logmod.set_log_level(logmod.levels.DISABLED)
    quickstart(Service(handle_con, 11211))
