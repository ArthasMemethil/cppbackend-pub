#pragma once

#include "http_server.h"
#include "model.h"
#include "json_support.h"


namespace http_handler {

    namespace beast = boost::beast;
    namespace http = boost::beast::http;
    namespace net = boost::asio;
    using namespace std::literals;
    namespace sys = boost::system;
    namespace json = boost::json;

    // Запрос, тело которого представлено в виде строки
    using StringRequest = http::request<http::string_body>;
    // Ответ, тело которого представлено в виде строки
    using StringResponse = http::response<http::string_body>;


    struct ContentType {
        ContentType() = delete;
        constexpr static std::string_view TEXT_HTML = "text/html"sv;
        constexpr static std::string_view TEXT_JSON = "application/json"sv;
        // При необходимости внутрь ContentType можно добавить и другие типы контента
    };


class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

        // Обработать запрос request и отправить ответ, используя send
        StringRequest req_str(req);
        send(HandleRequest(std::move(req_str)));
    }

private:

    StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
        bool keep_alive,
        std::string_view content_type );
 
    StringResponse HandleRequest(StringRequest&& req);

    model::Game& game_;
};

}  // namespace http_handler
