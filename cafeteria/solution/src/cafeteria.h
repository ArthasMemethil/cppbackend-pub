#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <memory>


#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;

using namespace std::literals;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

using Timer = net::steady_timer;

class Order : public std::enable_shared_from_this<Order> {
public:
    Order(net::io_context& io, int id, std::shared_ptr<Sausage> sausage, std::shared_ptr<Bread> bread, std::shared_ptr<GasCooker> gas_cooker, HotDogHandler handler)
        : io_{ io }
        , id_{ id }
        , sausage_{ sausage }
        , bread_{ bread }
        , gas_cooker_{ gas_cooker }
        , handler_{ std::move(handler) } {
    }

    void MakeHotDog()
    {
        sausage_->StartFry(*gas_cooker_, [self = shared_from_this()]
            {
                self->sausage_timer_.expires_after(1500ms);

                self->sausage_timer_.async_wait([self](sys::error_code ec)
                    {
                        self->sausage_->StopFry();
                        self->Complete();
                    });
            });

        bread_->StartBake(*gas_cooker_, [self = shared_from_this()]
            {
                self->bread_timer_.expires_after(1000ms);

                self->bread_timer_.async_wait([self](sys::error_code ec)
                    {
                        self->bread_->StopBake();
                        self->Complete();
                    });
            });
    }

private:

    void Complete()
    {
        if (sausage_->IsCooked() && bread_->IsCooked())
        {
            HotDog hot_dog(id_, sausage_, bread_);
            handler_(hot_dog);
        }
    }

    net::io_context& io_;
    int id_;
    std::shared_ptr<Sausage> sausage_;
    std::shared_ptr<Bread> bread_;
    std::shared_ptr<GasCooker> gas_cooker_;
    HotDogHandler handler_;

    net::strand<net::io_context::executor_type> strand_{ net::make_strand(io_) };
    net::steady_timer sausage_timer_{ strand_};
    net::steady_timer bread_timer_{ strand_};
};



// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
public:
    explicit Cafeteria(net::io_context& io)
        : io_{io} {
    }

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) {
        // TODO: Реализуйте метод самостоятельно
        // При необходимости реализуйте дополнительные классы
        auto sausage = store_.GetSausage();
        auto bread = store_.GetBread();

        net::post(strand_, [this, handler]() {
            auto order = std::make_shared<Order>(io_, ++order_id_, store_.GetSausage(), store_.GetBread(), gas_cooker_, std::move(handler));
            order->MakeHotDog();
            });

    }

private:
    net::io_context& io_;
    // Используется для создания ингредиентов хот-дога
    Store store_;
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
    // Используйте её для приготовления ингредиентов хот-дога.
    // Плита создаётся с помощью make_shared, так как GasCooker унаследован от
    // enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
    net::strand<net::io_context::executor_type> strand_{ net::make_strand(io_) };
    int order_id_ = 0;
    
};
