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
#include "sem.h"

#ifdef EASYCPP_LOGGING
#include "logger.h"
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
template <typename BASE=AMQP::Tagger>
using ReliablePtr = std::shared_ptr<AMQP::Reliable<BASE>>;

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

    void Wait()
    {
        _sem.wait();
    }

    void Signal()
    {
        _sem.post();
    }

private:
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        ERROR(message);
        this->Signal();
    }

    virtual void onConnected(AMQP::TcpConnection *connection) override 
    {
        INFO("connected");
    }

    virtual void onReady(AMQP::TcpConnection *connection) override 
    {
        INFO("ready");
        this->Signal();
        if (this->Ready)
        {
            this->Ready();
        }
    }

    virtual void onClosed(AMQP::TcpConnection *connection) override 
    {
        ERROR("closed");
        this->Signal();
    }

    virtual void onDetached(AMQP::TcpConnection *connection) override 
    {
        ERROR("detached");                
        this->Signal();
        if (this->Detach)
        {
            this->Detach();
        }
    }

public:
    Semaphore                   _sem;

    OnCallBack                  Detach;
    OnCallBack                  Ready;

};
using RabbitMqEventHandlerPtr = std::shared_ptr<RabbitMqEventHandler>;

enum class ChannelType
{
    Read = 0,
    Write = 1
};

static std::string ToString(ChannelType type)
{
    if (type == ChannelType::Read)
    {
        return "Consumer";
    }
    else if (type == ChannelType::Write)
    {
        return "Publisher";
    }
    else
    {
        return "Unknown";
    }
}

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

        Semaphore sem;
        this->_channel->consume("").
        onReceived([this](
            const AMQP::Message &message,
            uint64_t delivery_tag,
            bool redelivered)
        {
            try
            {
                auto data = std::string(message.body(), message.bodySize());
                INFO("[%s] (%s)onReceived: [%llu]%s", this->_name.data(), ToString(this->_type).data(), delivery_tag, data.data());
                if (this->_on_consume && this->_on_consume(data))
                {
                    if (this->_channel)
                    {
                        this->_channel->ack(delivery_tag);
                    }
                }
            }
            catch(std::exception &ex)
            {
                ERROR("[%s] %s", this->_name.data(), ToString(this->_type).data(), ex.what());
            }
        }).
        onSuccess([this, &sem]()
        {
            INFO("[%s] (%s)onSuccess", this->_name.data(), ToString(this->_type).data());
            sem.post();
        }).
        onError([this, &sem](const char* message)
        {
            ERROR("[%s] (%s)onError: %s", this->_name.data(), ToString(this->_type).data(), message);
            sem.post();
        });
        sem.timed_wait(5000);
    }

    void Publish(const std::string &data, uint8_t priority)
    {
        if (!this->_channel || !this->_reliable) return;

        AMQP::Envelope envelope(data);
        envelope.setPriority(priority);
        this->_reliable->publish("", this->_name, envelope).
        onAck([this]()
        {
            INFO("[%s] (%s)onAck", this->_name.data(), ToString(this->_type).data());
        }).
        onNack([this]()
        {
            INFO("[%s] (%s)onNack", this->_name.data(), ToString(this->_type).data());
        }).
        onLost([this]()
        {
            ERROR("[%s] (%s)onLost", this->_name.data(), ToString(this->_type).data());
        }).
        onError([this](const char* message)
        {
            ERROR("[%s] (%s)onError: %s", this->_name.data(), ToString(this->_type).data(), message);
        });
    }

    void Start(const TcpConnectionPtr &connection)
    {
        if (!connection) return;
        if (this->_channel)
        {
            this->_channel->close();
        }

        Semaphore sem;
        this->_channel = std::make_shared<AMQP::TcpChannel>(connection.get());
        this->_channel->onReady([this]()
        {
            INFO("[%s] (%s) Channel onReady", this->_name.data(), ToString(this->_type).data());
        });
        this->_channel->onError([this, &sem](const char* message)
        {
            ERROR("[%s] (%s) Channel onError: %s", this->_name.data(), ToString(this->_type).data(), message);
            sem.post();
        });
        this->_channel->declareQueue(this->_name, AMQP::durable).
        onSuccess([this, &sem](const std::string &name, int msgs, int consumers)
        {
            INFO("[%s] (%s) Queue onSuccess, [msgs: %d][consumers: %d]", this->_name.data(), ToString(this->_type).data(), msgs, consumers);
            sem.post();
        });
        sem.timed_wait(5000);
        if (this->_type == ChannelType::Write)
        {
            this->_reliable = std::make_shared<AMQP::Reliable<>>(*(this->_channel.get()));
        }
        else
        {
            this->_channel->setQos(this->_qos);
            this->Consume();
        }
        INFO("[%s](%s) Start", this->_name.data(), ToString(this->_type).data());
    }
