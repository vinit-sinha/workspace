#include <chrono>
#include <cassert>
#include <string>
#include <unordered_map>
#include <iostream>

#include "Listener.h"

enum class Side : char{
    Bid = 'B',
    Offer = 'O'
};

struct OrderInfo {
    Side side;
    double price;
    int quantity;

    OrderInfo(char s, double p, int q) 
        : side( static_cast<Side>(s) )
        , price( p )
        , quantity( q )
    {}

    OrderInfo(Side s, double p, int q) 
        : side( s )
        , price( p )
        , quantity( q)
    {}
    
};

struct ReplaceInfo {
    int newId;
    int deltaQty;
};

using OrderStore = std::unordered_map<int, OrderInfo>;
using PendingReplace = std::unordered_map<int, ReplaceInfo>;

struct NetFilledQuantity {
    NetFilledQuantity(const OrderStore& store)
        : orderStore(store)
    {}

    void notifyAck(int id ) {} //No-Op
    void notifyReject(int id ) {} // No-Op

    void notifyFill(int id, int qtyFilled ) {
        auto iter = orderStore.find( id );
        assert( iter != orderStore.end() );

        auto& orderInfo = iter->second;
        switch( orderInfo.side ) {
            case Side::Bid:
                val += qtyFilled;
                break;
            case Side::Offer:
                val -= qtyFilled;
                break;
        }
    }

    int value() const { return val; }
private:
    const OrderStore& orderStore;
    int val= 0;;
};

struct ConfirmedOrderValue {
    ConfirmedOrderValue(const OrderStore& store)
        : orderStore(store)
    {}
    void notifyAck(int id) {
        auto iter = orderStore.find( id );
        assert( iter != orderStore.end() );
        auto& order = iter->second;
        switch( order.side ) {
            case Side::Bid:
                bid += order.price * order.quantity;
                break;
            case Side::Offer:
                offer += order.price * order.quantity;
                break;
        }
    }

    void notifyFill(int id, int qtyFilled ) {
        auto iter = orderStore.find( id );
        assert( iter != orderStore.end() );
        auto& order = iter->second;
        switch( order.side ) {
            case Side::Bid:
                bid -= order.price * qtyFilled;
                break;
            case Side::Offer:
                offer -= order.price * qtyFilled;
                break;
        }
    }

    double bidValue() const { return bid; }
    double offerValue() const { return offer; }
private:
    const OrderStore& orderStore;
    double bid = 0.0;
    double offer = 0.0;
};

struct PendingOrderValue {
    PendingOrderValue(const OrderStore& store)
        : orderStore(store)
    {}
    void notifyInsert(const OrderInfo& order) {
        switch( order.side ) {
            case Side::Bid:
                bidMax += order.price * order.quantity;
                break;
            case Side::Offer:
                offerMax += order.price * order.quantity;
                break;
        }
    }
    void notifyAck(int id ) {
        auto iter = orderStore.find( id );
        assert( iter != orderStore.end() );
        auto& order = iter->second;
        switch( order.side ) {
            case Side::Bid:
                bidMin += order.price * order.quantity;
                break;
            case Side::Offer:
                offerMin += order.price * order.quantity;
                break;
        }
    }
    void notifyReject(int id ) {
    }
    void notifyFill(int id, int qtyFilled ) {
        auto iter = orderStore.find( id );
        assert( iter != orderStore.end() );
        auto& order = iter->second;
        switch( order.side ) {
            case Side::Bid:
                bidMin -= order.price * qtyFilled;
                bidMax -= order.price * qtyFilled;
                break;
            case Side::Offer:
                offerMin -= order.price * qtyFilled;
                offerMax -= order.price * qtyFilled;
                break;
        }
    }

    double bidMinValue() const { return bidMin; }
    double bidMaxValue() const { return bidMax; }
    double offerMinValue() const { return offerMin; }
    double offerMaxValue() const { return offerMax; }
private:
    const OrderStore& orderStore;
    double bidMin = 0.0;
    double bidMax = 0.0;
    double offerMin = 0.0;
    double offerMax = 0.0;
};

class OrderTracker: public Listener {
public:
    OrderTracker()
        : orders()
        , nfq(orders)
        , cov(orders)
        , pov(orders)
    {}

    void OnInsertOrderRequest(
        int id,
        char side,
        double price,
        int quantity
    );
    
    void OnReplaceOrderRequest(
        int oldId, // The existing order to modify
        int newId, // The new order ID to use if the modification succeeds
        int deltaQuantity
    );

    void OnRequestAcknowledged(
        int id
    );

