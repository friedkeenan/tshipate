#pragma once

namespace tsh {

    using Address   = std::uint16_t;
    using RawOpcode = std::uint16_t;

    using DisassembleOutputIterator = std::back_insert_iterator<std::string>;

}
