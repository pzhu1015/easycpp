#pragma once
#include <json_serialize.h>
#include <ev.h>
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
#include <logger.h>
#else
#define DEBUG(...)  ((void)0)
#define INFO(...) ((void)0)
#define WARNING(...) ((void)0)
#define ERROR(...) ((void)0)
#endif

using OnDetach = std::function<void()>;
using OnConsume = std::function<bool(const std::string &data)>;
using TcpChannelPtr = std::shared_ptr<AMQP::TcpChannel>;
using TcpConnectionPtr = std::shared_ptr<AMQP::TcpConnection>;

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
        if (this->Detach)
        {
            this->_task = std::async(std::launch::async, [this]()
            {
                this->Detach();
            });
        }
    }
public:
    std::future<void> _task;
    OnDetach Detach;
};
using RabbitMqEventHandlerPtr = std::shared_ptr<RabbitMqEventHandler>;

enum class ChannelType
{
    Read = 0,
    Write = 1
};

class RabbitChannel
{
public:
    RabbitChannel(const std::string &name)
    :
    _name(name),
    _type(ChannelType::Write)
    {
    }

    RabbitChannel(const std::string &name, const OnConsume &on_consume, int qos)
    :
    _qos(qos),
    _name(name),
    _on_consume(on_consume),
    _type(ChannelType::Read)
    {
    }

    ChannelType GetType()
    {
        return this->_type;
    }

    std::string Name()
    {
        return this->_name;
    }

