pragma solidity 0.6.2;

import "./GXC.sol";
contract GXCDeliver{
    mapping (bytes32 => bool) public executedMap;
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
        require(executedMap[txid] == false, "The txid has existed");
        executedMap[txid] = true;
        gxc.transfer(to,amount);
        emit Deliver(to, amount, from, txid);
    }

    function HasExecuted(bytes32 txid) public view returns (bool) {
        return executedMap[txid];
    }
}