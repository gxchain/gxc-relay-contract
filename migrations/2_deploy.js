const Token = artifacts.require('GXC');

module.exports = function(deployer) {
  deployer.deploy(Token, 'GXChain Core Asset', 'GXC');
};