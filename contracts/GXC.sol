// SPDX-License-Identifier: MIT

pragma solidity >=0.4.21 <0.8.0;

import "@openzeppelin/contracts/presets/ERC20PresetMinterPauser.sol";

contract GXC is ERC20PresetMinterPauser {
    event Mint(address indexed to, uint256 amount, string txid);

    event Burn(address indexed sender, uint256 amount, string gxcAccount);

    constructor(string memory name, string memory symbol)
        public
        ERC20PresetMinterPauser(name, symbol)
    {}

    function mint(
        address to,
        uint256 amount,
        string memory txid
    ) public virtual {
        super.mint(to, amount);
        emit Mint(to, amount, txid);
    }

    function burn(uint256 amount, string memory gxcAccount) public virtual {
        super.burn(amount);
        emit Burn(msg.sender, amount, gxcAccount);
    }
}
