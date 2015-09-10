var os          = require('os');
var net         = require('net');
const express   = require('express');
const config    = require('configure');
var app         = express();

app.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    res.header('Access-Control-Allow-Methods', 'GET,PUT,POST,DELETE');
    next();
});

var apiRouter = express.Router();
var sseRouter = express.Router();

app.use('/api', apiRouter);
app.use('/sse', sseRouter);

apiRouter.route('/').get(function(req, res) {
    res.end('API');
});

apiRouter.route('/debug').get(function(req, res) {
    res.end('DEBUG');
});

apiRouter.route('/setAngle/:id/:angle').get(function(req, res) {
    var data = '{' + 
                '"handler":2,' +
                '"id":' + req.params.id + "," +
                '"angle":' + req.params.angle + '}';
                
    var client = net.createConnection("/tmp/andy/position_request");
    client.on("connect", function() {
        client.write (data);
        console.log ("[setAngle] Sending request to Rob-E (" + data + ")");
    });
    
    res.end('OK');
});

/*
apiRouter.route('/setCoordinates/:x/:y/:z/:p').get(function(req, res) {
    var data = '{' + 
                '"handler":1,' +
                '"x":' + req.params.x + "," +
                '"y":' + req.params.y + "," +
                '"z":' + req.params.z + "," +
                '"p":' + req.params.p + '}';
    
    var client = net.createConnection("/tmp/andy/position_request");
    client.on("connect", function() {
        client.write (data);
        console.log ("[setCoordinates] Sending request to Rob-E (" + data + ")");
    });
    client.on('error', function(err){ console.log(err.message + " data:" + data); });
  
    
    res.end('OK');
});

apiRouter.route('/getBoundaries').get(function(req, res) {
    var data = {
        robotBoundaries: {
            minX: "-10",
            maxX: "10",
            minY: "0",
            maxY: "10",
            minZ: "0",
            maxZ: "10",
            minPinch: "0",
            maxPinch: "1"
        }
    }
    
    console.log ("[getBoundaries] ");
    res.end(JSON.stringify(data));
});

*/

apiRouter.route('/setCoordinates/:x/:y/:z/:closure/:pitch').get(function(req, res) {
    var data = '{' + 
                '"handler":1,' +
                '"x":' + req.params.x + "," +
                '"y":' + req.params.y + "," +
                '"z":' + req.params.z + "," +
                '"closure":' + req.params.closure + "," +
                '"pitch":' + req.params.pitch + '}';
    
    var client = net.createConnection("/tmp/andy/position_request");
    client.on("connect", function() {
        client.write (data);
        console.log ("[setCoordinates] Sending request to Rob-E (" + data + ")");
    });
    client.on('error', function(err){ console.log(err.message + " data:" + data); });
  
    
    res.end('OK');
});

apiRouter.route('/getBoundaries').get(function(req, res) {
    var data = {
        robotBoundaries: {
            minX: "-10",
            maxX: "10",
            minY: "0",
            maxY: "10",
            minZ: "0",
            maxZ: "10",
            minPinch: "0",
            maxPinch: "1",
            minPalmPitch: "0",
            maxPalmPitch: "180"
        }
    }
    
    console.log ("[getBoundaries] ");
    res.end(JSON.stringify(data));
});


var server = app.listen(config.serverPort, function() {
    console.info('Listening on port ' + server.address().port);
    printIPAdress ();
});

function printIPAdress () {
    var ifaces = os.networkInterfaces();
    Object.keys(ifaces).forEach(function (ifname) {
        var alias = 0;
        ifaces[ifname].forEach(function (iface) {
            if ('IPv4' !== iface.family || iface.internal !== false) {
                return;
            }

            if (alias >= 1) {
                console.log(ifname + ':' + alias, iface.address);
            } else {
                console.log(ifname, iface.address);
            }
        });
    });
}

