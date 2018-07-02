#include <fstream>
#include <iostream>
#include <functional>

#include "ex/msg/Decoder.h"
#include "ex/msg/NewOrder.h"
#include "ex/msg/AmendOrder.h"
#include "ex/msg/CancelOrder.h"
#include "ex/msg/Trade.h"
#include "ex/OrderBook.h"

struct DecodeHandler {
    DecodeHandler(ex::OrderBook& ob): orderBook(ob)
    {}

    void operator()(const ex::msg::NewOrder& obj) {
        orderBook.notify( obj );
        printOrderBook();
    }
    void operator()(const ex::msg::AmendOrder& obj) {
        orderBook.notify( obj );
        printOrderBook();
    }

    void operator()(const ex::msg::CancelOrder& obj) {
        orderBook.notify( obj );
        printOrderBook();
    }

    void operator()(const ex::msg::Trade& obj) {
        orderBook.notify( obj );
        printTrade(obj);
        printOrderBook();
    }

    ex::OrderBook& orderBook;
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
   
    std::cout << "Summary during exit" << std::endl;
    orderBook.print( std::cout, 5, true); 
    return 0;
}


