# GXC-Relay-Contract

This project constructs of 2 parts:

- A Mintable, Brnable and Pausable ERC20 smart contract implemented for GXC, based on openzeppelin and truffle
- A [Relay Contract](./gxc-contract) writen in C++ which will be deployed on GXChain

[Documents](./docs/README.md)

### Install dependencies

```bash
npm i truffle -g
npm install
```

### Deploy

> Note: A file named .secret should be created which contains the brain key of your eth account

```bash
npm run ropsten // for ropsten testnet
npm run live // for ethereum mainnet
```

### Test

```bash
truffle console --network ropsten
```
