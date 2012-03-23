/* Create the cache object */
var CACHE = {};

var handle_conn = function (sock) {

    "use strict";

    sock.on('data', function(chunk) {

        /* Upon receiving the socket connection, extract the data */
        var data = chunk.toString()

        /* Start splitting the command string */
        var parts = data.split('\r\n');
        var tmp = parts[0].split(' ');
        var cmd = tmp[0], key = tmp[1], val;

        switch(cmd) {
        
            case "get":
                val = CACHE[key];
                if (val) {
                    var msg = "VALUE " + key + " 0 " + val.length + "\r\n";
                    msg += val + "\r\n";
                    sock.write(msg);
                }
                sock.write("END\r\n");
                break;

            case "set":
                var val = parts[1];
                var length = +tmp[4];
                if (val) {
                    CACHE[key] = val.slice(0, length);
                    sock.write("STORED\r\n");
                };
                break;
        };

    });

    if (oneshot) process.exit(1);

};

var net = require('net');

/* Are we running in one-shot mode? */
var oneshot = (process.argv[2] === '--single') ? true : false;

/* Create an async network wrapper */
var server = net.createServer(function(sock) {
    
    /* Handle the connection received on our network socket */
    handle_conn(sock);

});

server.listen(11211);

