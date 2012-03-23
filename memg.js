var CACHE = {};

var handle_data = function (data) {

    console.log(data);

    var parts = data.split();
    var cmd = parts[0], key = parts[1], val;

    switch(cmd) {
        
        case "get":
            val = CACHE[key];
            console.log("VALUE " + key + " 0 " + val.length + "\r\n" );
            console.log(val + "\r\n");
            console.log("END\r\n");
            break;

        case "set":
            var length = +parts[4];
            val = String(Math.random());
            debugger;
            CACHE[key] = val.slice(0, length);
            console.log("STORED\r\n");
            break;

    };
};

var handle_conn = function (sock) {

    "use strict";

    var data;
    
    console.log("[200] " + sock.method + "to " + sock.url);

    sock.on('data', function(chunk) {

        console.log(chunk)

        // Extract the POSTed data
        data = chunk.toString()

        // If we have data, parse it and take action
        if (data) handle_data(data);

    });

    sock.on('end', function () {

        // Write the response
        sock.writeHead(200);
        sock.end(data);

    });
};

var net = require('net');

var server = net.createServer(function(sock) {
    
    handle_conn(sock);

});


server.listen(11211);
