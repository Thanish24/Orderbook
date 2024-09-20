#include <iostream>
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <limits>
#include <string>
#include <vector>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <format>

enum class OrderType {
    GoodTillCancel, // stays in the orderbook until executed
    FillAndKill // if it cant be filled immediately, cancel it
};

enum class Side {
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

struct LevelInfo { // the orderbook contains levels that have a list of bids and asks at a certain price. This contains the price of the level and how much is in the level 
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderbookLevelInfos { // this contains information of the different levels on the bid and ask sides of the orderbook- it contains 2 vectors for each side, 
                                // each containing the price of the level and the quantity at that price

    public:
        OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
            : bids_{ bids }
            , asks_{ asks }
        { }

        const LevelInfos& GetBids() const { return bids_; }
        const LevelInfos& GetAsks() const { return asks_; }

    private:
        LevelInfos bids_;
        LevelInfos asks_;

};

class Order {
    
    public:
        Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
            : orderType_ { orderType }
            , orderId_ { orderId }
            , side_ { side }
            , price_ { price }
            , initialQuantity_ { quantity }
            , remainingQuantity_ { quantity }
        { }

        OrderId GetOrderId() const { return orderId_; }
        Side GetSide() const { return side_; }
        Price GetPrice() const { return price_; }
        Quantity GetInitialQuantity() const { return initialQuantity_; }
        Quantity GetRemainingQuantity() const { return remainingQuantity_; }
        Quantity GetFilledQuantity() const { return initialQuantity_ - remainingQuantity_; }

        bool IsFilled() const { return GetRemainingQuantity() == 0; }
 
        void Fill(Quantity quantity) {
            if (quantity > GetRemainingQuantity()) {
                throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderId()));
            }

            remainingQuantity_ -= quantity;
        }

    private:
        OrderType orderType_;
        OrderId orderId_;
        Side side_;
        Price price_;
        Quantity initialQuantity_;
        Quantity remainingQuantity_;

};

using OrderPointer std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;

class OrderModify { // this is the format for a request to modify an order. it is passed to the orderbook class 

    public:
        OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
            : orderId_ { orderId; }
            , price_ { price; }
            , side_ { side; }
            , quantity_ { quantity; }
        { }

        OrderId GetOrderId() const { return orderId_; }
        OrderId GetPrice() const { return price_; }
        OrderId GetSide() const { return side_; }
        OrderId GetQuantity() const { return quantity_; }

        OrderPointer ToOrderPointer(OrderType type) const {
            
            return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());

        }

    private:
        OrderId orderId_;
        Price price_;
        Side side_;
        Quantity quantity_;

};

struct TradeInfo {
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
};

class Trade { // a trade contains info about the orders involved in the trade (buy and sell)
    
    public:
        Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
            : bidTrade_ { bidTrade }
            , askTrade_ { askTrade }
        { }

        const TradeInfo& GetBidTrade() const { return bidTrade_; }
        const TradeInfo& GetAskTrade() const { return askTrade_; }

    private:
        TradeInfo bidTrade_;
        TradeInfo askTrade_;

};

using Trades = std::vector<Trade>;

