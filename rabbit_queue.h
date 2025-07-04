#pragma once
#include <json_serialize.h>
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

using OnCallBack = std::function<bool()>;
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
        ERROR(message);
    }

    virtual void onConnected(AMQP::TcpConnection *connection) override 
    {
        INFO("connected");
    }

    virtual void onReady(AMQP::TcpConnection *connection) override 
    {
        INFO("ready");
        if (this->Ready)
        {
            this->Ready();
        }
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
            this->Detach();
        }
    }
public:
    std::future<void> _task;
    OnCallBack Detach;
    OnCallBack Ready;
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
        this->_channel->publish("", this->_name, data);
        /*
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
        */
    }

    void Start(const TcpConnectionPtr &connection)
    {
        if (!connection) return;
        if (this->_channel)
        {
            this->_channel->close();
        }
        if (this->_type == ChannelType::Read)
        {
            this->_channel = std::make_shared<AMQP::TcpChannel>(connection.get());
            this->_channel->setQos(this->_qos);
            this->_channel->declareQueue(this->_name, AMQP::durable).
            onSuccess([this](const std::string &name, int msgs, int consumers)
            {
                INFO("[%s]队列消费(consumer)声明成功, [msgs: %d][consumers: %d]", this->_name.data(), msgs, consumers);
            });
        }
        else
        {
            this->_channel = std::make_shared<AMQP::TcpChannel>(connection.get());
            this->_channel->declareQueue(this->_name, AMQP::durable).
            onSuccess([this](const std::string &name, int msgs, int consumers)
            {
                INFO("[%s]队列生产(publisher)声明成功, [msgs: %d][consumers: %d]", this->_name.data(), msgs, consumers);
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
            this->_loop = ev_loop_new(0);
            this->_handler = std::make_shared<RabbitMqEventHandler>(this->_loop);
            this->_handler->Detach = std::bind(&RabbitMq::ReStart, this);
            this->_handler->Ready = std::bind(&RabbitMq::ReStoreChannel, this);
            this->_connection = std::make_shared<AMQP::TcpConnection>(this->_handler.get(), AMQP::Address(this->_connection_str));
            INFO("启动事件线程");
            this->_loop_task = std::async(std::launch::async, [this]()
            { 
                INFO("事件监听开始");
                ev_run(this->_loop, 0); 
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
            this->_started.store(false);
            INFO("连接停止开始: [closed: %d][usable: %d][ready: %d]", 
                 this->_connection->closed(), this->_connection->usable(), this->_connection->ready());
            if (!this->_connection->closed())
            {
                INFO("连接未关闭，开始关闭...");

                this->_connection->close(true);
                INFO("连接关闭结束: [closed: %d][usable: %d][ready: %d]",
                    this->_connection->closed(), this->_connection->usable(), this->_connection->ready());
            }
            INFO("连接停止结束: [closed: %d][usable: %d][ready: %d]",
                this->_connection->closed(), this->_connection->usable(), this->_connection->ready());
            this->_handler = nullptr;
            this->_connection = nullptr;
            INFO("结束事件线程");
            ev_break(this->_loop, EVBREAK_ALL);
            this->_loop_task.wait();
            ev_loop_destroy(this->_loop);
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR(ex.what());
        }
        return false;
    }

    bool ReStart()
    {
        this->_on_detach_task = std::async(std::launch::async, [this]()
        {
            this->restart();
        });
        return true;
    }

    bool restart()
    {
        try
        {
            if (this->_reconnect_seconds.load() == 5)
            {
                this->_reconnect_seconds.store(0);
            }
            this->_reconnect_seconds++;
            INFO("开始重新连接, 等待[%d]秒后开始连接...", this->_reconnect_seconds.load());
            if (this->_restarting.exchange(true)) return true;
            this->_restart_count++;
            std::this_thread::sleep_for(std::chrono::seconds(this->_reconnect_seconds.load()));
            if (!this->Stop()) 
            {
                this->_restarting.store(false);
                INFO("连接停止失败: [closed: %d][usable: %d][ready: %d]",
                    this->_connection->closed(), this->_connection->usable(), this->_connection->ready());
                return false;
            }

            if (!this->Start(this->_connection_str))
            {
                this->_restarting.store(false);
                INFO("连接启动失败: [closed: %d][usable: %d][ready: %d]",
                    this->_connection->closed(), this->_connection->usable(), this->_connection->ready());
                return false;
            }
            INFO("连接启动成功: [closed: %d][usable: %d][ready: %d]", 
                 this->_connection->closed(), this->_connection->usable(), this->_connection->ready());
            this->_restarting.store(false);
            INFO("重新连接结束, 重连次数[%lld]...", this->_restart_count.load());
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR("重连接异常: %s", ex.what());
        }
        this->_restarting.store(false);
        return false;
    }

    bool ReStoreChannel()
    {
        this->_on_ready_task = std::async(std::launch::async, [this]()
        {
            this->restore_channel();
        });
        return true;
    }
    bool restore_channel()
    {
        try
        {
            if (this->_restart_count.load() == 0) return true;
            INFO("通道恢复开始");
            {
                INFO("消费者恢复开始: [通道个数: %llu]", this->_read_channels.size());
                std::shared_lock<std::shared_mutex> read_lock(this->_read_channel_mutex);
                for (auto itr : this->_read_channels)
                {
                    INFO("消费者恢复开始: [队列名称: %s]", itr.first.data());
                    itr.second->Start(this->_connection);
                    itr.second->Consume();
                    INFO("消费者恢复结束: [队列名称: %s]", itr.first.data());
                }
                INFO("消费者恢复成功: [通道个数: %llu]", this->_read_channels.size());
            }
            {
                INFO("生产者恢复开始: [通道个数: %llu]", this->_write_channels.size());
                std::shared_lock<std::shared_mutex> read_lock(this->_write_channel_mutex);
                for (auto itr : this->_write_channels)
                {
                    INFO("生产者恢复开始: [队列名称: %s]", itr.first.data());
                    itr.second->Start(this->_connection);
                    INFO("生产者恢复结束: [队列名称: %s]", itr.first.data());
                }
                INFO("生产者恢复成功: [通道个数: %llu]", this->_write_channels.size());
            }
            INFO("通道恢复结束");
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR("通道恢复异常: %s", ex.what());
        }
        return false;
    }

    TcpConnectionPtr Connection()
    {
        return this->_connection;
    }

    RabbitChannelPtr CreateReadChannel(const std::string &name, const OnConsume &on_consume, int qos)
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_connection_mutex);
        if (!this->_connection) return nullptr;
        auto channel = std::make_shared<RabbitChannel>(name, on_consume, qos);
        channel->Start(this->_connection);
        return channel;
    }

    RabbitChannelPtr CreateWriteChannel(const std::string &name)
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_connection_mutex);
        if (!this->_connection) return nullptr;
        auto channel = std::make_shared<RabbitChannel>(name);
        channel->Start(this->_connection);
        return channel;
    }

    RabbitChannelPtr GetReadChannel(const std::string &name, const OnConsume &on_consume, int qos)
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_read_channel_mutex);
        auto itr = this->_read_channels.find(name);
        if (itr == this->_read_channels.end())
        {
            auto channel = CreateReadChannel(name, on_consume, qos);
            this->_read_channels.insert({channel->Name(), channel});
            return channel;
        }
        return itr->second;
    }

    RabbitChannelPtr GetWriteChannel(const std::string &name)
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_write_channel_mutex);
        auto itr = this->_write_channels.find(name);
        if (itr == this->_write_channels.end())
        {
            auto channel = CreateWriteChannel(name);
            this->_write_channels.insert({channel->Name(), channel});
            return channel;
        }
        return itr->second;
    }

    static RabbitMq* Instance()
    {
        static RabbitMq rabbitmq;
        return &rabbitmq;
    }
