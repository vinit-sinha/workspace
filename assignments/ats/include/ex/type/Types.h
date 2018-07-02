#pragma once
#include <cstdint>
#include <iostream>

namespace ex { namespace type {

    using Quantity = std::uint32_t;
    using Price = double;

    using ProductId = std::uint64_t;
    using OrderId = std::uint64_t;

    enum class Action : char{
        New = 'N'
            , Amend = 'M'
            , Cancel = 'R'
            , Trade = 'X'
            , Unknown = '?'
    };

    std::ostream& operator<<(std::ostream& out, Action action);
    std::istream& operator>>(std::istream& in, Action& action);

    enum class Side : char {
        Buy = 'B'
            , Sell = 'S'
            , Unknown = '?'
    };

    std::ostream& operator<<(std::ostream& out, Side side);
    std::istream& operator>>(std::istream& in, Side& side);

    enum class ErrorCode {
        Ok
        , InvalidOrderId
        , InvalidProductId
        , DuplicateOrderId
        , DuplicateProductId
        , InvalidPrice
        , InvalidQuantity
        , CorruptMessage
        , Unknown = '?'
    };
    std::ostream& operator<<(std::ostream& out, const ErrorCode& e);
}}