Class Orderbook {
    
    private:
        
        struct OrderEntry {
            OrderPointer order_ { nullptr };
            OrderPointers::iterator location_;
        };

        std::map<Price, OrderPointers, std::greater<Price>> bids_; // map of bid levels sorted by price, containing a list of pointers to their locations in the orders map
        std::map<Price, OrderPointers, std::less<Price>> asks_; // map of ask levels sorted by price
        std::unordered_map<OrderId, OrderEntry> orders_; // map of all orders by order id to easily find location of orders in the bid and ask maps

        bool CanMatch(Side side, Price price) const { // checks if a bid/ask is possible. if the bid/ask price is too low/high then it returns false
            
            if (side == Side::Buy) {
                if (asks_.empty()) return false;

                const auto& [bestAsk, _] = *asks_.begin();
                return price >= bestAsk;
            } else {

                if (bids_.empty()) return false;

                const auto& [bestBid, _] = *bids_.begin();
                return price <= bestBid;

            }

        }

        Trades MatchOrders() { // makes trades and returns them in a vector of trades on price time priority
            Trades trades;
            trades.reserve(orders_.size()); // at max when orders are matched the entire orderbook can be cleared

            while (true) { // goes accross levels
                if (bids.empty() ||  asks.empty()) break;

                auto& [bidPrice, bids] = *bids.begin(); // get the level of lowest bids and the price of that level
                auto& [askPrice, asks] = *asks.begin(); // get the level of the lowest ask and the price of that ask

                if (bidPrice < askPrice) break; // continue if people are asking to buy for equal to or more what people are selling for 

                while (bids.size() && asks.size()) { // while the level has both bids and asks go through the level
                    auto bid = bids.front(); // get the first bid in the level
                    auto ask = asks.front();

                    bid->Fill(quantity);
                    ask->Fill(quantity);

                    if (bid->IsFilled()) {
                        bids.pop_front();
                        orders_.erase(bid->GetOrderId);
                    }

                    if (ask->IsFilled()) {
                        asks.pop_front();
                        orders_.erase(ask->GetOrderId());
                    }

                    if (bids.empty()) bids_.erase(bidPrice);

                    if (asks.empty()) asks.erase(askPrice);

                    trades.push_back(
                        Trade( TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity}, TradeInfo{ ask->GetOrderId(), ask->GetPrice(), quantity} ) // create an instance of structs for the trade made
                    );

                }

            }

            // cancel the orders that are still pending after matching if they are fill and kill

            if (!bids_.empty()) {
                auto& [_, bids] = *bids_.begin();
                auto& order = bids.front();

                if (order->GetOrderType() == OrderType::FillAndKill) CancelOrder(order->GetOrderId());
            }

            if (!asks_.empty()) {
                auto& [_, asks] = *asks_.begin();
                auto& order = asks.front();

                if (order->GetOrderType() == OrderType::FillAndKill) CancelOrder(order->GetOrderId());
            }

            return trades;

        }
    
    public:

        Trades AddOrder(OrderPointer order) {
            
            if (orders_.contains(order->GetOrderId())) return { }; // you cant add an order to the orderbook thats already there
            

            if (order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), Order->GetPrice())) return { }; // if the fill and kill wont work then dont add it

            OrderPointers::iterator iterator;

            if (order->GetSide() == Side::Buy) {
                auto& orders = bids_[order->GetPrice()]; // get a reference to the level; c++ creates the level if it doesnt already exist
                orders.push_back(order);
                iterator = std::next(orders.begin(), orders.size()); // pointer to the order in the side
            } else {
                auto& orders = asks_[order->GetPrice()];
                orders.push_back(order);
                iterator = std::next(orders.begin(), orders.size());
            }

            orders_.insert({order->GetOrderId(), OrderEntry{ order, iterator}}); // adds to list of order
            
            return MatchOrders(); // update market whenever an order is added

        }

        void CancelOrder(OrderId orderId) {
            if (!orders_.contains(orderId)) return; // cant cancel an order that isnt in the book

            const auto& [order, orderIterator] = orders_.at(orderId);
            orders_.erase(orderId); // remove from order list

            if (order->GetSide() == Side::sell) {
                auto price = order->GetPrice(); // get price of order to find level
                auto& orders = asks_.at(price); // remove from level
                orders.erase(orderIterator);
                if (orders.empty()) asks_.erase(price); // erase the level if there isnt anything in it
            } else {
                auto price = order->GetPrice();
                auto& orders = bids_.at(price);
                orders.erase(orderIterator)
                if (orders.empty()) asks_.erase(price);
            }
        }

        Trades MatchOrder(OrderModify order) { // modifies order in book
            
            if (!orders_.contains(order.GetOrderId())) return {};

            const auto& [existingOrder, _] = orders_.at(order.GetOrderId); // gets the order 

            CancelOrder(existingOrder.GetOrderId());

            return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
        }

        std::size_t Size() const { return orders_.size(); } // returns how many orders are in orderbook

        OrderbookLevelInfos GetOrderInfos() const { // get information about each level in the orderbook
            LevelInfos bidInfos, askInfos; // arrays of infos: each info has the level price and quantity in that level

            bidInfos.reserve(orders_.size());
            askInfos.reserve(orders_.size());

            auto CreateLevelInfos = [](Price price, const OrderPointers& Orders) { // lambda expression to create the infos, takes the price of the level and the orders of the level

                return LevelInfo{ price, std::accumulate(orders.begin(), orders.end()), (Quantity)0, // makes a struct for the level info 
                [](std::size_t runningSum, const OrderPointer& order) // TODO: label functionality later
                    { return runningSum + order->GetRemainingQuantity();}
                };

            };

            for (const auto& [price, orders] : bids_) // iterates through each level of bids
                bidInfos.push_back(CreateLevelInfos(price, orders)); // creates a level info struct for the level and adds it to the list

            for (const auto& [price, orders] : asks_)
                askInfos.push_back(CreateLevelInfos(price, orders));

            return OrderbooksLevelInfos{ bidInfos, askInfos };
        }

};

int main() {

    return 0;

}