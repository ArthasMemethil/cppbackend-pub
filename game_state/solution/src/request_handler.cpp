#include "request_handler.h" 



namespace http_handler {

    http::response<http::string_body> RequestHandler::ReportServerError(unsigned int version, bool keep_alive)
    {
        http::response<http::string_body> response(http::status::internal_server_error, version);
        std::string_view content_type = ContentType::JSON_APP;
        std::string body = "Internal server error";
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }

    bool RequestHandler::IsAPIRequest(const std::string& req)
    {
        return req.substr(0, 4) == "/api";
    }

}  // namespace http_handler
