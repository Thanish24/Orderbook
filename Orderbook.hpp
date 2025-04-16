#pragma once

#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <iostream>

#include "Side.hpp"
#include "OrderType.hpp"
#include "TypeAlias.hpp"
#include "LevelInfo.hpp"
#include "OrderbookLevelInfos.hpp"
#include "Order.hpp"
#include "OrderModify.hpp"
#include "TradeInfo.hpp"
#include "Trade.hpp"

class Orderbook {
    
    private:
        
        struct OrderEntry {
            OrderPointer order_ { nullptr };
            OrderPointers::iterator location_;
        };

        std::map<Price, OrderPointers, std::greater<Price>> bids_; // map of bid levels sorted by price, containing a list of pointers for each order DESCENDING ORDER
        std::map<Price, OrderPointers, std::less<Price>> asks_; // map of ask levels sorted by price ASCENDING ORDER
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

            while (true) { // goes across levels
                if (bids_.empty() ||  asks_.empty()) break;

                auto& [bidPrice, bids] = *bids_.begin(); // get the level of lowest bids and the price of that level
                auto& [askPrice, asks] = *asks_.begin(); // get the level of the lowest ask and the price of that ask

                if (bidPrice < askPrice) break; // continue if people are asking to buy for equal to or more what people are selling for 

                while (!bids.empty() && !asks.empty()) { // while the level has both bids and asks go through the level
                    auto bid = bids.front(); // get the first bid in the level
                    auto ask = asks.front();

                    Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

                    bid->Fill(quantity);
                    ask->Fill(quantity);

                    if (bid->IsFilled()) {
                        bids.pop_front();
                        orders_.erase(bid->GetOrderId());
                    }

                    if (ask->IsFilled()) {
                        asks.pop_front();
                        orders_.erase(ask->GetOrderId());
                    }

                    if (bids.empty()) bids_.erase(bidPrice);

                    if (asks.empty()) asks_.erase(askPrice);

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
            

            if (order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice())) return { }; // if the fill and kill wont work then dont add it

            OrderPointers::iterator iterator;

            if (order->GetSide() == Side::Buy) {
                auto& orders = bids_[order->GetPrice()]; // get a reference to the level; c++ creates the level if it doesnt already exist
                orders.push_back(order);
                iterator = orders.end();
                iterator--;
                //iterator = std::next(orders.begin(), orders.size()); // pointer to the order in the side
            } else {
                auto& orders = asks_[order->GetPrice()];
                orders.push_back(order);
                iterator = orders.end();
                iterator--;
                //iterator = std::next(orders.begin(), orders.size());
            }

            orders_.insert({order->GetOrderId(), OrderEntry{ order, iterator}}); // adds to list of order
            
            return MatchOrders(); // update market whenever an order is added

        }

        void CancelOrder(OrderId orderId) {
            if (!orders_.contains(orderId)) return; // cant cancel an order that isnt in the book
            
            const auto& [order, orderIterator] = orders_.at(orderId);
            orders_.erase(orderId); // remove from order list

            if (order->GetSide() == Side::Sell) {
                auto price = order->GetPrice(); // get price of order to find level
                auto& orders = asks_.at(price); // remove from level
                orders.erase(orderIterator);
                if (orders.empty()) asks_.erase(price); // erase the level if there isnt anything in it
            } else {
                auto price = order->GetPrice();
                auto& orders = bids_.at(price);
                orders.erase(orderIterator); // problem
                if (orders.empty()) bids_.erase(price);
            }
        }

        Trades ModifyOrder(OrderModify order) { // modifies order in book
            
            if (!orders_.contains(order.GetOrderId())) return {};

            const auto& [existingOrder, _] = orders_.at(order.GetOrderId()); // gets the order 

            CancelOrder(existingOrder->GetOrderId());

            return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
        }

        std::size_t Size() const { return orders_.size(); } // returns how many orders are in orderbook

        OrderbookLevelInfos GetOrderInfos() const { // get information about each level in the orderbook
            LevelInfos bidInfos, askInfos; // arrays of infos: each info has the level price and quantity in that level

            bidInfos.reserve(orders_.size());
            askInfos.reserve(orders_.size());

            auto CreateLevelInfos = [](Price price, const OrderPointers& orders) { // lambda expression to create the infos, takes the price of the level and the orders of the level

				Quantity levelQuantity = 0;

				for (OrderPointer order : orders) {
					levelQuantity += order->GetRemainingQuantity();
				}

                return LevelInfo{ price, levelQuantity};

            };

            for (const auto& [price, orders] : bids_) // iterates through each level of bids
                bidInfos.push_back(CreateLevelInfos(price, orders)); // creates a level info struct for the level and adds it to the list

            for (const auto& [price, orders] : asks_)
                askInfos.push_back(CreateLevelInfos(price, orders));

            return OrderbookLevelInfos{ bidInfos, askInfos };
        }

};