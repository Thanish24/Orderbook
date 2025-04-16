#pragma once

#include "LevelInfo.hpp"

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