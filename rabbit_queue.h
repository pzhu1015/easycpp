#pragma once
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include <memory>
#include <shared_mutex>
#include <functional>

#ifdef EASYCPP_LOGGING
#include "logger.h"
#else
#define DEBUG(...)  ((void)0)
#define INFO(...) ((void)0)
#define WARNING(...) ((void)0)
#define ERROR(...) ((void)0)
#endif

namespace queue
{
using TcpChannelPtr = std::shared_ptr<AMQP::TcpChannel>;
using TcpConnectionPtr = std::shared_ptr<AMQP::TcpConnection>;
template <typename BASE=AMQP::Tagger>
using ReliablePtr = std::shared_ptr<AMQP::Reliable<BASE>>;

class RabbitMqEventHandler : public AMQP::LibEvHandler
{
public:
    using OnDetach = std::function<void(AMQP::TcpConnection*)>;
    using OnReady = std::function<void(AMQP::TcpConnection*)>;
    RabbitMqEventHandler(
        struct ev_loop *loop, 
        const OnDetach &on_deatch,
        const OnReady &on_ready)
    :
    AMQP::LibEvHandler(loop),
    _on_detach(on_deatch),
    _on_ready(on_ready)
    {
    }

    virtual ~RabbitMqEventHandler() = default;

private:
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        ERROR(message);
    }

    virtual void onConnected(AMQP::TcpConnection *connection) override 
    {
        INFO("connected");
    }

    virtual void onReady(AMQP::TcpConnection *connection) override 
    {
        INFO("ready");
        if (this->_on_ready)
        {
            this->_on_ready(connection);
        }
    }

    virtual void onClosed(AMQP::TcpConnection *connection) override 
    {
        ERROR("closed");
    }

    virtual void onDetached(AMQP::TcpConnection *connection) override 
    {
        ERROR("detached");                
        if (this->_on_detach)
        {
            this->_on_detach(connection);
        }
    }

public:
    OnDetach    _on_detach;
    OnReady     _on_ready;
};
using RabbitMqEventHandlerPtr = std::shared_ptr<RabbitMqEventHandler>;
using OnConsume = std::function<bool(const std::string &data)>;

class RabbitChannel
{
public:

    RabbitChannel(const std::string &name, int qos = 100)
    :
    _name(name),
    _qos(qos)
    {
        this->_started.store(false);
        this->_restarting.store(false);
        this->_restart_count.store(0);
        this->_reconnect_seconds.store(0);
    }

