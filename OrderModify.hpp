#pragma once

#include <memory>

#include "TypeAlias.hpp"
#include "Side.hpp"
#include "OrderType.hpp"
#include "Order.hpp"


class OrderModify { // this is the format for a request to modify an order. it is passed to the orderbook class 

    public:
        OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
            : orderId_ { orderId }
            , price_ { price }
            , side_ { side }
            , quantity_ { quantity }
        { }

        OrderId GetOrderId() const { return orderId_; }
        Price GetPrice() const { return price_; }
        Side GetSide() const { return side_; }
        Quantity GetQuantity() const { return quantity_; }

        OrderPointer ToOrderPointer(OrderType type) const {
            
            return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());

        }

    private:
        OrderId orderId_;
        Price price_;
        Side side_;
        Quantity quantity_;

};