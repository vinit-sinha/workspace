#pragma once
#include "ex/type/Types.h"
#include <iostream>

namespace ex { namespace msg {

    struct NewOrder {
        static constexpr  ex::type::Action action = ex::type::Action::New;

        ex::type::ProductId productId;
        ex::type::OrderId orderId;
        ex::type::Side side;
        ex::type::Quantity quantity;
        ex::type::Price price;

        friend std::istream& operator >> (std::istream& in, NewOrder& obj);
        friend std::ostream& operator << (std::ostream& out, const NewOrder& obj);
    };
}}
