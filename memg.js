/* Create the cache object */
var CACHE = {};

var handle_data = function (data, sock) {

    "use strict";

    /* Process the data, splitting it into parts 

       TODO: improve the splitting part. */
    var parts = data.split(' ');
    var cmd = parts[0], key = parts[1], val;

    switch(cmd) {
        
        case "get":
            key = key.split('\r\n')[0];
            val = CACHE[key];
            if (val) {
                var msg = "VALUE " + key + " 0 " + val.length + "\r\n";
                msg += val + "\r\n";
                sock.write(msg);
            }
            sock.write("END\r\n");
            break;

        case "set":
            var tmp = parts[4].split('\r\n');
            if (tmp[1]) {
                var length = +tmp[0];
                CACHE[key] = tmp[1].slice(0, length);
                sock.write("STORED\r\n");
            };
            break;
    };
};

var handle_conn = function (sock) {

    "use strict";

    var data;
    
    sock.on('data', function(chunk) {

        /* Upon receiving the socket connection, extract the data */
        data = chunk.toString()

        /* If we have something, we will process it */
        if (data) handle_data(data, sock);

    });

    /* Wrap up and return the headers */
    sock.on('end', function () {

        sock.writeHead(200);
        sock.end();

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

