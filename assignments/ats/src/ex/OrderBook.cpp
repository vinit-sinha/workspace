#include "ex/OrderBook.h"
#include "ex/state/OrderInfo.h"
#include <algorithm>

template<typename Book>
void printAll(std::ostream& out, Book& book) 
{
    out << "[";
    for(auto& x: book ) {
        out << x.orderId << ":" << x.quantity << "@" << x.price << " ";
    }
    out << "]" << std::endl;
}
void printHeaders(std::ostream& out, std::size_t uptoLevel)
{
    out << std::setw(10) << std::left << "Product";
    std::string columnName = "Bid";
    for( std::size_t i = 1; i <= uptoLevel; ++i ) {
        out << std::setw(10) << (columnName + std::to_string(i));
    } 
    columnName = "Ask";
    for( std::size_t i = 1; i <= uptoLevel; ++i ) {
        out << std::setw(10) << (columnName + std::to_string(i));
    } 
}
template<typename OrderInfoSet>
void printPricePoints(std::ostream& out, OrderInfoSet& infoSet, std::size_t uptoLevel) 
{
    std::size_t i = std::min( uptoLevel, infoSet.size() );
    auto infoSetIter = infoSet.begin();
    while( infoSetIter != infoSet.end() && i > 0 ) {
        out << std::setw(10) << std::left << infoSetIter->price;
        i--;
        //Skip to next price level
        infoSetIter = infoSet.upper_bound( *infoSetIter );
    }

    if( infoSet.size() < uptoLevel ) {
        for(std::size_t  blankEntries = uptoLevel - infoSet.size(); blankEntries > 0; blankEntries-- ) {
            out << std::setw(10) << std::left << "-";
        }
    }
}

void ex::OrderBook::print(std::ostream& out, std::size_t level, bool printHeader) 
{
    if( printHeader ) {
        printHeaders(out, level);
        out << std::endl;
    }

    for(auto& product: products) {
        out << std::setw(10) << std::left << product;
        printPricePoints( out, buys[product], level );
        printPricePoints( out, sells[product], level );
        out << std::endl;
    }
}
ex::type::ErrorCode ex::OrderBook::notify(const ex::msg::NewOrder& obj)
{
    if ( orderExists( obj.orderId ) ) return ex::type::ErrorCode::DuplicateOrderId;

    ex::state::OrderInfo ord;
    ord.orderId = obj.orderId;
    ord.price = obj.price;
    ord.quantity = obj.quantity;


    products.emplace( obj.productId );
    orderIdToProductIdMap[obj.orderId] = obj.productId;

    if( obj.side == ex::type::Side::Buy ) {
        buys[obj.productId].emplace( ord );
    } else {
        sells[obj.productId].emplace( ord );
    }

    return ex::type::ErrorCode::Ok;
}

ex::type::ErrorCode ex::OrderBook::notify(const ex::msg::AmendOrder& obj)
{
    auto productIdIter = orderIdToProductIdMap.find( obj.orderId );
    if( productIdIter == orderIdToProductIdMap.end() ) {
        return ex::type::ErrorCode::InvalidProductId;
    }

    ex::type::ErrorCode err = ex::type::ErrorCode::Ok;

    if( obj.side == ex::type::Side::Buy ) {
        err = amend( buys[productIdIter->second], obj );
    } else {
        err = amend( sells[productIdIter->second], obj );
    }

    return err;
}

ex::type::ErrorCode ex::OrderBook::notify(const ex::msg::CancelOrder& obj)
{
    auto productIdIter = orderIdToProductIdMap.find( obj.orderId );
    if( productIdIter == orderIdToProductIdMap.end() ) {
        return ex::type::ErrorCode::InvalidProductId;
    }

    ex::type::ErrorCode err = ex::type::ErrorCode::Ok;

    if( obj.side == ex::type::Side::Buy ) {
        err = cancel( buys[productIdIter->second], obj );
    } else {
        err = cancel( sells[productIdIter->second], obj );
    }

    return err;
}

ex::type::ErrorCode ex::OrderBook::notify(const ex::msg::Trade& obj)
{
    return execute( buys[obj.productId], sells[obj.productId], obj);
}
