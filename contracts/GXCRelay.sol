pragma solidity 0.6.2;

import "@openzeppelin/contracts/access/AccessControl.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
import "@openzeppelin/contracts/utils/Pausable.sol";

contract GXCRelay is AccessControl, Pausable {
    struct TokenSettings {
        bool enableDeposit;
        uint256 minDeposit;
        uint256 minDeliver;
    }

    bytes32 public constant MANAGER_ROLE = keccak256("MANAGER_ROLE");
    bytes32 public constant PAUSER_ROLE = keccak256("PAUSER_ROLE");
    bytes32 public constant DELIVER_ROLE = keccak256("DELIVER_ROLE");

    TokenSettings private _ETHSettings;
    mapping (address => TokenSettings) private _tokenSettings;

    event Deposit(address indexed token, address indexed from, uint256 amount, string to);
    event DepositETH(address indexed from, uint256 amount, string to);
    
    event Deliver(address indexed token, address indexed to, uint256 amount, string from, bytes32 txid);
    event DeliverETH(address indexed to, uint256 amount, string from, bytes32 txid);

    constructor() public {
        _setupRole(DEFAULT_ADMIN_ROLE, _msgSender());
        
        _setupRole(MANAGER_ROLE, _msgSender());
        _setupRole(PAUSER_ROLE, _msgSender());
    }

    function deposit(address token, uint256 amount, string memory to) public whenNotPaused {
        require(_tokenSettings[token].enableDeposit, "Unsupported token.");
        require(_tokenSettings[token].minDeposit <= amount, "Insufficient amount.");
        IERC20(token).transferFrom(_msgSender(), address(this), amount);
        emit Deposit(token, _msgSender(), amount, to);
    }
    function depositETH(string memory to) public payable whenNotPaused {
        require(_ETHSettings.enableDeposit, "Unsupported token.");
        require(_ETHSettings.minDeposit <= msg.value, "Insufficient amount.");
        emit DepositETH(_msgSender(), msg.value, to);
    }
    
    function deliver(address token, address to, uint256 amount, string memory from, bytes32 txid) public whenNotPaused {
        require(hasRole(DELIVER_ROLE, _msgSender()), "Invalid sender.");
        require(_tokenSettings[token].minDeliver <= amount, "Insufficient amount.");
        IERC20(token).transfer(to, amount);
        emit Deliver(token, to, amount, from, txid);
    }
    function deliverETH(address payable to, uint256 amount, string memory from, bytes32 txid) public whenNotPaused {
        require(hasRole(DELIVER_ROLE, _msgSender()), "Invalid sender.");
        require(_ETHSettings.minDeliver <= amount, "Insufficient amount.");
        to.transfer(amount);
        emit DeliverETH(to, amount, from, txid);
    }
    
    function pause() public {
        require(hasRole(PAUSER_ROLE, _msgSender()), "Invalid sender.");
        _pause();
    }
    function unpause() public {
        require(hasRole(PAUSER_ROLE, _msgSender()), "Invalid sender.");
        _unpause();
    }
    
    function manageTokenSettings(address token, bool enableDeposit, uint256 minDeposit, uint256 minDeliver) public {
        require(hasRole(MANAGER_ROLE, _msgSender()), "Invalid sender.");
        _tokenSettings[token].enableDeposit = enableDeposit;
        _tokenSettings[token].minDeposit = minDeposit;
        _tokenSettings[token].minDeliver = minDeliver;
    }
    function manageETHSettings(bool enableDeposit, uint256 minDeposit, uint256 minDeliver) public {
        require(hasRole(MANAGER_ROLE, _msgSender()), "Invalid sender.");
        _ETHSettings.enableDeposit = enableDeposit;
        _ETHSettings.minDeposit = minDeposit;
        _ETHSettings.minDeliver = minDeliver;
    }
    
    function getTokenSettings(address token) public view returns (bool enableDeposit, uint256 minDeposit, uint256 minDeliver) {
        enableDeposit = _tokenSettings[token].enableDeposit;
        minDeposit = _tokenSettings[token].minDeposit;
        minDeliver = _tokenSettings[token].minDeliver;
    }
    function getETHSettings() public view returns (bool enableDeposit, uint256 minDeposit, uint256 minDeliver) {
        enableDeposit = _ETHSettings.enableDeposit;
        minDeposit = _ETHSettings.minDeposit;
        minDeliver = _ETHSettings.minDeliver;
    }
}