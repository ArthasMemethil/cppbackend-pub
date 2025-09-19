#include "request_handler.h"


namespace http_handler {

    StringResponse RequestHandler::MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
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

    StringResponse RequestHandler::HandleRequest(StringRequest&& req) {
        const auto text_response = [&req, this](http::status status, std::string_view text) {
            return MakeStringResponse(status, text, req.version(), req.keep_alive());
        };

        // Здесь можно обработать запрос и сформировать ответ
        std::string req_str = "/api/v1/maps";
        std::string req_str_map;

        std::string req_str_t(req.target());
        if (req_str_t > req_str) {
            req_str_map = req_str_t.substr(12);
        }

        if ((req.method() == http::verb::get) && (req_str == req.target())) {
            //сделать джс карт
            auto js_maps = json_support::GetJSONAllMaps(game_.GetMaps());

            return text_response(http::status::ok, json_support::GetFormattedJSONStr(js_maps));
        }
        else if (req.method() == http::verb::get && ((req_str + req_str_map) == req.target())) {
            //сделать джс карты плюс описание
            std::string map_to_find = req_str_map.substr(1);

            model::Map::Id id(map_to_find);
            auto map_ptr = game_.FindMap(id);
            if (map_ptr != nullptr) {
                auto js_map = json_support::GetJSONRequiredMap(*map_ptr);
                return text_response(http::status::ok, json_support::GetFormattedJSONStr(js_map));
            }
            else {
                auto js_empty = json_support::GetJSONNotFound();
                return text_response(http::status::not_found, json_support::GetFormattedJSONStr(js_empty));
            }
        }
        else {
            auto js_bad_req = json_support::GetJSONBadRequest();
            return text_response(http::status::bad_request, json_support::GetFormattedJSONStr(js_bad_req));
        }
    }


}  // namespace http_handler
