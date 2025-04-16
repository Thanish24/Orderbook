#pragma once

#include <list>
#include <stdexcept>
#include <string>
#include <memory>

#include "OrderType.hpp"
#include "Side.hpp"
#include "TypeAlias.hpp"


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
        OrderType GetOrderType() const { return orderType_; }
        Side GetSide() const { return side_; }
        Price GetPrice() const { return price_; }
        Quantity GetInitialQuantity() const { return initialQuantity_; }
        Quantity GetRemainingQuantity() const { return remainingQuantity_; }
        Quantity GetFilledQuantity() const { return initialQuantity_ - remainingQuantity_; }

        bool IsFilled() const { return GetRemainingQuantity() == 0; }
 
        void Fill(Quantity quantity) {
            if (quantity > GetRemainingQuantity()) {

                

                throw std::logic_error("Order " + std::to_string(GetOrderId()) + " cannot be filled for more than its remaining quantity");
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

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;