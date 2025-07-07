#include <iostream>
#include <chrono>
#include <inja/inja.hpp>
#include "obj.h"
#include "json.h"
#include "templates.h"
#include "rabbit_queue.h"

void TestJsonSerialize()
{
    test::Object t;
    t.DateTime = datetime::DateTime::Now();
    t.Sub;
    t.Sub.String = "sub.string";
    t.Sub.Int32 = 200000000;
    t.SubPtr = std::make_shared<test::SubObject>();
    t.SubPtr->String = "sub_ptr.string";
    t.SubPtr->Int32 = 100000000;
    for (auto i=0; i < 2; i++)
    {
        auto sub_ptr = std::make_shared<test::SubObject>();
        sub_ptr->String = "vector_sub_ptr.string";
        sub_ptr->Int32 = i;
        t.VectorSubPtr.emplace_back(sub_ptr);
        
        test::SubObject sub;
        sub.String = "vector_sub.string";
        sub.Int32 = i;
        t.VectorSub.emplace_back(sub);
    }

    t.String = "string";
    t.Float = 3.14;
    t.Bool = true;
    t.Double = 3.1415926;
    t.Int8 = 127;
    t.Int16 = 32767;
    t.Int32 = 2000000000;
    t.Int64 = 8000000000;
    t.UInt8 = 255;
    t.UInt16 = 65535;
    t.UInt32 = 4200000000;
    t.UInt64 = 16000000000;

    t.VectorString = {"string1", "string2"};
    t.VectorFloat = {3.14, 3.15};
    t.VectorDouble = {3.1415926, 3.1415927};
    t.VectorInt8 = {-127, 127};
    t.VectorInt16 = {-32767, 32767};
    t.VectorInt32 = {-2000000000, 2000000000};
    t.VectorInt64 = {-8000000000, 8000000000};
    t.VectorUInt8 = {0, 255};
    t.VectorUInt16 = {0, 65535};
    t.VectorUInt32 = {0, 4200000000};
    t.VectorUInt64 = {0, 16000000000};

    t.ListString = {"string1", "string2"};
    t.ListFloat = {3.14, 3.15};
    t.ListDouble = {3.1415926, 3.1415927};
    t.ListInt8 = {-127, 127};
    t.ListInt16 = {-32767, 32767};
    t.ListInt32 = {-2000000000, 2000000000};
    t.ListInt64 = {-8000000000, 8000000000};
    t.ListUInt8 = {0, 255};
    t.ListUInt16 = {0, 65535};
    t.ListUInt32 = {0, 4200000000};
    t.ListUInt64 = {0, 16000000000};

    t.SetString = {"string1", "string2"};
    t.SetFloat = {3.14, 3.15};
    t.SetDouble = {3.1415926, 3.1415927};
    t.SetInt8 = {-127, 127};
    t.SetInt16 = {-32767, 32767};
    t.SetInt32 = {-2000000000, 2000000000};
    t.SetInt64 = {-8000000000, 8000000000};
    t.SetUInt8 = {0, 255};
    t.SetUInt16 = {0, 65535};
    t.SetUInt32 = {0, 4200000000};
    t.SetUInt64 = {0, 16000000000};


    auto json = serialize::JsonSerializer<test::Object>::ToString(t);
    std::cout << "json =  " << json << std::endl;
    std::cout << "======================================================" << std::endl;

    auto t1 = serialize::JsonSerializer<test::Object>::FromString(json);
    std::cout << "t1.String = " << t1.String << std::endl;
    std::cout << "t1.DateTime = " << t1.DateTime.ToString() << std::endl;
    t1.DateTime = t1.DateTime.Add(std::chrono::minutes(10));
    std::cout << "t1.DateTime + 10(minute) = " << t1.DateTime.ToString() << std::endl;
    std::cout << "======================================================" << std::endl;

    auto t2 = serialize::JsonSerializer<test::Object>::FromStringPtr(json);
    t2->DateTime = t2->DateTime.Add(std::chrono::minutes(10));
    t2->String = "modify.string";
    auto json2 = serialize::JsonSerializer<test::Object>::ToString(t2);
    std::cout << "json2 = " << json2 << std::endl;
    std::cout << "======================================================" << std::endl;

    std::set<std::string> sets = {"sa", "sb", "sc"};
    auto setstr = serialize::JsonSerializer<std::set<std::string>>::ToString(sets);
    std::cout << "setstr = " << setstr << std::endl;
    auto setjson = serialize::JsonSerializer<std::set<std::string>>::ToJson(sets);
    std::cout << "setjson = " << setjson << std::endl;
    std::cout << "======================================================" << std::endl;

    auto sets1 = serialize::JsonSerializer<std::set<std::string>>::FromString(setstr);
    sets1.emplace("sd");
    auto setstr1 = serialize::JsonSerializer<std::set<std::string>>::ToString(sets1);
    std::cout << "setstr1 = " << setstr1 << std::endl;
    std::cout << "======================================================" << std::endl;

    std::vector<std::string> vectors = {"va", "vb", "vc"};
    auto vectorstr = serialize::JsonSerializer<std::vector<std::string>>::ToString(vectors);
    std::cout << "vectorstr= " << vectorstr << std::endl;
    auto vectorjson = serialize::JsonSerializer<std::vector<std::string>>::ToJson(vectors);
    std::cout << "vectorjson = " << vectorjson << std::endl;
    std::cout << "======================================================" << std::endl;

    auto vectors1 = serialize::JsonSerializer<std::vector<std::string>>::FromString(vectorstr);
    vectors1.emplace_back("vd");
    auto vectorstr1 = serialize::JsonSerializer<std::vector<std::string>>::ToString(vectors1);
    std::cout << "vectorstr1 = " << vectorstr1 << std::endl;
    std::cout << "======================================================" << std::endl;

    auto vector_ptr_str = serialize::JsonSerializer<std::vector<std::shared_ptr<test::SubObject>>>::ToString(t.VectorSubPtr);
    std::cout << "vectorptr_str = " << vector_ptr_str << std::endl;
    auto vector_ptr = serialize::JsonSerializer<std::vector<std::shared_ptr<test::SubObject>>>::FromString(vector_ptr_str);
    auto vector_ptr_str1 = serialize::JsonSerializer<std::vector<std::shared_ptr<test::SubObject>>>::ToString(vector_ptr);
    std::cout << "vectorptr_str1 = " << vector_ptr_str1 << std::endl;
    std::cout << "======================================================" << std::endl;
}

