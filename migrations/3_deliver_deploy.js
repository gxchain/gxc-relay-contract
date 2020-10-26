const deliver = artifacts.require('GXCDeliver');

module.exports = function(deployer) {
  deployer.deploy(deliver, '0x2e93FE8d550a7B7E7b2e561cd45ceBccbAa79358','0x8A7AB6894860949e75a94420DCa2D5Ea9Cbe31C9');
};