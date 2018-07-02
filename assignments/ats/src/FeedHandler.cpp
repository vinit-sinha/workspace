#include <fstream>
#include <iostream>

#include "ex/msg/Decoder.h"
#include "ex/msg/NewOrder.h"
#include "ex/msg/AmendOrder.h"
#include "ex/msg/CancelOrder.h"
#include "ex/msg/Trade.h"
#include "ex/OrderBook.h"

struct DecodeHandler {
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
        printOrderBook();
    }

    ex::OrderBook orderBook;
    std::size_t msgCount = 0;

    void printOrderBook() {
        if( ++msgCount == 10 ) {
            orderBook.print(std::cout, 5, true );
            msgCount = 1;
        }
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
    DecodeHandler dh;

    ex::msg::Decoder<DecodeHandler> decoder(ifile, dh);

    while (decoder.hasMoreMessages()) {
        decoder.decode();
    }
    
    return 0;
}


