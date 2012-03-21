#!/usr/bin/env bash

function timeit() {
    python -m timeit "import benchmarklib;benchmarklib.main($1)"
}

function timeit_single() {
    python -m timeit -n 1 -r 1 "import benchmarklib;benchmarklib.main($1)"
}


function start_server_timeit_and_kill_server() {
    echo $2 clients
    python $1 &
    sleep 1
    if [ "$1" == "memg_original.py" ]
    then
        timeit_single $2
    else
        timeit $2
    fi
    kill %% &> /dev/null
}

echo "Modified memg.py, using sockfile.write()/.flush()"
start_server_timeit_and_kill_server memg.py 1
start_server_timeit_and_kill_server memg.py 10

echo "Using gevent"
start_server_timeit_and_kill_server memg_gevent.py 1
start_server_timeit_and_kill_server memg_gevent.py 10

echo "Using diesel"
start_server_timeit_and_kill_server memg-diesel.py 1
start_server_timeit_and_kill_server memg-diesel.py 10

echo "The original memg.py, using socket.sendall()"
start_server_timeit_and_kill_server memg_original.py 1