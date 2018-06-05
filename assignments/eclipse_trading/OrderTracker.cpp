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

// OrderInfo is used to hold all parameters associated with an Insert Order Request
struct OrderInfo {
    Side side;
    double price;
    int quantity;

    OrderInfo() = default;
    OrderInfo(const OrderInfo&) = default;
    OrderInfo(OrderInfo&&) = default;

    OrderInfo& operator=(const OrderInfo&) = default;
    OrderInfo& operator=(OrderInfo&&) = default;

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

// ReplaceInfo is used to hold all parameters associated with an Insert Order Request
struct ReplaceInfo {
    int newId;
    int deltaQty;
};

using OrderStore = std::unordered_map<int, OrderInfo>;
using PendingReplace = std::unordered_map<int, ReplaceInfo>;


// A Simple Accumulator class which "Accumulates" Net Filled Quantity.
//  In order to keep extracting this value efficient, we keep track of 
//  NFQ known so far. notifyFill does the most of work by re-computing
//   (and hence accumulating) NFQ.
struct NetFilledQuantity {
    NetFilledQuantity(const OrderStore& store)
        : orderStore(store)
    {}

    void notifyAck(int id ) {} //No-Op
    void notifyReject(int id ) {} // No-Op

    // TimeComplexity: O(1)  - Amortized Cost
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

    int value() const { return val; } // TimeComplexity: Theta(1)
private:
    const OrderStore& orderStore;
    int val= 0;;
};

// A Simple Accumulator class which "Accumulates" Confirmed Order Value.
//  In order to keep extracting this value efficient, we keep track of 
//  COV for bid and offer sides separately. 
//  notifyAck and notifyFill does the most of work by re-computing
//   (and hence accumulating) COV.
struct ConfirmedOrderValue {
    ConfirmedOrderValue(const OrderStore& store)
        : orderStore(store)
    {}
   
    // TimeComplexity: O(1)  - Amortized Cost
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

    // TimeComplexity: O(1)  - Amortized Cost
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

    double bidValue() const { return bid; } // TimeComplexity: Theta(1)
    double offerValue() const { return offer; } // TimeComplexity: Theta(1)
private:
    const OrderStore& orderStore;
    double bid = 0.0;
    double offer = 0.0;
};

// A Simple Accumulator class which "Accumulates" Pending Order Value.
//  In order to keep extracting this value efficient, we keep track of 
//  POV for bid and offer sides separately. Each of side has two values
//  one for min and another for max.
//  notifyInsert, notifyAck and notifyFill does the most of work by re-computing
//   (and hence accumulating) COV.
struct PendingOrderValue {
    PendingOrderValue(const OrderStore& store)
        : orderStore(store)
    {}

    // TimeComplexity: Theta(1)
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

    // TimeComplexity: O(1)  - Amortized Cost
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

    // TimeComplexity: Theta(1)
    void notifyReject(int id ) {} //No-Op

    // TimeComplexity: O(1)  - Amortized Cost
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

    double bidMinValue() const { return bidMin; } // TimeComplexity: Theta(1)
    double bidMaxValue() const { return bidMax; } // TimeComplexity: Theta(1)
    double offerMinValue() const { return offerMin; } // TimeComplexity: Theta(1)
    double offerMaxValue() const { return offerMax; } // TimeComplexity: Theta(1)
private:
    const OrderStore& orderStore;
    double bidMin = 0.0;
    double bidMax = 0.0;
    double offerMin = 0.0;
    double offerMax = 0.0;
};


// OrderTracker class implemets the Listener interface and provides functions to fetch
//  Net Filled Quantity
//  Confirmed Order Value, and
//  Pending Order Value
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

    // TimeComplexity: Theta(1)
    int netFilledQuantity() const {
        return nfq.value();
    }
    
    // TimeComplexity: Theta(1)
    double confirmedBidValue() const {
        return cov.bidValue(); 
    }

    // TimeComplexity: Theta(1)
    double confirmedOfferValue() const {
        return cov.offerValue();
    }

    // TimeComplexity: Theta(1)
    double pendingBidMinValue() const {
        return pov.bidMinValue();
    }

    // TimeComplexity: Theta(1)
    double pendingBidMaxValue() const {
        return pov.bidMaxValue();
    }

    // TimeComplexity: Theta(1)
    double pendingOfferMinValue() const {
        return pov.offerMinValue();
    }
    
    // TimeComplexity: Theta(1)
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
// TimeComplexity: O(1) - Amortized cost
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
    
// TimeComplexity: O(1) - Amortized cost
void OrderTracker::OnReplaceOrderRequest(
    int oldId, // The existing order to modify
    int newId, // The new order ID to use if the modification succeeds
    int deltaQuantity
) {
    pendingReplaces.emplace( oldId, ReplaceInfo{newId, deltaQuantity});
}

//Simple Utility to merge values of OrderInfo to ReplaceInfo
OrderInfo mergeOrderInfo(const OrderInfo& order, const ReplaceInfo& replace) {
    return OrderInfo(order.side, order.price, order.quantity + replace.deltaQty );
}

// TimeComplexity: O(1)- Amortized Cost
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

// TimeComplexity: O(1)
void OrderTracker::OnRequestRejected(
    int id
) {
    nfq.notifyReject( id );
    pov.notifyReject( id );
}

// TimeComplexity: O(1)
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
