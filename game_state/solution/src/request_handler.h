#pragma once


#include "http_server.h"
#include "static_support.h"
#include "request_handler_api.h"

namespace http_handler {

    namespace net = boost::asio;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace fs = std::filesystem;
    namespace json = boost::json;
    namespace sys = boost::system;
    using namespace std::literals;
    using Strand = net::strand<net::io_context::executor_type>;

    struct AllowedRequests
    {
        AllowedRequests() = delete;
        constexpr static std::string_view API = "/api/"sv;
    };

    class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
    public:
        explicit RequestHandler(Application& app, fs::path game_data, Strand& api_strand)
            : app_(app), req_static_{ game_data }, api_strand_(api_strand), req_api_{ app_ } {}

        RequestHandler(const RequestHandler&) = delete;
        RequestHandler& operator=(const RequestHandler&) = delete;

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            //Обработать запрос request и отправить ответ, используя send
            std::string request = std::string(req.target());
            auto version = req.version();
            auto keep_alive = req.keep_alive();

            try
            {
                if (IsAPIRequest(request))
                {
                    auto handle = [self = shared_from_this(), send,
                        req = std::forward<decltype(req)>(req), version, keep_alive] {
                        try {
                            assert(self->api_strand_.running_in_this_thread());
                            return send(self->req_api_.operator()(req));
                        }
                        catch (...) {
                            send(self->ReportServerError(version, keep_alive));
                        }
                    };
                    return net::dispatch(api_strand_, handle);
                }

                return std::visit(
                    [&send](auto&& result) {
                        send(std::forward<decltype(result)>(result));
                    },
                    req_static_.operator()(req));
            }
            catch (...) {
                send(ReportServerError(version, keep_alive));
            }
        }

    private:
        static_support::RequestHandlerStatic req_static_;
        Application& app_;
        Strand& api_strand_;
        http_handler_api::RequestHandlerAPI req_api_;

        http::response<http::string_body> ReportServerError(unsigned int version, bool keep_alive);

        bool IsAPIRequest(const std::string& req);
    };

}  // namespace http_handler