    void Consume()
    {
        if (!this->_channel) return;
        this->_channel->consume("").
        onReceived([this](
            const AMQP::Message &message,
            uint64_t delivery_tag,
            bool redelivered)
        {
            try
            {
                auto data = std::string(message.body(), message.bodySize());
                INFO("[%s]消费数据: [%llu]%s", message.routingkey().data(), delivery_tag, data.data());
                if (this->_on_consume && this->_on_consume(data))
                {
                    if (this->_channel)
                    {
                        this->_channel->ack(delivery_tag);
                        INFO("[%s]消费数据成功: [%llu]", message.routingkey().data(), delivery_tag);
                    }
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
    }

    void Publish(const std::string &data, uint8_t priority)
    {
        AMQP::Reliable reliable(*(this->_channel.get()));
        reliable.publish("", this->_name, data).
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
    }

    void Start(const TcpConnectionPtr &connection)
    {
        if (this->_type == ChannelType::Read)
        {
            if (!connection) return;
            this->_channel = std::make_shared<AMQP::TcpChannel>(connection.get());
            this->_channel->setQos(this->_qos);
            this->_channel->declareQueue(this->_name, AMQP::durable).
            onSuccess([this](const std::string &name, int msgcount, int consumercount)
            {
                INFO("[%s]队列消费声明成功, msgcount: %d, consumercount: %d", this->_name.data(), msgcount, consumercount);
            });
        }
        else
        {
            if (!connection) return;
            this->_channel = std::make_shared<AMQP::TcpChannel>(connection.get());
            this->_channel->declareQueue(this->_name, AMQP::durable).
            onSuccess([this](const std::string &name, int msgcount, int consumercount)
            {
                INFO("[%s]队列生产声明成功, msgcount: %d, consumercount: %d", this->_name.data(), msgcount, consumercount);
            });
        }
    }
private:
    int             _qos;
    std::string     _name;
    OnConsume       _on_consume;
    ChannelType     _type;
    TcpChannelPtr   _channel;
};
using RabbitChannelPtr = std::shared_ptr<RabbitChannel>;

class RabbitMq
{
public:
    bool Start(const std::string &conn_str)
    {
        try
        {
            if (this->_started.exchange(true))
            {
                INFO("连接已经启动: %s", this->_connection_str.data());
                return true;
            }
            INFO("连接启动开始: %s", this->_connection_str.data());
            this->_connection_str = conn_str;
            if (!this->_handler)
            {
                this->_handler = std::make_shared<RabbitMqEventHandler>(EV_DEFAULT);
                this->_handler->Detach = std::bind(&RabbitMq::ReStart, this);
            }
            this->_connection = std::make_shared<AMQP::TcpConnection>(this->_handler.get(), AMQP::Address(this->_connection_str));
            INFO("启动事件线程: %s", this->_connection_str.data());
            this->_task = std::async(std::launch::async, [this]()
            { 
                INFO("事件监听开始");
                while (this->_started) 
                {
                    ev_run(EV_DEFAULT, EVRUN_ONCE); 
                }
                INFO("事件监听结束");
            });
            INFO("连接启动结束: %s", this->_connection_str.data());
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR(ex.what());
        }
        return false;
    }

    bool Stop()
    {
        try
        {
            INFO("连接停止开始: %s", this->_connection_str.data());
            this->_started.store(false);
            this->_connection->close();
            this->_task.wait();
            INFO("连接停止结束: %s", this->_connection_str.data());
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR(ex.what());
        }
        return false;
    }

    void ReStart()
    {
        try
        {
            if (this->_restarting.exchange(true)) return;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (!this->Stop()) return;
            if (!this->Start(this->_connection_str)) return;
            {
                std::shared_lock<std::shared_mutex> read_lock(this->_read_channel_mutex);
                for (auto itr : this->_read_channels)
                {
                    itr.second->Start(this->_connection);
                    itr.second->Consume();
                }
            }
            {
                std::shared_lock<std::shared_mutex> read_lock(this->_write_channel_mutex);
                for (auto itr : this->_write_channels)
                {
                    itr.second->Start(this->_connection);
                }
            }
            this->_restarting.store(false);
        }
        catch(std::exception &ex)
        {
            ERROR(ex.what());
        }
    }

    TcpConnectionPtr Connection()
    {
        return this->_connection;
    }

    RabbitChannelPtr CreateReadChannel(const std::string &name, const OnConsume &on_consume, int qos)
    {
        if (!this->_connection) return nullptr;
        auto channel = std::make_shared<RabbitChannel>(name, on_consume, qos);
        channel->Start(this->_connection);
        return channel;
    }

    RabbitChannelPtr CreateWriteChannel(const std::string &name)
    {
        if (!this->_connection) return nullptr;
        auto channel = std::make_shared<RabbitChannel>(name);
        channel->Start(this->_connection);
        return channel;
    }

    void AddChannel(const RabbitChannelPtr &channel)
    {
        if (channel->GetType() == ChannelType::Read)
        {
            std::unique_lock<std::shared_mutex> write_lock(this->_read_channel_mutex);
            this->_read_channels.insert({channel->Name(), channel});
        }
        else
        {
            std::unique_lock<std::shared_mutex> write_lock(this->_write_channel_mutex);
            this->_write_channels.insert({channel->Name(), channel});
        }
    }

    RabbitChannelPtr GetReadChannel(const std::string &name)
    {
        std::shared_lock<std::shared_mutex> read_lock(this->_read_channel_mutex);
        auto itr = this->_read_channels.find(name);
        if (itr == this->_read_channels.end())
        {
            return nullptr;
        }
        return itr->second;
    }

    RabbitChannelPtr GetWriteChannel(const std::string &name)
    {
        std::shared_lock<std::shared_mutex> read_lock(this->_write_channel_mutex);
        auto itr = this->_write_channels.find(name);
        if (itr == this->_write_channels.end())
        {
            return nullptr;
        }
        return itr->second;
    }

    static RabbitMq* Instance()
    {
        static RabbitMq rabbitmq;
        return &rabbitmq;
    }
private:
    std::future<void>                       _task;
    std::atomic<bool>                       _started;
    std::atomic<bool>                       _restarting;
    std::string                             _connection_str;
    std::shared_mutex                       _read_channel_mutex;
    std::shared_mutex                       _write_channel_mutex;
    RabbitMqEventHandlerPtr                 _handler;
    TcpConnectionPtr                        _connection;
    std::map<std::string, RabbitChannelPtr> _read_channels;
    std::map<std::string, RabbitChannelPtr> _write_channels;
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

    void Consume(const OnConsume &on_consume)
    {
        INFO("[%s]启动消费开始", this->_name.data());
        this->_on_consume = on_consume;
        auto channel = this->get_read_channel();
        if (!channel) return;
        channel->Consume();
        INFO("[%s]启动消费结束", this->_name.data());
    }

    void Publish(const std::string &data, uint8_t priority = 0)
    {
        try
        {
            auto channel = this->get_write_channel();
            if (!channel) return;

            channel->Publish(data, priority);
            INFO("[%s]生产数据: %s", this->_name.data(), data.data());
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s, 发送异常数据: %s", this->_name.data(), ex.what(), data.data());
        }
    }

private:
    RabbitChannelPtr get_write_channel()
    {
        try
        {
            std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
            auto channel = RabbitMq::Instance()->GetWriteChannel(this->_name);
            if (!channel)
            {
                channel = RabbitMq::Instance()->CreateWriteChannel(this->_name);
                RabbitMq::Instance()->AddChannel(channel);
            }
            return channel;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s", this->_name.data(), ex.what());
        }
        return nullptr;
    }

    RabbitChannelPtr get_read_channel()
    {
        try
        {
            std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
            auto channel = RabbitMq::Instance()->GetReadChannel(this->_name);
            if (!channel)
            {
                channel = RabbitMq::Instance()->CreateReadChannel(this->_name, this->_on_consume, this->_qos);
                RabbitMq::Instance()->AddChannel(channel);
            }
            return channel;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s", this->_name.data(), ex.what());
        }
        return nullptr;
    }

private:
    int                 _qos;
    std::string         _name;
    OnConsume           _on_consume;
    std::shared_mutex   _mutex;
};
}
