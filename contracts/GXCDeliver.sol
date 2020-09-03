pragma solidity 0.6.2;

import "./GXC.sol";
contract GXCDeliver{
    bytes32[50] private txidArray;
    uint256 private arrayLength = 50;
    uint256 private id;
    GXC gxc;
    address ADMINACCOUNT;
    constructor(GXC _gxc, address _ADMINACCOUNT)public{
        gxc = _gxc;
        ADMINACCOUNT = _ADMINACCOUNT;
    }
    event Deliver(address indexed to, uint256 amount, string from, bytes32 txid);

    
    function deliver(
        address to,
        uint256 amount,
        string memory from,
        bytes32 txid
    ) public {
        require(
            msg.sender == ADMINACCOUNT,"Must be the adminaccount"
        );
        for (uint256 i = 0; i < arrayLength; i++) {
            require(
                txidArray[i] !=txid,"The txid has existed"
            );
        }
        uint256 id_number = id % arrayLength;
        txidArray[id_number] = txid;
        id++;
        gxc.transfer(to,amount);
        emit Deliver(to, amount, from, txid);
    }

    function getTxids() public view returns (bytes32[50] memory) {
        return txidArray;
    }
}