    bool Consume(const OnConsume &on_consume)
    {
        try
        {
            std::unique_lock<std::mutex> lock(this->_mutex);
            this->_on_consume = on_consume;
            if (!this->_channel)
            {
                ERROR("[%s] consume channel not connected", this->_name.data());
                return false;
            }


            this->_channel->consume("").
            onReceived([this](
                const AMQP::Message &message,
                uint64_t delivery_tag,
                bool redelivered)
            {
                try
                {
                    std::unique_lock<std::mutex> lock(this->_mutex);
                    auto data = std::string(message.body(), message.bodySize());
                    //INFO("[%s] consume onReceived: [%llu]%s", this->_name.data(), delivery_tag, data.data());
                    if (this->_on_consume && this->_on_consume(data))
                    {
                        if (this->_channel)
                        {
                            this->_channel->ack(delivery_tag);
                            INFO("[%s] consume onReceived to ack: [%llu]%s", this->_name.data(), delivery_tag, data.data());
                        }
                    }
                }
                catch(std::exception &ex)
                {
                    ERROR("[%s] consume: %s", this->_name.data(), ex.what());
                }
            }).
            onSuccess([this]()
            {
                INFO("[%s] consume onSuccess", this->_name.data());
            }).
            onError([this](const char* message)
            {
                ERROR("[%s] consume onError: %s", this->_name.data(), message);
            });
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] consume exception: %s", this->_name.data(), ex.what());
        }
        return false;
    }

    bool Publish(const std::string &data, uint8_t priority)
    {
        try
        {
            std::unique_lock<std::mutex> lock(this->_mutex);
            if (!this->_channel)
            {
                ERROR("[%s] publish channel not connected", this->_name.data());
                return false;
            }
            if (!this->_reliable)
            {
                this->_reliable = std::make_shared<AMQP::Reliable<>>(*(this->_channel.get()));
            }

            //INFO("[%s] publish: %s", this->_name.data(), data.data());
            AMQP::Envelope envelope(data);
            envelope.setPriority(priority);
            envelope.setDeliveryMode(2);
            this->_reliable->publish("", this->_name, envelope).
            onAck([this, data]()
            {
                INFO("[%s] publish onAck: %s", this->_name.data(), data.data());
            }).
            onNack([this]()
            {
                INFO("[%s] publish onNack", this->_name.data());
            }).
            onLost([this]()
            {
                ERROR("[%s] publish onLost", this->_name.data());
            }).
            onError([this](const char* message)
            {
                ERROR("[%s] publish onError: %s", this->_name.data(), message);
            });
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] publish exception: %s", this->_name.data(), ex.what());
        }
        return false;
    }

    bool Start(const std::string connection_str)
    {
        try
        {
            std::unique_lock<std::mutex> lock(this->_mutex);
            if (this->_started.exchange(true))
            {
                ERROR("[%s] already started", this->_name.data());
                return true;
            }
            INFO("[%s] channel start", this->_name.data());
            using namespace std::placeholders;
            this->_connection_str = connection_str;
            this->_loop = ev_loop_new(0);
            this->_handler = std::make_shared<RabbitMqEventHandler>(
                 this->_loop, 
                 std::bind(&RabbitChannel::OnDetach, this, _1),
                 std::bind(&RabbitChannel::OnReady, this, _1));
            this->_connection = std::make_shared<AMQP::TcpConnection>(this->_handler.get(), AMQP::Address(this->_connection_str));
            this->_channel = std::make_shared<AMQP::TcpChannel>(this->_connection.get());
            this->_channel->setQos(this->_qos);
            this->_channel->onReady([this]()
            {
                INFO("[%s] Channel onReady", this->_name.data());
            });
            this->_channel->onError([this](const char* message)
            {
                ERROR("[%s] Channel onError: %s", this->_name.data(), message);
            });
            this->_channel->declareQueue(this->_name, AMQP::durable).
            onSuccess([this](const std::string &name, int msgs, int consumers)
            {
                INFO("[%s] DeclareQueue onSuccess, [msgs: %d][consumers: %d]", this->_name.data(), msgs, consumers);
            }).
            onError([this](const char* message)
            {
                ERROR("[%s] DeclareQueue onError: %s", this->_name.data(), message);
            });
            INFO("[%s] channel start event loop task", this->_name.data());
            this->_loop_task = std::async(std::launch::async, [this]()
            { 
                INFO("[%s] event start...", this->_name.data());
                ev_run(this->_loop, 0); 
                INFO("[%s] event exit...", this->_name.data());
            });
            INFO("[%s] channel started", this->_name.data());
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s", this->_name.data(), ex.what());
        }
        return false;
    }

    bool Stop()
    {
        try
        {
            std::unique_lock<std::mutex> lock(this->_mutex);
            INFO("[%s] channel stop", this->_name.data());
            this->_started.store(false);
            if (this->_channel && this->_channel->connected())
            {
                this->_channel->close();
                INFO("[%s] close channel", this->_name.data());
            }
            if (this->_connection && !this->_connection->closed())
            {
                this->_connection->close();
                INFO("[%s] close connection", this->_name.data());
            }
            this->_closed_handlers.emplace_back(this->_handler);
            this->_reliable = nullptr;
            this->_channel = nullptr;
            this->_connection = nullptr;
            this->_handler = nullptr;
            ev_break(this->_loop, EVBREAK_ALL);
            this->_loop_task.wait();
            ev_loop_destroy(this->_loop);
            INFO("[%s] channel stopped", this->_name.data());
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s", this->_name.data(), ex.what());
        }
        return false;
    }

    void OnReady(AMQP::TcpConnection* connection)
    {
        INFO("[%s] connection ready", this->_name.data());
        if (this->_connection.get() != connection)
        {
            ERROR("[%s] connection ready, connection not match", this->_name.data());
            return;
        }
        if (this->_restart_count.load() > 0 && this->_on_consume)
        {
            if (this->_on_ready_task.valid())
            {
                this->_on_ready_task.wait();
            }
            this->_on_ready_task = std::async(std::launch::async, [this]()
            {
                INFO("[%s] connection ready, restore consume", this->_name.data());
                this->Consume(this->_on_consume);
            });
        }
    }

    void OnDetach(AMQP::TcpConnection* connection)
    {
        INFO("[%s] connection detach", this->_name.data());
        if (this->_connection.get() != connection)
        {
            ERROR("[%s] connection detach, connection not match", this->_name.data());
            return;
        }
        if (this->_restarting.exchange(true)) 
        {
            ERROR("[%s] already restaring", this->_name.data());
            if (this->_on_detach_task.valid())
            {
                this->_on_detach_task.wait();
            }
        }
        this->_on_detach_task = std::async(std::launch::async, [this]()
        {
            this->restart();
            this->_restarting.store(false);
        });
    }