void TestParamSerialize()
{
    test::Object t;
    t.String = "std::string";
    t.Float = 3.14;
    t.Bool = true;
    t.Double = 3.1415926;
    t.Int8 = -127;
    t.Int16 = -32760;
    t.Int32 = -210000000;
    t.Int64 = -8000000000;
    t.UInt8 = 255;
    t.UInt16 = 65535;
    t.UInt32 = 4200000000;
    t.UInt64 = 80000000000;

    auto params = serialize::ParamSerializer<test::Object>::ToString(t);
    std::cout << "params = " << params << std::endl;

    auto t3 = serialize::ParamSerializer<test::Object>::FromString(params);
    std::cout << "t3.String = " << t3.String << std::endl;

    auto t5 = serialize::ParamSerializer<test::Object>::FromStringPtr(params);
    auto param5 = serialize::ParamSerializer<test::Object>::ToString(t5);
    std::cout << "param5 = " << param5 << std::endl;
}

void TestTemplateSerialize()
{
    try
    {
        inja::Environment env;
        auto directory_tpl = env.parse(XmlDirectory);
        auto sofia_tpl = env.parse(XmlSofia);
        auto extension_gateway_tpl = env.parse(XmlExtensionGatewayDialplan);
        std::string out;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i=0; i < 100000; i++)
        {
            auto outdto = std::make_shared<test::DirectoryOutDTO>();
            outdto->Account = std::make_shared<test::Account>();
            outdto->Account->ID = 8001;
            outdto->Account->Domain = "sbc.shyzhy.com";
            outdto->Account->Password = "123456";
            auto json = serialize::JsonSerializer<test::DirectoryOutDTO>::ToJson(outdto);
            out = env.render(directory_tpl, json);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << out << std::endl;
        std::cout << "Time elapsed: " << std::fixed << std::setprecision(6) 
            << duration.count() << " seconds" << std::endl;

    }
    catch(std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }
}

void TestDateTime()
{
    datetime::DateTime dt;
    std::cout << dt.IsZero() << std::endl;

    auto dt2 = datetime::DateTime::Parse("2025-06-21 00:00:00");
    std::cout << dt2.ToString() << std::endl;

    auto now = datetime::DateTime::Now();
    std::cout << now.ToString() << std::endl;

    auto add = now.Add(std::chrono::minutes(10));
    std::cout << add.ToString() << std::endl;
}

void TestRabbitMq()
{
    queue::RabbitMq::Instance()->Start("amqp://admin:admin@127.0.0.1:5672/");    

    std::vector<std::thread> threads;
    for (int i=0; i < 100; i++)
    {
        threads.emplace_back(std::thread([](int idx)
        {
            std::atomic<int64_t> id;
            std::set<int64_t> objs;
            std::shared_mutex mu;

            auto CountObj = [&objs, &mu]() -> size_t
            {
                std::shared_lock<std::shared_mutex> read_lock(mu);
                return objs.size();
            };

            auto AddObj = [&objs, &mu](int int32)
            {
                std::unique_lock<std::shared_mutex> write_lock(mu);
                objs.insert(int32);
            };

            auto RemoveObj = [&objs, &mu](int int32)
            {
                std::unique_lock<std::shared_mutex> write_lock(mu);
                objs.erase(int32);
            };

            std::string queue_name = "test_";
            queue_name.append(std::to_string(idx));
            auto q = std::make_shared<queue::RabbitQueue>(queue_name);
            q->Consume([&RemoveObj](const std::string &data)
            {
                auto obj = serialize::JsonSerializer<test::SubObject>::FromStringPtr(data);
                RemoveObj(obj->Int32);
                return true;
            }, 500);
            for (int n=0; n < 10; n++)
            {
                auto obj = std::make_shared<test::SubObject>();
                obj->Int32 = id++;
                obj->String = "测试队列";
                q->Publish(serialize::JsonSerializer<test::SubObject>::ToString(obj).data());
                AddObj(obj->Int32);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            while(1)
            {
                auto count = CountObj();
                if (count > 0)
                {
                    ERROR("=============================================%s]: %llu", queue_name.data(), count);
                }
                else
                {
                    INFO("==============================================%s]: %llu", queue_name.data(), count);
                }

                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }, i));
    }

    while(1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    INFO("测试开始");
    TestRabbitMq();
    //TestDateTime();
    //TestJsonSerialize();
    //TestParamSerialize();
    //std::cout << XmlDirectory << std::endl;
    //TestTemplateSerialize();
    return 0;
}

