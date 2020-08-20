// SPDX-License-Identifier: MIT

pragma solidity >=0.4.21 <0.8.0;
pragma experimental ABIEncoderV2;

import "@openzeppelin/contracts/presets/ERC20PresetMinterPauser.sol";
contract GXC is ERC20PresetMinterPauser{
    bytes32 public constant ADJUST_ROLE = keccak256("ADJUST_ROLE");

    string[10] private txid_array;
    uint arrayLength = 10;
    uint256 private id;
    
    uint minMint = 50000;
    uint minBurn = 50000;

    uint8 decimals_ = 5;

    event Mint(address indexed to, uint256 amount, string txid, string from_account);

    event Burn(address indexed sender, uint256 amount, string gxcAccount);

    constructor(string memory name, string memory symbol)
        public
        ERC20PresetMinterPauser(name, symbol)
        {super._setupDecimals(decimals_);
        _setupRole(ADJUST_ROLE, _msgSender());}

    function mint(
        address to,
        uint256 amount,
        string memory txid,
        string memory from_account
    ) public virtual {
        require(amount >= minMint, "The minimum value must be greater than minMint");
        for(uint i =0; i<arrayLength; i++){
            require( keccak256(abi.encodePacked(txid_array[i])) != keccak256(abi.encodePacked(txid)), "The txid has existed ,you can't use it again");
        }
        uint id_number = id % arrayLength;
        txid_array[id_number] = txid;
        id_number++;
        super.mint(to, amount);
        emit Mint(to, amount, txid, from_account);
    }

    function burn(uint256 amount, string memory gxcAccount) public virtual {
        require(amount >= minBurn, "The minimum value must be greater than minMint");
        super.burn(amount);
        emit Burn(msg.sender, amount, gxcAccount);
    }

    function adjustMinnumber(uint _burnMin, uint _mintMin) public virtual {
        require(hasRole(ADJUST_ROLE, _msgSender()), "Must have adjust role to mint");
        minMint = _mintMin; 
        minBurn = _burnMin;
    }

    function get_txid_array()public returns(string[10] memory){
        return txid_array;
    }

}
