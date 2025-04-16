#include "Orderbook.hpp"
#include <string>
#include "Display.hpp"
#include <random>
#include <chrono>

int getRandomNumber2() {

    static std::random_device rd;

    static std::mt19937 engine(rd());

    static std::uniform_int_distribution<int> dist(0, 1);

    return dist(engine);
}

int getRandomNumber1000() {

    static std::random_device rd2;

    static std::mt19937 engine2(rd2());

    static std::uniform_int_distribution<uint32_t> dist2(1, 1000);

    return dist2(engine2);
}

void randomOrder(Orderbook& orderbook) {

    static uint64_t ids = 1;

    orderbook.AddOrder(std::make_shared<Order>(Order(
        getRandomNumber2() ? OrderType::GoodTillCancel : OrderType::FillAndKill,
        ids++,
        getRandomNumber2() ? Side::Buy : Side::Sell,
        getRandomNumber1000(),
        getRandomNumber1000()
    )));

    if (getRandomNumber1000() < 51) {
        orderbook.ModifyOrder(OrderModify(
            ids - 1,
            getRandomNumber2() ? Side::Buy : Side::Sell,
            getRandomNumber1000(),
            getRandomNumber1000()
        ));
    }

    if (getRandomNumber1000() < 100) {
        orderbook.CancelOrder(ids - 1);
    }

}


int main() {

    Orderbook orderbook;

    
    int ops = 1000000;
    /*
    auto start = std::chrono::high_resolution_clock::now();
    */
    for (int i = 0; i < ops; i++) {
        randomOrder(orderbook);
    }
    

    /*
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    ops = 1.15 * ops;

    std::cout << "time taken for " << ops << " operations: " << duration.count() << " microseconds" << '\n';
    std::cout << "avg time per operation: " << duration.count() / static_cast<double>(ops) << " microsec/op" << '\n';

    */

    return 0;

}