#pragma once

#include "TypeAlias.hpp"

struct LevelInfo { // the orderbook contains levels that have a list of bids and asks at a certain price. This contains the price of the level and how much is in the level 
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;