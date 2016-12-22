'use strict';

require('./index.html');
var Elm = require('./Main');

var elm = Elm.Main.fullscreen({
  swapCount: 0
});

elm.hot.subscribe(function(event, context) {
  context.state.swapCount ++
});
