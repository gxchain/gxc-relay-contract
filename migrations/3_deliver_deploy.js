const deliver = artifacts.require('GXCDeliver');

module.exports = function(deployer) {
  deployer.deploy(deliver, '0x04dC2FbA33E82F606aB57cf5CEfB4eF729530671','0xAE0c03FdeDB61021272922F7804505CEE2C12c78');
};