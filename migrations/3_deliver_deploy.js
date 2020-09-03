const deliver = artifacts.require('GXCDeliver');

module.exports = function(deployer) {
  deployer.deploy(deliver, '0x04dC2FbA33E82F606aB57cf5CEfB4eF729530671','0x7BEE40ECB2cAFf57ea9020234960e1a72b770Dc7');
};