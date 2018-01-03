var WebSocketServer = require("ws").Server;

var port = 3000;
var wss = new WebSocketServer({
  port: port
});
var clientIdCounter = 0;

wss.broadcast = function broadcast(data, ignoreClient) {
  wss.clients.forEach(function each(client) {
    if (client === ignoreClient) {
      return;
    }

    console.log("#" + client.id + " > " + data);

    client.send(data);
  });
};
wss.on("listening", function connection() {
  console.log("server has been started on port " + port);
});
wss.on("connection", function connection(client) {
  client.id = ++clientIdCounter;

  console.log("client #" + client.id + " connected");
  client.on("error", function(err) {
    console.log(err);
  });
  client.on("message", function incoming(message) {
    console.log("received: %s", message);

    wss.broadcast(message, client);
  });
});
