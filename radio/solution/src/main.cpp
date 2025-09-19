

#include <boost/asio.hpp>

#include "audio.h"
#include <iostream>
#include <string>
#include <string_view>

namespace net = boost::asio;

using net::ip::udp;

using namespace std::literals;

static const size_t max_buffer_size = 65000;

#include "audio.h"
#include <iostream>

using namespace std::literals;

void StartServer(uint16_t port) {

    try {
        
        boost::asio::io_context io_context;

        udp::socket socket(io_context, udp::endpoint(udp::v4(), port));

        // «апускаем сервер в цикле, чтобы можно было работать со многими клиентами

        Player player(ma_format_u8, 1);

        for (;;) {

            // —оздаЄм буфер достаточного размера, чтобы вместить датаграмму.
            std::vector<char> recv_buf;

            recv_buf.resize(max_buffer_size);

            udp::endpoint remote_endpoint;

             socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

            //уверенно пытаемс€ воспроизвести

            auto s = player.GetFrameSize();

            long int f = recv_buf.size() / s;

            player.PlayBuffer(recv_buf.data(), 65000, 1.5s);

            std::cout << "Playing done" << std::endl;
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

}


void StartClient(uint16_t port) {

    try {
        net::io_context io_context;

        // ѕеред отправкой данных нужно открыть сокет. 
        // ѕри открытии указываем протокол (IPv4 или IPv6) вместо endpoint.
        udp::socket socket(io_context, udp::v4());

        boost::system::error_code ec;


        Recorder recorder(ma_format_u8, 1);

        while (true) {
            
            std::cout << "Press ip..." << std::endl;
            std::string ip;
            std::getline(std::cin, ip);

            std::string str;
            std::cout << "Press Enter to record message..." << std::endl;
            std::getline(std::cin, str);

            auto rec_result = recorder.Record(65000, 1.5s);
            std::cout << "Recording done" << std::endl;

           auto size = rec_result.data.size() * recorder.GetFrameSize();

  
            auto endpoint = udp::endpoint(net::ip::make_address(ip, ec), port);
            socket.send_to(net::buffer(rec_result.data, size),endpoint);
        }

    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

}




int main(int argc, char** argv){
    
    static const int port =std::stoi(std::string(argv[2]));

    std::string com = std::string(argv[1]);

    //проевер€ем колличество аргументов командной строки должен быть сервер\клиент и порт
    if (argc != 3) {
        std::cout << "Usage: "sv << argv[0] << "server\clien, port"sv << std::endl;
        return 1; 
    }

    //запускаем либо сервер либо клиент
    if (com == "client") {
        StartClient(port);
    }

    if (com == "server") {
        StartServer(port);
    }

    return 0;
}
