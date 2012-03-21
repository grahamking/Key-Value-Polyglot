Running ./compare_python_implementations on my machine I get the following:

Modified memg.py, using sockfile.write()/.flush()
1 clients
10 loops, best of 3: 148 msec per loop
10 clients
10 loops, best of 3: 3.16 sec per loop
Using gevent
1 clients
10 loops, best of 3: 60 msec per loop
10 clients
10 loops, best of 3: 451 msec per loop
Using diesel
1 clients
10 loops, best of 3: 92.5 msec per loop
10 clients
10 loops, best of 3: 687 msec per loop
The original memg.py, using socket.sendall()
1 clients
1 loops, best of 1: 20.1 sec per loop

Requests/second
---------------
                                       1 client    10 clients
                                        Req/sec     Req/sec
threaded, using sockfile.write/flush     7 k          3 k
threaded, using socket.sendall            50
gevent                                   17 k         22 k
diesel                                   11 k         14 k