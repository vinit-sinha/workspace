#include <fstream>
#include <iostream>
#include <functional>
#include <deque>

#include "ex/msg/Decoder.h"
#include "ex/msg/NewOrder.h"
#include "ex/msg/AmendOrder.h"
#include "ex/msg/CancelOrder.h"
#include "ex/msg/Trade.h"
#include "ex/OrderBook.h"

struct NewOrderError {
    ex::type::ErrorCode err;
    ex::msg::NewOrder obj;
};

struct AmendOrderError {
    ex::type::ErrorCode err;
    ex::msg::AmendOrder obj;
};

struct CancelOrderError {
    ex::type::ErrorCode err;
    ex::msg::CancelOrder obj;
};

struct TradeError {
    ex::type::ErrorCode err;
    ex::msg::Trade obj;
};

struct DecodeHandler {
    DecodeHandler(ex::OrderBook& ob): orderBook(ob)
    {}

    DecodeHandler(const DecodeHandler&) = delete; 
    DecodeHandler(DecodeHandler&&) = delete; 


    DecodeHandler& operator=(const DecodeHandler&) = delete;
    DecodeHandler& operator=(DecodeHandler&&) = delete;

    void operator()(const ex::msg::NewOrder& obj) {
        std::cout << "RCVD: " << obj << std::endl;
        auto err = orderBook.notify( obj );
        if( err != ex::type::ErrorCode::Ok ) 
            newOrderErrors.push_back( {err, obj} );
        else 
            printOrderBook();
    }
    void operator()(const ex::msg::AmendOrder& obj) {
        auto err = orderBook.notify( obj );
        if( err != ex::type::ErrorCode::Ok ) 
            amendOrderErrors.push_back( {err, obj} );
        else
            printOrderBook();
    }

    void operator()(const ex::msg::CancelOrder& obj) {
        auto err = orderBook.notify( obj );
        if( err != ex::type::ErrorCode::Ok ) 
            cancelOrderErrors.push_back( {err, obj} );
        else
            printOrderBook();
    }

    void operator()(const ex::msg::Trade& obj) {
        auto err = orderBook.notify( obj );
        if( err != ex::type::ErrorCode::Ok ) {
            tradeErrors.push_back( {err, obj} );
        } else {
            printTrade(obj);
            printOrderBook();
        }
    }

    ~DecodeHandler() {
        std::cout << "Error Summary during exit" << std::endl;
        printAllErrors();
        std::cout << "Order Book Summary during exit" << std::endl;
        orderBook.print( std::cout, 5, true); 
    }
private:
    ex::OrderBook& orderBook;

    std::deque<NewOrderError> newOrderErrors;
    std::deque<AmendOrderError> amendOrderErrors;
    std::deque<CancelOrderError> cancelOrderErrors;
    std::deque<TradeError> tradeErrors;

    std::size_t msgCount = 0;

    void printOrderBook() {
        if( ++msgCount == 10 ) {
            std::cout << "Summary after receiveing 10 messages" << std::endl;
            orderBook.print(std::cout, 5, true );
            std::cout << std::endl;
            msgCount = 1;
        }
    }

    void printTrade(const ex::msg::Trade& obj) {
        std::cout << obj << " => ";
        auto priceQty = orderBook.getLastTradedPriceAndQuantiity(obj.productId);
        std::cout << "Product " << obj.productId << ":" << priceQty.second << "@" << priceQty.first << std::endl;;
    }

    template<typename Errors>
    void printErrors(Errors& errors) {
        for(auto& e: errors ) {
            std::cout << "\"" << e.err << "\" occuerred while processing " << e.obj << std::endl;
        }

        errors.clear();
    }

    void printAllErrors() {
        printErrors( newOrderErrors );
        printErrors( amendOrderErrors );
        printErrors( cancelOrderErrors );
        printErrors( tradeErrors );
    }

};

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "[ERROR]: Missing messages file name" << std::endl
                << "[USAGE]: feed_handler <path/to/messages/file" << std::endl;
        return -1;
    }

    std::ifstream ifile(argv[1]);
    if( !ifile ) {
        std::cerr << "[ERROR]: File specified at " << argv[1] << " doesnot exist" << std::endl;
        return -2;
    }
    ex::OrderBook orderBook;
    DecodeHandler dh(orderBook);

    ex::msg::Decoder<DecodeHandler> decoder(ifile, std::ref(dh) );

    while (decoder.hasMoreMessages()) {
        decoder.decode();
    }
   
    return 0;
}


