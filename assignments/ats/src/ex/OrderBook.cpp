#include "ex/OrderBook.h"
#include <algorithm>

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
template<typename Book>
void printPricePoints(std::ostream& out, Book& book, std::size_t uptoLevel) 
{
    for( auto iter = book.begin(); iter != book.end(); ++iter ) {
        out << std::setw(10) << std::left << iter->first;
        auto& infoSet = iter->second;
        std::size_t i = uptoLevel;
        for( auto infoSetIter = infoSet.begin(); infoSetIter != infoSet.end() && i >= 0; ++infoSetIter ) {
            out << std::setw(10) << std::left << infoSetIter->price;
            i--;
        }
        out << std::endl;
    }
}
template<typename Book>
void printAll(std::ostream& out, Book& book) 
{
    out << "[";
    for(auto& x: book ) {
        out << x.orderId << ":" << x.quantity << "@" << x.price << " ";
    }
    out << "]" << std::endl;
}
void ex::OrderBook::print(std::ostream& out, std::size_t level, bool printHeader) 
{
    if( printHeader ) {
        printHeaders(out, level);
        out << std::endl;
    }

    printPricePoints( out, buys, level );
    printPricePoints( out, sells, level );

    out << std::endl;
}
void ex::OrderBook::notify(const ex::msg::NewOrder& obj)
{
    OrderInfo ord;
    ord.orderId = obj.orderId;
    ord.price = obj.price;
    ord.quantity = obj.quantity;
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
    auto productIdIter = orderIdToProductIdMap.find( obj.orderId );
    if( productIdIter == orderIdToProductIdMap.end() ) {
        //TODO Error Reporting
        return;
    }

    // execute( buys[productIdIter->second], sells[productIdIter->second], obj);
}