private:
    struct ev_loop*                         _loop;
    std::atomic<int>                        _reconnect_seconds;
    std::atomic<bool>                       _started;
    std::atomic<bool>                       _restarting;
    std::atomic<int64_t>                    _restart_count;
    std::string                             _connection_str;
    std::shared_mutex                       _read_channel_mutex;
    std::shared_mutex                       _write_channel_mutex;
    std::shared_mutex                       _connection_mutex;
    std::future<void>                       _loop_task;
    std::future<void>                       _on_detach_task;
    std::future<void>                       _on_ready_task;
    RabbitMqEventHandlerPtr                 _handler;
    TcpConnectionPtr                        _connection;
    std::map<std::string, RabbitChannelPtr> _read_channels;
    std::map<std::string, RabbitChannelPtr> _write_channels;
};

class RabbitQueue
{
public:
    RabbitQueue(const std::string &name)
    :
    _name(name)
    {
    }

    void Consume(const OnConsume &on_consume, int qos = 100)
    {
        INFO("[%s]启动消费开始", this->_name.data());
        auto channel = RabbitMq::Instance()->GetReadChannel(this->_name, on_consume, qos);
        if (!channel) return;

        channel->Consume();
        INFO("[%s]启动消费结束", this->_name.data());
    }

    bool Publish(const std::string &data, uint8_t priority = 0)
    {
        try
        {
            auto channel = RabbitMq::Instance()->GetWriteChannel(this->_name);
            if (!channel) return false;

            channel->Publish(data, priority);
            INFO("[%s]生产数据: %s", this->_name.data(), data.data());
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s, 发送异常数据: %s", this->_name.data(), ex.what(), data.data());
        }
        return false;
    }

private:
    std::string         _name;
};
}
