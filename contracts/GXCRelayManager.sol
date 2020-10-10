pragma solidity 0.6.2;

import "@openzeppelin/contracts/access/AccessControl.sol";
import "./GXCRelay.sol";

contract GXCRelayManager is AccessControl {
    bytes32 public constant DELIVER_ROLE = keccak256("DELIVER_ROLE");
    
    bytes32[50] private _txidArray;
    uint256 constant private _arrayLength = 50;
    uint8 private _id;
    
    GXCRelay private _relay;
    
    constructor(address relay) public {
        _setupRole(DEFAULT_ADMIN_ROLE, _msgSender());
        
        _relay = GXCRelay(relay);
    }
    
    modifier checkTxid(bytes32 txid) {
        for (uint256 i = 0; i < _arrayLength; i++) {
            require(_txidArray[i] != txid,"The txid has existed.");
        }
        _txidArray[_id] = txid;
        _id = _id == _arrayLength - 1 ? 0 : _id + 1;
        _;
    }
    
    function deliver(address token, address to, uint256 amount, string memory from, bytes32 txid) public checkTxid(txid) {
        require(hasRole(DELIVER_ROLE, _msgSender()), "Invalid sender.");
        _relay.deliver(token, to, amount, from, txid);
    }
    function deliverETH(address payable to, uint256 amount, string memory from, bytes32 txid) public checkTxid(txid) {
        require(hasRole(DELIVER_ROLE, _msgSender()), "Invalid sender.");
        _relay.deliverETH(to, amount, from, txid);
    }
}