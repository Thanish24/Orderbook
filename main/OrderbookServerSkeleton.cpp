#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "color.hpp"
#include "../Orderbook.hpp"
#include "../Display.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include <vector>

using boost::asio::ip::tcp;
using std::string;







void orderBookConnection(tcp::socket &socket, Orderbook &orderbook) {

    try {

        int orderId = 0;

        while (true) {

            // holds the clients message
            char data[1024];



            // read message from client
            boost::system::error_code error;
            size_t length = socket.read_some(boost::asio::buffer(data), error);



            // handle errors from reading from the client
            if (error == boost::asio::error::eof) {
                std::cout << "Client disconnected" << std::endl;
                break;
            } else if (error) {
                throw boost::system::system_error(error);
            }

            std::string response(data, length);
            
            boost::system::error_code ignored_error;

            // parse message

            std::vector<std::string> response_arr;

            boost::split(response_arr, response, boost::is_any_of(","));

            // action
            std::string message = response_arr[0];


            // process request

            if (message == "buy") {
                std::cout << makeGreen("Recieved buy order") << std::endl;

                std::string type = response_arr[1];
                int price = boost::lexical_cast<int>(response_arr[2]);
                int quantity = boost::lexical_cast<int>(response_arr[3]);

                OrderType t;

                if (type == "gtc") {
                    t = OrderType::GoodTillCancel;
                } else if (type == "fok") {
                    t = OrderType::FillAndKill;
                } else {
                    t = OrderType::FillAndKill;
                }

                Trades trades = orderbook.AddOrder(std::make_shared<Order>(Order(t, orderId++, Side::Buy, price, quantity)));

                std::string ifTradeMade = "";

                if (trades.size() != 0) {
                    ifTradeMade = "Trades made: \n";

                    for (Trade &trade : trades) {

                        TradeInfo bid = trade.GetBidTrade();
                        TradeInfo ask = trade.GetAskTrade();

                        ifTradeMade +=  "ids: " + std::to_string(bid.orderId_) + "," + std::to_string(ask.orderId_);
                        
                        ifTradeMade += " quantity: " + std::to_string(bid.quantity_) + " price: " + std::to_string(bid.price_) + "\n";

                    }

                    std::cout << ifTradeMade << std::endl;

                }

                boost::asio::write(socket, boost::asio::buffer(displayLevelData(orderbook) + ifTradeMade), error);

            } else if (message == "sell") {
                std::cout << makeRed("Recieved sell order") << std::endl;
                
                std::string type = response_arr[1];
                int price = boost::lexical_cast<int>(response_arr[2]);
                int quantity = boost::lexical_cast<int>(response_arr[3]);

                OrderType t;

                if (type == "gtc") {
                    t = OrderType::GoodTillCancel;
                } else if (type == "fok") {
                    t = OrderType::FillAndKill;
                } else {
                    t = OrderType::FillAndKill;
                }

                Trades trades = orderbook.AddOrder(std::make_shared<Order>(Order(t, orderId++, Side::Sell, price, quantity)));

                std::string ifTradeMade = "";

                if (trades.size() != 0) {
                    ifTradeMade = "Trades made: \n";
                    
                    for (Trade &trade : trades) {

                        TradeInfo bid = trade.GetBidTrade();
                        TradeInfo ask = trade.GetAskTrade();

                        ifTradeMade +=  "ids: " + std::to_string(bid.orderId_) + "," + std::to_string(ask.orderId_);
                        
                        ifTradeMade += " quantity: " + std::to_string(bid.quantity_) + " price: " + std::to_string(bid.price_) + "\n";
   
                    }

                    std::cout << ifTradeMade << std::endl;
                }

                

                boost::asio::write(socket, boost::asio::buffer(displayLevelData(orderbook) + ifTradeMade), error);

            } else if (message == "cancel") {
                std::cout << "Recieved cancel order for id " << response_arr[1] << std::endl;

                orderbook.CancelOrder(boost::lexical_cast<int>(response_arr[1]));

                boost::asio::write(socket, boost::asio::buffer(displayLevelData(orderbook)), error);

            } else if (message == "modify") {
                std::cout << "Recieved order modify" << std::endl;

                boost::asio::write(socket, boost::asio::buffer("modified"), error);
            } else if (message == "init") {
                std::cout << "Recieved init" << std::endl;

                addDummyOrders(orderbook);

                boost::asio::write(socket, boost::asio::buffer(displayLevelData(orderbook)), error);                

            } else if (message == "display") {
                std::cout << "Recieved request to display orderbook" << std::endl;

                boost::asio::write(socket, boost::asio::buffer(displayLevelData(orderbook)), error);
            } else {
                std::cout << "Recieved unknown query" << std::endl;

                boost::asio::write(socket, boost::asio::buffer("could not understand query"), error);
            }

            std::cout << std::endl;

        }

    } catch (std::exception &e) {

        std::cerr << "Exception: " << e.what() << std::endl;

    }

}







int main() {

    try {
        boost::asio::io_context io_context;

        // tcp acceptor on port 8080
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

        // loop to accept connections
        while (true) {
            
            // accept connection on socket
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            // create new orderbook
            Orderbook orderbook;

            // write confirmation message:
            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(displayLevelData(orderbook)), ignored_error);
            std::cout << "sending start message" << std::endl;


            // handle actions
            orderBookConnection(socket, orderbook);

        }

    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }


    return 0;
}