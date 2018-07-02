#pragma once
#include "ex/type/Types.h"
#include <iostream>

namespace ex { namespace msg {

    struct Trade {
        static constexpr  ex::type::Action action = ex::type::Action::Trade;

        ex::type::ProductId productId;
        ex::type::Quantity quantity;
        ex::type::Price price;

        friend std::istream& operator >> (std::istream& in, Trade& obj);
        friend std::ostream& operator << (std::ostream& out, const Trade& obj);
    };
}}
