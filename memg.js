
var net = require('net');

/* Are we running in one-shot mode? */
var oneshot = (process.argv[2] === '--single') ? true : false;

/* Create an async network wrapper */
var server = net.createServer(function(sock) {

    "use strict";

    /* Create the cache object */
    var CACHE = {};
    
    /* Handle the connection received on our network socket */
    sock.on('data', function (chunk) {

        /* Upon receiving the socket connection, extract the data */
        var data = chunk.toString();

        /* Start splitting the command string */
        var parts = data.split('\r\n'), tmp = parts[0].split(' ');
        var cmd = tmp[0], key = tmp[1], val;
        var msg = [];

        switch (cmd) {
        
            case "get":
                val = CACHE[key];
                if (val) {
                    msg.push("VALUE ", key, " 0 ", val.length, "\r\n")
                    msg.push(val, "\r\n")
                };
                msg.push("END\r\n");
                break;

            case "set":
                val = parts[1];
                var length = +tmp[4];
                if (val) {
                    CACHE[key] = val.slice(0, length);
                    msg.push("STORED\r\n");
                };
                break;

        };

        sock.write(msg.join(""));

    });

    if (global.oneshot) process.exit(1);

});

server.listen(11211);
