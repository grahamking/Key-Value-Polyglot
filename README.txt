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

I also tested against memcached:

$ memcached &
$ python -m timeit "import benchmarklib;benchmarklib.main(1)"
10 loops, best of 3: 110 msec per loop
$ python -m timeit "import benchmarklib;benchmarklib.main(10)"
10 loops, best of 3: 94.1 msec per loop

(wow)

The C version has the same problems as the original Python version.

$ ./memg &
$ python -m timeit -n 1 -r 1 "import benchmarklib;benchmarklib.main(1)"
1 loops, best of 1: 20.1 sec per loop

The results are summarized in the table below.

Requests/second
---------------
                                       1 client    10 clients
                                        Req/sec     Req/sec
C                                         50
threaded, using socket.sendall            50
threaded, using sockfile.write/flush       7 k         3 k
memg.go                                    9 k        11 k
gevent                                    17 k        22 k
diesel                                    11 k        14 k
memcached                                 10 k       100 k


Thoughts
--------

* The original C and Python implementations are simply flawed
* Python's async frameworks outperforms go. This is (to me) surprising.
* The threaded Python program does not scale.
* memcached is amazing