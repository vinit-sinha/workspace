#include <unordered_map>
#include <set>
#include <algorithm>
#include <iomanip>

#include "ex/msg/NewOrder.h"
#include "ex/msg/AmendOrder.h"
#include "ex/msg/CancelOrder.h"
#include "ex/msg/Trade.h"

namespace ex {
    struct OrderInfo {
        ex::type::OrderId orderId;
        ex::type::Quantity quantity;
        ex::type::Price price;
    };

    struct OrderBook {
        void notify(const ex::msg::NewOrder& obj);
        void notify(const ex::msg::AmendOrder& obj);
        void notify(const ex::msg::CancelOrder& obj);
        void notify(const ex::msg::Trade& obj);
        
        void print(std::ostream& out, std::size_t level, bool printHeader);
    private:
        struct DescendingPriceOrdering {
            bool operator()(const OrderInfo& lhs, const OrderInfo& rhs) {
                return lhs.price > rhs.price;
            }
        };

        struct AscendingPriceOrdering {
            bool operator()(const OrderInfo& lhs, const OrderInfo& rhs) {
                return lhs.price < rhs.price;
            }
        };
        using Buys = std::unordered_map<ex::type::ProductId, std::set<OrderInfo, DescendingPriceOrdering>>;
        using Sells = std::unordered_map<ex::type::ProductId, std::set<OrderInfo, AscendingPriceOrdering>>;
        using OrderIdToProductIdMap = std::unordered_map< ex::type::OrderId, ex::type::ProductId>;

        Buys buys;
        Sells sells;
        OrderIdToProductIdMap orderIdToProductIdMap;

        template<typename OrderSet>
        void amend(OrderSet& infoSet, const ex::msg::AmendOrder& obj) {
           auto orderId = obj.orderId;
           auto iter = std::find_if( infoSet.begin(), infoSet.end(), [&orderId](const OrderInfo& info) {
               return info.orderId == orderId;
           });

           if( iter != infoSet.end() ) {
               infoSet.erase( iter );
               OrderInfo newInfo;
               newInfo.orderId = obj.orderId;
               newInfo.price = obj.price;
               newInfo.quantity = obj.quantity;
 
               infoSet.emplace( newInfo );                
           } else {
                //TODO: Indicate Error
           }
        }

        template<typename OrderSet>
        void cancel(OrderSet& infoSet, const ex::msg::CancelOrder& obj) {
           auto orderId = obj.orderId;
           auto iter = std::find_if( infoSet.begin(), infoSet.end(), [&orderId](const OrderInfo& info) {
               return info.orderId == orderId;
           });

           if( iter != infoSet.end() ) {
               infoSet.erase( iter );
           } else {
                //TODO: Indicate Error
           }
        }

       template<typename Iter> 
       Iter bestPriceMatch(Iter b, Iter e, ex::type::Price price) {
               return std::find_if( b, e, [&price](const ex::OrderInfo& info) { return info.price == price; }); 
       }

       template<typename BuyOrderSet, typename SellOrderSet> 
       void execute(BuyOrderSet& buySet, SellOrderSet& sellSet, const ex::msg::Trade& obj) {
           ex::type::Quantity matchedQty = obj.quantity;
           auto iter = bestPriceMatch( buySet.begin(), buySet.end(), obj.price );
           while( matchedQty > 0 && iter != buySet.end() ) {
               if( iter->quantity >= matchedQty ) {
                   OrderInfo newInfo = *iter;
                   newInfo.quantity -= matchedQty;
                   buySet.erase( iter );
                   auto emplace_result = buySet.emplace( newInfo ); 
                   iter = emplace_result.first;
                   // ++iter;
                   matchedQty = 0;
               } else { // All quatity for this order can be executed. so can be removed from book
                   iter = buySet.erase( iter ); //Closed Order
                   matchedQty -= iter->quantity;
               }
               iter = bestPriceMatch( iter, buySet.end(), obj.price ); //Try the next bestMatch
           }
       }
    };
}
