// Подключаем заголовочный файл <sdkddkver.h> в системе Windows,
// чтобы избежать предупреждения о неизвестной версии Platform SDK,
// когда используем заголовочные файлы библиотеки Boost.Asio

// Boost.Beast будет использовать std::string_view вместо boost::string_view

#define BOOST_BEAST_USE_STD_STRING_VIEW

#ifdef WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>
#include <optional>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std::literals;
namespace beast = boost::beast;
namespace http = beast::http;
using namespace std::literals;

namespace beast = boost::beast;
namespace http = beast::http;
// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;


// Структура ContentType задаёт область видимости для констант,
// задающий значения HTTP-заголовка Content-Type
struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponseGet(http::status status, std::string_view body, unsigned http_version,
    bool keep_alive,
    std::string_view content_type = ContentType::TEXT_HTML)
 {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse MakeStringResponseHead(http::status status, std::string_view body, unsigned http_version,
    bool keep_alive,
    std::string_view content_type = ContentType::TEXT_HTML)
{
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = "";
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse MakeStringResponseOther(http::status status, std::string_view body, unsigned http_version,
    bool keep_alive,
    std::string_view content_type = ContentType::TEXT_HTML)
{
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.set(http::field::allow, "GET, HEAD");
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}



StringResponse HandleRequest(StringRequest&& req) {
    const auto text_response_get = [&req](http::status status, std::string_view text) {
        return MakeStringResponseGet(status, text, req.version(), req.keep_alive());
    };

    const auto text_response_head = [&req](http::status status, std::string_view text) {
        return MakeStringResponseHead(status, text, req.version(), req.keep_alive());
    };

    const auto text_response_other = [&req](http::status status, std::string_view text) {
        return MakeStringResponseOther(status, text, req.version(), req.keep_alive());
    };

    // Здесь можно обработать запрос и сформировать ответ
    std::string text = "Hello,";
    std::string text_second(req.target());
    text_second[0] = ' ';


    if (req.method() == http::verb::get){
        return text_response_get(http::status::ok, text + text_second);

    }
    else if (req.method() == http::verb::head) {
        return text_response_head(http::status::ok, text + text_second);
    }
    else {
        return text_response_other(http::status::method_not_allowed, "Invalid method"sv);
    }
} 



std::optional<StringRequest> ReadRequest(tcp::socket& socket, beast::flat_buffer& buffer) {
    beast::error_code ec;
    StringRequest req;
    // Считываем из socket запрос req, используя buffer для хранения данных.
    // В ec функция запишет код ошибки.
    http::read(socket, buffer, req, ec);

    if (ec == http::error::end_of_stream) {
        return std::nullopt;
    }
    if (ec) {
        throw std::runtime_error("Failed to read request: "s.append(ec.message()));
    }
    return req;
}

void DumpRequest(const StringRequest& req) {
    std::cout << req.method_string() << ' ' << req.target() << std::endl;
    // Выводим заголовки запроса
    for (const auto& header : req) {
        std::cout << "  "sv << header.name_string() << ": "sv << header.value() << std::endl;
    }
}

template <typename RequestHandler>
void HandleConnection(tcp::socket& socket, RequestHandler&& handle_request) {
    try {
        // Буфер для чтения данных в рамках текущей сессии.
        beast::flat_buffer buffer;

        // Продолжаем обработку запросов, пока клиент их отправляет
        while (auto request = ReadRequest(socket, buffer)) {
            // Обрабатываем запрос и формируем ответ сервера
            DumpRequest(*request);
            
            StringResponse response = handle_request(*std::move(request));
            // Отправляем ответ сервера клиенту
            http::write(socket, response);

            // Прекращаем обработку запросов, если семантика ответа требует это
            if (response.need_eof()) {
                break;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    beast::error_code ec;
    // Запрещаем дальнейшую отправку данных через сокет
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

int main() {
    net::io_context ioc;

    const auto address = net::ip::make_address("0.0.0.0");
    constexpr unsigned short port = 8080;

    tcp::acceptor acceptor(ioc, { address, port });


    std::cout << "Server has started..." << std::endl;
    while (true) {
        tcp::socket socket(ioc);
        acceptor.accept(socket);
        std::thread t([](tcp::socket socket){HandleConnection(socket, HandleRequest);},std::move(socket));
        t.detach();
    }
}
   
