#pragma once
#include "ex/type/Types.h"
#include <iostream>

namespace ex { namespace msg {

    struct CancelOrder {
        static constexpr  ex::type::Action action = ex::type::Action::Cancel;

        ex::type::OrderId orderId;
        ex::type::Side side;
        ex::type::Quantity quantity;
        ex::type::Price price;

        friend std::istream& operator >> (std::istream& in, CancelOrder& obj);
        friend std::ostream& operator << (std::ostream& out, const CancelOrder& obj);
    };
}}
