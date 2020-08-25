// SPDX-License-Identifier: MIT

pragma solidity >=0.4.21 <0.8.0;
pragma experimental ABIEncoderV2;

import "@openzeppelin/contracts/presets/ERC20PresetMinterPauser.sol";

contract GXC is ERC20PresetMinterPauser {
    bytes32 public constant ADJUST_ROLE = keccak256("ADJUST_ROLE");
    bytes32 public constant DELIVER_ROLE = keccak256("DELIVER_ROLE");

    string[10] private txidArray;
    uint256 arrayLength = 10;
    uint256 private id;

    uint256 private _minDeliver = 50000;
    uint256 private _minBurn = 50000;

    uint8 private decimals_ = 5;

    event Deliver(address indexed to, uint256 amount, string from, string txid);

    event Burn(address indexed from, uint256 amount, string to);

    constructor(string memory name, string memory symbol)
        public
        ERC20PresetMinterPauser(name, symbol)
    {
        super._setupDecimals(decimals_);
        _setupRole(ADJUST_ROLE, _msgSender());
    }


    function deliver(
        address to,
        uint256 amount,
        string memory from,
        string memory txid
    ) public {
        require(
            amount >= _minDeliver,
            "The minimum value must be greater than minDeliver"
        );
        require(hasRole(DELIVER_ROLE, _msgSender()), "Must have deliver role to deliver");
        for (uint256 i = 0; i < arrayLength; i++) {
            require(
                keccak256(abi.encodePacked(txidArray[i])) !=
                    keccak256(abi.encodePacked(txid)),
                "The txid has existed"
            );
        }
        uint256 id_number = id % arrayLength;
        txidArray[id_number] = txid;
        id++;
        transfer(to, amount);
        emit Deliver(to, amount, from, txid);
    }

    function burn(uint256 amount, string memory to) public {
        require(
            amount >= _minBurn,
            "The minimum value must be greater than minBurn"
        );
        super.burn(amount);
        emit Burn(msg.sender, amount, to);
    }

    function adjustParams(uint256 minDeliver , uint256 minBurn)
        public
    {
        require(hasRole(ADJUST_ROLE, _msgSender()), "Adjust role required");
        _minDeliver = minDeliver;
        _minBurn = minBurn;
    }

    function getParams() public returns (uint256 ,uint256){
        return (_minDeliver, _minBurn);
    }

    function getTxids() public returns (string[10] memory) {
        return txidArray;
    }
}
