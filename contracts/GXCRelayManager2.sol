pragma solidity 0.6.2;

import "@openzeppelin/contracts/access/AccessControl.sol";
import "./GXCRelay.sol";

contract GXCRelayManager2 is AccessControl {
    bytes32 public constant DELIVER_ROLE = keccak256("DELIVER_ROLE");
    
    mapping(bytes32 => bool) public txidMap;
    
    GXCRelay public relay;
    
    constructor(address _relay) public {
        _setupRole(DEFAULT_ADMIN_ROLE, _msgSender());
        
        relay = GXCRelay(_relay);
    }
    
    modifier checkTxid(bytes32 txid) {
        require(txidMap[txid] == false, "Invalid txid.");
        txidMap[txid] = true;
        _;
    }
    
    function deliver(address token, address to, uint256 amount, string memory from, bytes32 txid) public checkTxid(txid) {
        require(hasRole(DELIVER_ROLE, _msgSender()), "Invalid sender.");
        relay.deliver(token, to, amount, from, txid);
    }
    function deliverETH(address payable to, uint256 amount, string memory from, bytes32 txid) public checkTxid(txid) {
        require(hasRole(DELIVER_ROLE, _msgSender()), "Invalid sender.");
        relay.deliverETH(to, amount, from, txid);
    }
}