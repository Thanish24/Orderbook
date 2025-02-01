#include <boost/asio.hpp>
#include <array>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

void orderbookClientHandle(tcp::socket &socket) {

    try {

        while (true) {
            
            // gets command from terminal
            std::string command;

            std::cout << "Enter a command: ";
            std::cin >> command;

            boost::system::error_code ignored_error;

            // send command
            boost::asio::write(socket, boost::asio::buffer(command), ignored_error);

            // recieve message

            char data[1024];

            boost::system::error_code error;
            size_t length = socket.read_some(boost::asio::buffer(data), error);

            // handle errors from reading from the server
            if (error == boost::asio::error::eof) {
                std::cout << "Server disconnected" << std::endl;
                break;
            } else if (error) {
                throw boost::system::system_error(error);
            }  

            std::string response(data, length);

            std::system("clear");

            // write response
            std::cout << response << std::endl << std::endl;     

        }

    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

}

int main() {

    try {

        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);

        tcp::resolver::results_type endpoints = resolver.resolve("localhost", "8080");

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        while (true) {

            // get confirmation message

            std::array<char, 128> buf;
            boost::system::error_code error;

            // read data from server into buffer
            size_t len = socket.read_some(boost::asio::buffer(buf), error);

            // process possible errors
            if (error == boost::asio::error::eof) {
                break;
            } else if (error) {
                throw boost::system::system_error(error);
            }

            // print what was recieved
            std::cout.write(buf.data(), len);
            std::cout << std::endl;
            std::cout << std::endl;



            // handle sending commands to the orderbook
            orderbookClientHandle(socket);


        }

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }


}