private:
    int             _qos;
    std::string     _name;
    OnConsume       _on_consume;
    ChannelType     _type;
    TcpChannelPtr   _channel;
    ReliablePtr<>   _reliable;
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
            this->_handler->Detach = std::bind(&RabbitMq::Detach, this);
            this->_handler->Ready = std::bind(&RabbitMq::Ready, this);
            this->_connection = std::make_shared<AMQP::TcpConnection>(this->_handler.get(), AMQP::Address(this->_connection_str));
            INFO("启动事件线程");
            this->_loop_task = std::async(std::launch::async, [this]()
            { 
                INFO("事件监听开始");
                ev_run(this->_loop, 0); 
                INFO("事件监听结束");
            });
            if (this->_restart_count.load() == 0)
            {
                this->_handler->Wait();
            }
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
            INFO("连接停止开始...");
            if (!this->_connection->closed())
            {
                INFO("连接未关闭，开始关闭...");
                this->_connection->close(true);
                INFO("连接关闭结束...");
            }
            INFO("连接停止结束...");
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

    bool Detach()
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

    bool Ready()
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

    RabbitChannelPtr CreateReadChannel(const std::string &name, const OnConsume &on_consume, int qos)
    {
        std::unique_lock<std::mutex> lock(this->_connection_mutex);
        if (!this->_connection) return nullptr;
        auto channel = std::make_shared<RabbitChannel>(name, on_consume, qos);
        channel->Start(this->_connection);
        return channel;
    }

    RabbitChannelPtr CreateWriteChannel(const std::string &name)
    {
        std::unique_lock<std::mutex> lock(this->_connection_mutex);
        if (!this->_connection) return nullptr;
        auto channel = std::make_shared<RabbitChannel>(name);
        channel->Start(this->_connection);
        return channel;
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
    struct ev_loop*                         _loop;
    std::atomic<int>                        _reconnect_seconds;
    std::atomic<bool>                       _started;
    std::atomic<bool>                       _restarting;
    std::atomic<int64_t>                    _restart_count;
    std::string                             _connection_str;
    std::mutex                              _connection_mutex;
    std::shared_mutex                       _read_channel_mutex;
    std::shared_mutex                       _write_channel_mutex;
    std::future<void>                       _loop_task;
    std::future<void>                       _on_detach_task;
    std::future<void>                       _on_ready_task;
    std::map<std::string, RabbitChannelPtr> _read_channels;
    std::map<std::string, RabbitChannelPtr> _write_channels;
    RabbitMqEventHandlerPtr                 _handler;
    TcpConnectionPtr                        _connection;
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
        auto channel = this->get_read_channel(on_consume, qos);
        if (!channel) return;

        //channel->Consume();
    }

    void Publish(const std::string &data, uint8_t priority = 0)
    {
        try
        {
            auto channel = this->get_write_channel();
            if (!channel) return;

            channel->Publish(data, priority);
            return;
        }
        catch(std::exception &ex)
        {
            ERROR("[%s] %s, 发送异常数据: %s", this->_name.data(), ex.what(), data.data());
        }
        return;
    }

private:
    RabbitChannelPtr get_read_channel(const OnConsume &on_consume, int qos)
    {
        std::unique_lock<std::mutex> lock(this->_read_mutex);
        auto channel = RabbitMq::Instance()->GetReadChannel(this->_name);
        if (!channel)
        {
            channel = RabbitMq::Instance()->CreateReadChannel(this->_name, on_consume, qos);
            RabbitMq::Instance()->AddChannel(channel);
            return channel;
        }
        return channel;
    }

    RabbitChannelPtr get_write_channel()
    {
        std::unique_lock<std::mutex> lock(this->_write_mutex);
        auto channel = RabbitMq::Instance()->GetWriteChannel(this->_name);
        if (!channel)
        {
            channel = RabbitMq::Instance()->CreateWriteChannel(this->_name);
            RabbitMq::Instance()->AddChannel(channel);
            return channel;
        }
        return channel;

    }
    std::string _name;
    std::mutex  _read_mutex;
    std::mutex  _write_mutex;
};
}