private:
    bool restart()
    {
        try
        {
            if (this->_reconnect_seconds.load() >= 5)
            {
                this->_reconnect_seconds.store(0);
            }
            this->_reconnect_seconds++;
            this->_restart_count++;
            INFO("[%s] channel wait [%d(s)] restart...", this->_name.data(), this->_reconnect_seconds.load());
            std::this_thread::sleep_for(std::chrono::seconds(this->_reconnect_seconds.load()));
            if (!this->Stop()) 
            {
                INFO("[%s] channel stop fail: [closed: %d][usable: %d][ready: %d]",
                    this->_name.data(), this->_connection->closed(), this->_connection->usable(), this->_connection->ready());
                return false;
            }
            INFO("[%s] channel stop success", this->_name.data());

            if (!this->Start(this->_connection_str))
            {
                INFO("[%s] channel start fail : [closed: %d][usable: %d][ready: %d]",
                    this->_name.data(), this->_connection->closed(), this->_connection->usable(), this->_connection->ready());
                return false;
            }
            INFO("[%s] channel start success: [closed: %d][usable: %d][ready: %d]", 
                 this->_name.data(), this->_connection->closed(), this->_connection->usable(), this->_connection->ready());
            INFO("[%s] channel restarted, try [%lld] times...", this->_name.data(), this->_restart_count.load());
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] reconnect exception: %s", this->_name.data(), ex.what());
        }
        return false;
    }
private:
    int                     _qos;
    std::string             _name;
    std::string             _connection_str;
    std::mutex              _mutex;
    std::future<void>       _on_detach_task;
    std::future<void>       _on_ready_task;
    std::future<void>       _loop_task;
    std::atomic<int>        _reconnect_seconds;
    std::atomic<bool>       _restarting;
    std::atomic<bool>       _started;
    std::atomic<int64_t>    _restart_count;
    struct ev_loop*         _loop;
    OnConsume               _on_consume;
    TcpChannelPtr           _channel;
    ReliablePtr<>           _reliable;
    TcpConnectionPtr        _connection;
    RabbitMqEventHandlerPtr _handler;
    std::vector<TcpConnectionPtr> _closed_connections;
    std::vector<RabbitMqEventHandlerPtr> _closed_handlers;
};
using RabbitChannelPtr = std::shared_ptr<RabbitChannel>;

class RabbitMq
{
public:
    bool Start(const std::string &conn_str)
    {
        this->_connection_str = conn_str;
        INFO("started: %s", this->_connection_str.data());
        return true;
    }

    RabbitChannelPtr OpenChannel(const std::string &name, int qos)
    {
        auto channel = std::make_shared<RabbitChannel>(name, qos);
        channel->Start(this->_connection_str);
        return channel;
    }

    static RabbitMq* Instance()
    {
        static RabbitMq rabbitmq;
        return &rabbitmq;
    }
private:
    std::string             _connection_str;
};

class RabbitQueue
{
public:
    RabbitQueue(const std::string &name, int qos = 100)
    :
    _name(name),
    _qos(qos)
    {
    }

    bool Consume(const OnConsume &on_consume)
    {
        auto channel = this->get_channel();
        if (!channel) return false;

        return channel->Consume(on_consume);
    }

    bool Publish(const std::string &data, uint8_t priority = 0)
    {
        try
        {
            auto channel = this->get_channel();
            if (!channel) return false;

            return channel->Publish(data, priority);
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s, 发送异常数据: %s", this->_name.data(), ex.what(), data.data());
        }
        return false;
    }

private:
    RabbitChannelPtr get_channel()
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        if (!this->_channel)
        {
            this->_channel = RabbitMq::Instance()->OpenChannel(this->_name, this->_qos);
        }
        return this->_channel;
    }

    int                 _qos;
    std::string         _name;
    std::mutex          _mutex;
    RabbitChannelPtr    _channel;
};
}
