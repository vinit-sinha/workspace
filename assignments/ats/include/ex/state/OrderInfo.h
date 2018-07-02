#pragma once
#include "ex/type/Types.h"

namespace ex{ namespace state{

    struct OrderInfo {
        ex::type::OrderId orderId;
        ex::type::Quantity quantity;
        ex::type::Price price;
    };

    struct DescendingPriceOrdering {
        bool operator()(const ex::state::OrderInfo& lhs, const ex::state::OrderInfo& rhs) {
            return lhs.price > rhs.price;
        }
    };

    struct AscendingPriceOrdering {
        bool operator()(const ex::state::OrderInfo& lhs, const ex::state::OrderInfo& rhs) {
            return lhs.price < rhs.price;
        }
    };


}}
