#pragma once

#include "Orderbook.hpp"
#include <iomanip>
#include <sstream>
#include "main/color.hpp"

void clear() {
    system("clear");
}

std::string displayLevelData(Orderbook& orderbook) {

    // OrderbookLevelInfos -> bids, asks pointers -> vector<levelInfo> -> level info -> price, quantity

    OrderbookLevelInfos infos = orderbook.GetOrderInfos();

    std::stringstream stream;

    stream << std::endl;

    stream << "        OrderBook:" << std::endl << std::endl;

    for (int i = infos.GetBids().size() - 1; i > -1; i--) {
        stream << "|" << std::setw(10 + 9) << std::right << makeGreen(std::to_string(infos.GetBids()[i].price_));
        stream << " | " << std::setw(10) << std::left << infos.GetBids()[i].quantity_<< "|" << std::endl;
    }

    stream << "_____________________________________________" << std::endl << std::endl; // 20 spaces for percentage of orderbook

    for (LevelInfo level : infos.GetAsks()) {
        stream << "|" << std::setw(10 + 9) << std::right << makeRed(std::to_string(level.price_)) << " | " << std::setw(10) << std::left << level.quantity_ << "|" << std::endl;
    }

    return stream.str();

}

void addDummyOrders(Orderbook& Orderbook) {

    OrderId orderIds = 0;
    Price price = 100;
    OrderPointer tempOrder;

    for (int i = 0; i < 10; i++) {

        tempOrder = std::make_shared<Order>(Order(OrderType::GoodTillCancel, orderIds, Side::Buy, price, 1000));

        orderIds++;
        price += 100;

        Orderbook.AddOrder(tempOrder);

    }

    price = 1100;

    
    for (int i = 0; i < 10; i++) {

        tempOrder = std::make_shared<Order>(Order(OrderType::GoodTillCancel, orderIds, Side::Sell, price, 1000));

        orderIds++;
        price += 100;

        Orderbook.AddOrder(tempOrder);

    }

}