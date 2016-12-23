'use strict';

require('./index.html');
var Elm = require('./Main');

var elm = Elm.Main.fullscreen();

elm.hot.subscribe(function(event, context) {
  context.state.swapCount ++
});
