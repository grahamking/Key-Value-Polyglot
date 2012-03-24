var net = require('net');

/* Create the cache object */
var CACHE = {};
var SINGLE = (process.argv[2] === '--single') ? true : false;

/* Create an async network wrapper */
var server = net.createServer(function(sock) {

    "use strict";

    /* Handle the connection received on our network socket */
    sock.on('data', function (chunk) {

        /* Upon receiving the socket connection, extract the data */
        var data = chunk.toString();

        /* Start splitting the command string */
        var parts = data.split('\r\n'), tmp = parts[0].split(' ');
        var cmd = tmp[0], key = tmp[1], val;
        var msg = [];

        /* Take action according to the command */
        if (cmd === 'get') {

            val = CACHE[key];

            if (val) {
                msg.push("VALUE ", key, " 0 ", val.length, "\r\n")
                msg.push(val, "\r\n")
            };

            msg.push("END\r\n");

        } else if (cmd === 'set') {

            val = parts.slice(1, -1);
            var length = +tmp[4];

            if (val) {
                CACHE[key] = val.join("\r\n").slice(0, length);
                msg.push("STORED\r\n");
            };

        };

        /* Write out the response */
        sock.write(msg.join(""));

    });

    if (SINGLE) process.exit(1);

});

server.listen(11211);
