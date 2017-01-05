var WebSocketServer = require('ws').Server;
var wss = new WebSocketServer({
    port: 8080
});
var clientIdCounter = 0;

wss.broadcast = function broadcast(data, ignoreClient) {
    wss.clients.forEach(function each(client) {
        if (client === ignoreClient) {
            return;
        }

        console.log('#' + client.id + ' > ' + data);

        client.send(data);
    });
};

wss.on('connection', function connection(client) {
    client.id = ++clientIdCounter;

    console.log('client #' + client.id + ' connected');

    client.on('message', function incoming(message) {
        console.log('received: %s', message);

        wss.broadcast(message, client);
    });
});
