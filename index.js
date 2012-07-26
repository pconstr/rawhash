try {
  module.exports = require('./build/Release/rawhash.node');
} catch(err) {
  try {
    module.exports = require('./build/Debug/rawhash.node');
  } catch(err) {
    module.exports = require('./rawhash.node');
  }
}
