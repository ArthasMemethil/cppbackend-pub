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

    StringResponse MakeStringResponseGet(http::status status, std::string_view body, unsigned http_version,
        bool keep_alive,
        std::string_view content_type = ContentType::TEXT_JSON)
    {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }

    StringResponse MakeStringResponseOther(http::status status, std::string_view body, unsigned http_version,
        bool keep_alive,
        std::string_view content_type = ContentType::TEXT_JSON)
    {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }

    StringResponse HandleRequest(StringRequest&& req) {
        const auto text_response_get = [&req, this](http::status status, std::string_view text) {
            return MakeStringResponseGet(status, text, req.version(), req.keep_alive());
        };

        const auto text_response_get_full = [&req, this](http::status status, std::string_view text) {
            return MakeStringResponseGet(status, text, req.version(), req.keep_alive());
        };

        const auto text_response_other = [&req, this](http::status status, std::string_view text) {
            return MakeStringResponseOther(status, text, req.version(), req.keep_alive());
        };

        // Здесь можно обработать запрос и сформировать ответ
        std::string req_str = "/api/v1/maps";
        std::string api_str = "/api/";
        std::string req_str_map;

        std::string req_str_t(req.target());
        if (req_str_t > req_str) {
            req_str_map = req_str_t.substr(12);
        }

        if ((req.method() == http::verb::get) && (req_str == req.target())) {
            //сделать джс карт
            auto js_maps = json_support::GetJSONAllMaps(game_.GetMaps());

            return text_response_get(http::status::ok, json_support::GetFormattedJSONStr(js_maps));
        }
        else if (req.method() == http::verb::get && ((req_str + req_str_map) == req.target())) {
            //сделать джс карты плюс описание
            std::string map_to_find = req_str_map.substr(1);

            model::Map::Id id(map_to_find);
            auto map_ptr = game_.FindMap(id);
            if (map_ptr != nullptr) {
                auto js_map = json_support::GetJSONRequiredMap(*map_ptr);
                return text_response_get_full(http::status::ok, json_support::GetFormattedJSONStr(js_map));
            }
            else { 
                auto js_empty = json_support::GetJSONNotFound();
                return text_response_other(http::status::not_found, json_support::GetFormattedJSONStr(js_empty));
            }
        } else {
            auto js_bad_req = json_support::GetJSONBadRequest();
            return text_response_other(http::status::bad_request, json_support::GetFormattedJSONStr(js_bad_req));
        }
    }

    model::Game& game_;
};

}  // namespace http_handler
