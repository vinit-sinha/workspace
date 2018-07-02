#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include "ex/msg/NewOrder.h"
#include "ex/msg/AmendOrder.h"
#include "ex/msg/CancelOrder.h"
#include "ex/msg/Trade.h"
#include "ex/state/OrderInfo.h"

namespace ex {
    struct OrderBook {
        ex::type::ErrorCode notify(const ex::msg::NewOrder& obj);
        ex::type::ErrorCode notify(const ex::msg::AmendOrder& obj);
        ex::type::ErrorCode notify(const ex::msg::CancelOrder& obj);
        ex::type::ErrorCode notify(const ex::msg::Trade& obj);
        
        void print(std::ostream& out, std::size_t level, bool printHeader);
        std::pair<ex::type::Price, ex::type::Quantity> getLastTradedPriceAndQuantiity(ex::type::ProductId productId) {
            return lastTradedPriceAndQuantity[productId];
        }
    private:
        using Buys = std::unordered_map<ex::type::ProductId, std::set<ex::state::OrderInfo, ex::state::DescendingPriceOrdering>>;
        using Sells = std::unordered_map<ex::type::ProductId, std::set<ex::state::OrderInfo, ex::state::AscendingPriceOrdering>>;
        using OrderIdToProductIdMap = std::unordered_map< ex::type::OrderId, ex::type::ProductId>;

        Buys buys;
        Sells sells;
        OrderIdToProductIdMap orderIdToProductIdMap;
        std::unordered_set<ex::type::ProductId> products;
        std::unordered_map<ex::type::ProductId, std::pair<ex::type::Price, ex::type::Quantity>> lastTradedPriceAndQuantity;

        bool orderExists(ex::type::OrderId orderId) const {
            return orderIdToProductIdMap.find( orderId ) != orderIdToProductIdMap.end();
        }
        template<typename OrderSet>
        ex::type::ErrorCode amend(OrderSet& infoSet, const ex::msg::AmendOrder& obj) {
           auto orderId = obj.orderId;
           auto iter = std::find_if( infoSet.begin(), infoSet.end(), [orderId](const ex::state::OrderInfo& info) {
               return info.orderId == orderId;
           });

           if( iter == infoSet.end() ) return ex::type::ErrorCode::InvalidOrderId;

           infoSet.erase( iter );
           ex::state::OrderInfo newInfo;
           newInfo.orderId = obj.orderId;
           newInfo.price = obj.price;
           newInfo.quantity = obj.quantity;
 
           infoSet.emplace( newInfo );

           return ex::type::ErrorCode::Ok;
        }

        template<typename OrderSet>
        ex::type::ErrorCode cancel(OrderSet& infoSet, const ex::msg::CancelOrder& obj) {
           ex::type::ErrorCode err = ex::type::ErrorCode::Ok;

           auto orderId = obj.orderId;
           auto iter = std::find_if( infoSet.begin(), infoSet.end(), [orderId](const ex::state::OrderInfo& info) {
               return info.orderId == orderId;
           });

           if( iter != infoSet.end() ) {
               infoSet.erase( iter );
           } else {
              err = ex::type::ErrorCode::InvalidOrderId; 
           }

            return err;
        }

       template<typename Iter> 
       Iter bestPriceMatch(Iter b, Iter e, ex::type::Price price) {
                // TODO: Implement our own verson of find_if for a reasonable "closet" match (which requires defining "closest" )
               return std::find_if( b, e, [&price](const ex::state::OrderInfo& info) { return info.price == price; }); 
       }
 
       template<typename OrderSet> 
       void execute(OrderSet& orderSet, const ex::msg::Trade& obj) {
           ex::type::Quantity remainingQty = obj.quantity;
           auto iter = bestPriceMatch( orderSet.begin(), orderSet.end(), obj.price );
           while( remainingQty > 0 && iter != orderSet.end() ) {
               if( iter->quantity >= remainingQty ) {
                   ex::state::OrderInfo newInfo = *iter;
                   newInfo.quantity -= remainingQty;
                   orderSet.erase( iter );
                   auto emplace_result = orderSet.emplace( newInfo ); 
                   iter = emplace_result.first;
                   remainingQty = 0;
               } else { // All quatity for this order can be executed. so quantity for this order needs to be set to 0
                   remainingQty -= iter->quantity;

                   ex::state::OrderInfo newInfo = *iter;
                   newInfo.quantity = 0;

                   orderSet.erase( iter );
                   auto emplace_result = orderSet.emplace( newInfo ); 
                   iter = emplace_result.first;
               }
               iter = bestPriceMatch( ++iter, orderSet.end(), obj.price ); //Try the next bestMatch
           }
        }

       template<typename BuyOrderSet, typename SellOrderSet> 
        void execute(BuyOrderSet& buySet, SellOrderSet& sellSet, const ex::msg::Trade& obj) {
           execute( buySet, obj);
           execute( sellSet, obj);
           auto& priceQtyPair = lastTradedPriceAndQuantity[obj.productId];
           if( priceQtyPair.first == obj.price ) { //Update Quantity
               priceQtyPair.second += obj.quantity;
           } else { //Reset Price and Quantity
               priceQtyPair.first = obj.price;
               priceQtyPair.second = obj.quantity;
           }
       }
    };
}
