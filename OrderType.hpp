#pragma once

enum class OrderType {
    GoodTillCancel, // stays in the orderbook until executed
    FillAndKill, // fill as much as possible and then cancel the order
};