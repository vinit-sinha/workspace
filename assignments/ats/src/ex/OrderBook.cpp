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
    for( auto infoSetIter = infoSet.begin(); infoSetIter != infoSet.end() && i > 0; infoSetIter++ ) {
        out << std::setw(10) << std::left << infoSetIter->price;
        i--;
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
void ex::OrderBook::notify(const ex::msg::NewOrder& obj)
{
    ex::state::OrderInfo ord;
    ord.orderId = obj.orderId;
    ord.price = obj.price;
    ord.quantity = obj.quantity;

    products.emplace( obj.productId );
    if( obj.side == ex::type::Side::Buy ) {
        buys[obj.productId].emplace( ord );
    } else {
        sells[obj.productId].emplace( ord );
    }

    orderIdToProductIdMap[obj.orderId] = obj.productId;
}
void ex::OrderBook::notify(const ex::msg::AmendOrder& obj)
{
    auto productIdIter = orderIdToProductIdMap.find( obj.orderId );
    if( productIdIter == orderIdToProductIdMap.end() ) {
        //TODO Error Reporting
        return;
    }

    //TODO: Also make sure that only Price and Quantiy is asked to change
    if( obj.side == ex::type::Side::Buy ) {
        amend( buys[productIdIter->second], obj );
    } else {
        amend( sells[productIdIter->second], obj );
    }
}

void ex::OrderBook::notify(const ex::msg::CancelOrder& obj)
{
    auto productIdIter = orderIdToProductIdMap.find( obj.orderId );
    if( productIdIter == orderIdToProductIdMap.end() ) {
        //TODO Error Reporting
        return;
    }

    if( obj.side == ex::type::Side::Buy ) {
        cancel( buys[productIdIter->second], obj );
    } else {
        cancel( sells[productIdIter->second], obj );
    }
}

void ex::OrderBook::notify(const ex::msg::Trade& obj)
{
    execute( buys[obj.productId], sells[obj.productId], obj);
}
