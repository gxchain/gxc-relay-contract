# GXC-Solidity

Mintable, Brnable and Pausable ERC20 smart contract implemented for GXC, based on openzeppelin and truffle

[Documents](./docs/READEME.md)

### Install dependencies

```bash
npm i truffle -g
npm install
```

### Deploy

> Note: A file named .secret should be created with contains the brain key of your eth account

```bash
npm run ropsten // for ropsten testnet
npm run live // for ethereum mainnet
```

### Test

```bash
truffle console --network ropsten
```