    void OnRequestRejected(
        int id
    );

    void OnOrderFilled(
        int id,
        int quantityFilled
   );

    int netFilledQuantity() const {
        return nfq.value();
    }

    double confirmedBidValue() const {
        return cov.bidValue(); 
    }
    double confirmedOfferValue() const {
        return cov.offerValue();
    }
    double pendingBidMinValue() const {
        return pov.bidMinValue();
    }
    double pendingBidMaxValue() const {
        return pov.bidMaxValue();
    }
    double pendingOfferMinValue() const {
        return pov.offerMinValue();
    }
    double pendingOfferMaxValue() const {
        return pov.offerMaxValue();
    }
private:
   OrderStore orders;
   PendingReplace pendingReplaces;
   
   NetFilledQuantity nfq;
   ConfirmedOrderValue cov;
   PendingOrderValue pov;
};


// Pre-Condition: id is unique and has never been seen before
void OrderTracker::OnInsertOrderRequest(
    int id,
    char side,
    double price,
    int quantity
) {
    OrderInfo order(side, price, quantity);
    orders.emplace(id, order); 
    pov.notifyInsert( order );
}
    
void OrderTracker::OnReplaceOrderRequest(
    int oldId, // The existing order to modify
    int newId, // The new order ID to use if the modification succeeds
    int deltaQuantity
) {
    pendingReplaces.emplace( oldId, ReplaceInfo{newId, deltaQuantity});
}
OrderInfo mergeOrderInfo(const OrderInfo& order, const ReplaceInfo& replace) {
    return OrderInfo(order.side, order.price, order.quantity + replace.deltaQty );
}
void OrderTracker::OnRequestAcknowledged(
   int id
) {

    // Find if it was a pending replace.
    auto iter = pendingReplaces.find( id );
    if( iter != pendingReplaces.end() ) {
        auto& replaceInfo = iter->second;
        // Update Id to replaceInfo.newID and 
        // Update Quantity to replaceInfo.deltaQty + order.quantity and 
        auto orig_order_iter = orders.find( id );
        assert( orig_order_iter != orders.end() );
        orders.emplace( replaceInfo.newId, mergeOrderInfo( orig_order_iter->second, replaceInfo) );

        // Cleanup
        pendingReplaces.erase( iter );
        orders.erase( orig_order_iter );
    }
    nfq.notifyAck( id );
    cov.notifyAck( id );
    pov.notifyAck( id );
}

void OrderTracker::OnRequestRejected(
    int id
) {
    nfq.notifyReject( id );
    pov.notifyReject( id );
}

void OrderTracker::OnOrderFilled(
     int id,
     int quantityFilled
) {
    auto iter = orders.find( id );
    assert( iter != orders.end() );
    auto& order = iter->second;
    order.quantity -= quantityFilled;
    nfq.notifyFill( id, quantityFilled );
    cov.notifyFill( id, quantityFilled );
    pov.notifyFill( id, quantityFilled );
}
    
std::ostream& operator<<(std::ostream& out, const OrderTracker& ot) {
    out << "NFQ: " << ot.netFilledQuantity()
        << ", COV-Bid: " << ot.confirmedBidValue()
        << ", COV-Offer: " << ot.confirmedOfferValue()
        << ", POV-Bid-Min: " << ot.pendingBidMinValue() 
        << ", POV-Bid-Max: " << ot.pendingBidMaxValue() 
        << ", POV-Offer-Min: " << ot.pendingOfferMinValue() 
        << ", POV-Offer-Max: " << ot.pendingOfferMaxValue();

    return out;
}


// Test Program which tries the test cases provided in the assignment
int main() {
    OrderTracker ot;
    ot.OnInsertOrderRequest(1, 'B', 10.0, 10);
    std::cout << ot << std::endl;
    ot.OnRequestAcknowledged(1);
    std::cout << ot << std::endl;
    ot.OnInsertOrderRequest(2, 'O', 15.0, 25);
    std::cout << ot << std::endl;
    ot.OnRequestAcknowledged(2);
    std::cout << ot << std::endl;
    ot.OnOrderFilled(1, 5);
    std::cout << ot << std::endl;
    ot.OnOrderFilled(1, 5);
    std::cout << ot << std::endl;
    ot.OnReplaceOrderRequest(2, 3, 10);
    std::cout << ot << std::endl;
    ot.OnOrderFilled(2, 25);
    std::cout << ot << std::endl;
    ot.OnRequestRejected(3);
    std::cout << ot << std::endl;

    return 0;
}
