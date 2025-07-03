#pragma once
#include <json_serialize.h>
#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <memory>
#include <shared_mutex>

#ifdef EASYCPP_LOGGING
#include <logger.h>
#else
#define DEBUG(...)  ((void)0)
#define INFO(...) ((void)0)
#define WARNING(...) ((void)0)
#define ERROR(...) ((void)0)
#endif

namespace queue
{
class RabbitMqEventHandler : public AMQP::LibEvHandler
{
public:
    RabbitMqEventHandler(struct ev_loop *loop)
    :
    AMQP::LibEvHandler(loop)
    {
    }

    virtual ~RabbitMqEventHandler() = default;
private:
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        ERROR("error: %s", message);
    }

    virtual void onConnected(AMQP::TcpConnection *connection) override 
    {
        INFO("connected");
    }

    virtual void onReady(AMQP::TcpConnection *connection) override 
    {
        INFO("ready");
    }

    virtual void onClosed(AMQP::TcpConnection *connection) override 
    {
        ERROR("closed");
    }

    virtual void onDetached(AMQP::TcpConnection *connection) override 
    {
        ERROR("detached");                
    }
};

class RabbitMq
{
public:
    bool Start(const std::string &conn_str)
    {
        INFO("连接启动开始: %s", this->_connection_str.data());
        if (this->_started.exchange(true))
        {
            //已经启动过了
            INFO("连接已经启动: %s", this->_connection_str.data());
            return true;
        }
        this->_connection_str = conn_str;
        this->_handler = std::make_shared<RabbitMqEventHandler>(EV_DEFAULT);
        this->_connection = std::make_shared<AMQP::TcpConnection>(this->_handler.get(), AMQP::Address(this->_connection_str));
        this->_thread = std::thread([]()
        { 
            ev_run(EV_DEFAULT, 0); 
        });
        INFO("连接启动结束: %s", this->_connection_str.data());
        return true;
    }

    bool Stop()
    {
        try
        {
            INFO("连接停止开始: %s", this->_connection_str.data());
            this->_started.store(false);
            this->_connection->close();
            if (this->_thread.joinable())
            {
                this->_thread.join();
            }
            INFO("连接停止结束: %s", this->_connection_str.data());
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR(ex.what());
        }
        return false;
    }
    std::shared_ptr<AMQP::TcpConnection> Connection() const
    {
        return this->_connection;
    }
    static RabbitMq* Instance()
    {
        static RabbitMq rabbitmq;
        return &rabbitmq;
    }
private:
    std::shared_ptr<RabbitMqEventHandler>   _handler;
    std::shared_ptr<AMQP::TcpConnection>    _connection;
    std::thread                             _thread;
    std::atomic<bool>                       _started;
    std::string                             _connection_str;
};

template<class T>
class RabbitQueue
{
public:
    using OnCall = std::function<bool(const std::shared_ptr<T> &data)>;
    RabbitQueue() = default;
    ~RabbitQueue() = default;
    RabbitQueue(const std::string &name, int qos = 100)
    :
    _name(name),
    _qos(qos)
    {
    }

    void Consume(const OnCall &on_call)
    {
        INFO("[%s]启动消费开始", this->_name.data());
        auto channel = this->get_read_channel();
        if (!channel) return;

        channel->consume("").
        onReceived([this, &on_call](
            const AMQP::Message &message,
            uint64_t delivery_tag,
            bool redelivered)
        {
            try
            {
                auto body = std::string(message.body(), message.bodySize());
                INFO("[%s]消费数据: [%llu]%s", message.routingkey().data(), delivery_tag, body.data());
                auto data = serialize::JsonSerializer<T>::FromStringPtr(body);
                if (on_call(data))
                {
                    auto channel = this->get_read_channel();
                    channel->ack(delivery_tag);
                    INFO("[%s]消费数据成功: [%llu]", message.routingkey().data(), delivery_tag);
                }
            }
            catch(std::exception &ex)
            {
                ERROR("[%s] %s", this->_name.data(), ex.what());
            }
        }).
        onSuccess([this]()
        {
            INFO("[%s] Consume onSuccess", this->_name.data());
        }).
        onError([this](const char* message)
        {
            ERROR("[%s] Consume onError: %s", this->_name.data(), message);
        });
        INFO("[%s]启动消费结束", this->_name.data());
    }

    void Publish(const std::shared_ptr<T> &data, uint8_t priority = 0)
    {
        try
        {
            auto channel = this->get_write_channel();
            if (!channel) return;

            AMQP::Reliable reliable(*channel.get());
            auto message = serialize::JsonSerializer<T>::ToString(data);
            reliable.publish("", this->_name, message).
            onAck([this]()
            {
                INFO("[%s] onAck", this->_name.data());
            }).
            onNack([this]()
            {
                INFO("[%s] onNack", this->_name.data());
            }).
            onLost([this]()
            {
                INFO("[%s] onLost", this->_name.data());
            }).
            onError([this](const char* message)
            {
                INFO("[%] onError", this->_name.data());
            });
            INFO("[%s]生产数据: %s", this->_name.data(), message.data());
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s", this->_name.data(), ex.what());
        }
    }

private:
    std::shared_ptr<AMQP::TcpChannel> get_write_channel()
    {
        try
        {
            std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
            if (!this->_write_channel)
            {
                auto connection = RabbitMq::Instance()->Connection();
                if (!connection) return nullptr;
                this->_write_channel = std::make_shared<AMQP::TcpChannel>(connection.get());
                this->_write_channel->declareQueue(this->_name, AMQP::durable).
                onSuccess([this](const std::string &name, int msgcount, int consumercount)
                {
                    INFO("[%s]队列生产声明成功, msgcount: %d, consumercount: %d", this->_name.data(), msgcount, consumercount);
                });
            }
            return this->_write_channel;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s", this->_name.data(), ex.what());
        }
        return nullptr;
    }

    std::shared_ptr<AMQP::TcpChannel> get_read_channel()
    {
        try
        {
            std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
            if (!this->_read_channel)
            {
                auto connection = RabbitMq::Instance()->Connection();
                if (!connection) return nullptr;
                this->_read_channel = std::make_shared<AMQP::TcpChannel>(connection.get());
                this->_read_channel->setQos(this->_qos);
                this->_read_channel->declareQueue(this->_name, AMQP::durable).
                onSuccess([this](const std::string &name, int msgcount, int consumercount)
                {
                    INFO("[%s]队列消费声明成功, msgcount: %d, consumercount: %d", this->_name.data(), msgcount, consumercount);
                });
            }
            return this->_read_channel;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s", this->_name.data(), ex.what());
        }
        return nullptr;
    }

private:
    int _qos;
    std::string _name;
    std::shared_mutex _mutex;
    std::shared_ptr<AMQP::TcpChannel> _read_channel;
    std::shared_ptr<AMQP::TcpChannel> _write_channel;
};
}
