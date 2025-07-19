#include <iostream>
#include <chrono>
#include <inja/inja.hpp>
#include <shared_mutex>
#include <aho_corasick/aho_corasick.hpp>
#include "obj.h"
#include "json.h"
#include "param.h"
#include "xml.h"
#include "templates.h"
#include "rabbit_queue.h"
#include "phonedata.h"
#include "local_cache.h"
#include "expire_cache.h"
#include "encoding.h"
#include "ratelimit.h"

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


    auto json = serialize::JsonSerializer<test::Object>::ToString(t, 4);
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

void TestXmlSerialize()
{
    test::Extension extension;
    auto registration = std::make_shared<test::Registration>();
    registration->CallId = "xxx";
    extension.Registrations.push_back(registration);
    auto data = serialize::XmlSerializer<test::Extension>::ToString(extension, true);
    INFO("\n%s", data.data());

    auto extension1 = serialize::XmlSerializer<test::Extension>::FromStringPtr(data);
    INFO("%d", extension1->Registrations.size());
    auto data1 = serialize::XmlSerializer<test::Extension>::ToString(extension1, true);
    INFO("\n%s", data1.data());

    //test::Gateway gateway;
    //gateway.Name = "1000";
    //auto data = serialize::XmlSerializer<test::Gateway>::ToString(gateway, true);
    //INFO("\n%s", data.data());

    //auto gateway1 = serialize::XmlSerializer<test::Gateway>::FromStringPtr(data);
    //auto data1 = serialize::XmlSerializer<test::Gateway>::ToString(gateway1, true);
    //INFO("\n%s", data1.data());
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
            });
            for (int n=0; n < 1000; n++)
            {
                auto obj = std::make_shared<test::SubObject>();
                obj->Int32 = id++;
                obj->String = "测试队列";
                auto result = q->Publish(serialize::JsonSerializer<test::SubObject>::ToString(obj).data());
                if (result) AddObj(obj->Int32);
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            }

            while(1)
            {
                auto count = CountObj();
                if (count > 0)
                {
                    ERROR("=============================================%s]: %llu", queue_name.data(), count);
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

void TestPhoneData()
{
    std::vector<std::string> numbers = {
        "15695866526",
        "18069202072",
        "13819813415",
        "15306627776",
        "13355915301",
        "13262429999",
        "15824205247",
        "18167219893",
        "15968925439",
        "15051515852",
        "80901142",
    };
    for (const auto &number: numbers)
    {
        auto pr = phonedata::PhoneData::Instance()->Find(number);
        if (!pr)
        {
            ERROR("未找到此手机号信息");
            return;
        }
        INFO("[%s][%s-%s][%s]", pr->Number.data(), pr->Province.data(), pr->City.data(), pr->CardType.data());
    }
}

void TestExpireCache()
{
    cache::ExpireCache<int64_t, test::SubObject> caches([](const int64_t &key, const std::shared_ptr<test::SubObject> &value, bool is_manual)
    {
        INFO("key: %lld, value: %s, is_manual: %d", 
             key, serialize::JsonSerializer<test::SubObject>::ToString(value).data(), is_manual);
    }, std::chrono::milliseconds(30000));
    auto sub1 = std::make_shared<test::SubObject>();
    sub1->Int32 = 1;
    sub1->String = "1111";
    caches.Put(sub1->Int32, sub1);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    auto sub2 = std::make_shared<test::SubObject>();
    sub2->Int32 = 2;
    sub2->String = "2222";
    caches.Put(sub2->Int32, sub2);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    auto sub3 = std::make_shared<test::SubObject>();
    sub3->Int32 = 3;
    sub3->String = "3333";
    caches.Put(sub3->Int32, sub3);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    auto sub4 = std::make_shared<test::SubObject>();
    sub4->Int32 = 4;
    sub4->String = "4444";
    caches.Put(sub4->Int32, sub4);
    caches.Delete(sub1->Int32);
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void TestLocalCache()
{
    cache::LocalCache<int64_t, test::SubObject> caches;
    auto sub1 = std::make_shared<test::SubObject>();
    sub1->Int32 = 1;
    sub1->String = "1111";
    auto itr = caches.Put(sub1->Int32, sub1);
    INFO("是否存在: %d, value: %s", itr.second, serialize::JsonSerializer<test::SubObject>::ToString(itr.first).data());
    auto sub2 = std::make_shared<test::SubObject>();
    sub2->Int32 = 1;
    sub2->String = "2222";
    itr = caches.Put(sub1->Int32, sub1);
    INFO("是否存在: %d, value: %s", itr.second, serialize::JsonSerializer<test::SubObject>::ToString(itr.first).data());
}

void TestEncoding()
{
    std::string utf8_data = "【短信测试】 我是普通短信，编码GBK, 没有表情图 end";
    auto data = encoding::Encoding::GBK->FromUtf8(utf8_data);
    INFO("[utf8: %d, gbk: %d=50]%s", utf8_data.size(), data.size(), utf8_data.data());

    utf8_data = "[ASCII1] I am normal short message, smpp protocol end";
    data = encoding::Encoding::ASCII->FromUtf8(utf8_data);
    INFO("[utf8: %d, ascii: %d=53]%s", utf8_data.size(), data.size(), utf8_data.data());

    utf8_data = "【短信测试】 我是普通短信，编码UCS2, 表情图[🐳][🐠] end";
    data = encoding::Encoding::UCS2->FromUtf8(utf8_data);
    INFO("[utf8: %d, ucs2: %d=74]%s", utf8_data.size(), data.size(), utf8_data.data());

    utf8_data = "[LATIN1] I am normal short message, smpp protocol Ó®end";
    data = encoding::Encoding::LATIN1->FromUtf8(utf8_data);
    INFO("[utf8: %d, latin1: %d=55]%s", utf8_data.size(), data.size(), utf8_data.data());

    utf8_data = "[GSM7] I am @£$¥èéùìòÇØøÅΔ_ΦΓΛΩΠΨΣΘΞÆæßÉ, smpp protocol end";
    data = encoding::Encoding::GSM7->FromUtf8(utf8_data);
    INFO("[utf8: %d, gsm7: %d=74]%s", utf8_data.size(), data.size(), utf8_data.data());
}

void TestTrie()
{
    aho_corasick::trie trie;
    trie.insert("韩福才青海");
    trie.insert("欧阳德广东");
    trie.insert("韦泽芳海南");
    trie.insert("铁英北京");
    trie.insert("辛业江海南");
    trie.insert("于飞广东");
    trie.insert("姜殿武河北");
    trie.insert("cmpp2.0");
    trie.insert("秦昌典重庆");
    trie.insert("范广举黑龙江");
    trie.insert("张凯广东");
    trie.insert("王厚宏海南");
    trie.insert("陈维席安徽");
    trie.insert("王有杰河南");
    trie.insert("王武龙江苏");
    trie.insert("米凤君吉林");
    trie.insert("宋勇辽宁");
    trie.insert("张家盟浙江");
    trie.insert("马烈孙宁夏");
    trie.insert("黄纪诚北京");
    trie.insert("常征贵州");
    trie.insert("王式惠重庆");
    trie.insert("周文吉");
    trie.insert("王庆录广西");
    trie.insert("潘广田山东");
    trie.insert("朱作勇甘肃");
    trie.insert("孙善武河南");
    trie.insert("宋晨");
    trie.insert("梁春禄广西政协");
    trie.insert("鲁家善中国交通");
    trie.insert("金德琴中信");
    trie.insert("李大强神华");
    trie.insert("吴文英纺织");
    trie.insert("查克明华能");
    trie.insert("朱小华光大");
    trie.insert("高严国家电力");
    trie.insert("王雪冰");
    trie.insert("林孔兴");
    trie.insert("刘金宝");
    trie.insert("张恩照");
    trie.insert("陈同海");
    trie.insert("康日新");
    trie.insert("王益");
    trie.insert("张春江");
    trie.insert("洪清源");
    trie.insert("平义杰");
    trie.insert("李恩潮");
    trie.insert("孙小虹");
    trie.insert("陈忠");
    trie.insert("慕绥新");
    trie.insert("田凤岐");
    trie.insert("麦崇楷");
    trie.insert("柴王群");
    trie.insert("吴振汉");
    trie.insert("张秋阳");
    trie.insert("徐衍东");
    trie.insert("徐发黑龙江");
    trie.insert("张宗海");
    trie.insert("丁鑫发");
    trie.insert("徐国健");
    trie.insert("李宝金");
    trie.insert("单平");
    trie.insert("段义和");
    trie.insert("荆福生");
    trie.insert("陈少勇");
    trie.insert("黄松有");
    trie.insert("皮黔生");
    trie.insert("王华元");
    trie.insert("王守业");
    trie.insert("刘连昆");
    trie.insert("孙晋美");
    trie.insert("邵松高");
    trie.insert("肖怀枢");
    trie.insert("刘广智空军");
    trie.insert("姬胜德总参");
    trie.insert("廖伯年北京");
    trie.insert("陈水遍");
    trie.insert("台湾");
    trie.insert("发送");
    trie.insert("竞选");
    auto start = std::chrono::high_resolution_clock::now();
    for (int i=0; i < 1; i++)
    {
        auto result = trie.parse_text("【中国移动】 陈少勇发送给台湾的短信，陈水遍竞选");
        if (result.empty())
        {
            ERROR("没有匹配到");
        }
        for (const auto& emit : result) 
        {
            std::cout << "匹配到关键词: " << emit.get_keyword() << "，起始位置: " << emit.get_start() << "，结束位置: " << emit.get_end() << std::endl;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time elapsed: " << std::fixed << std::setprecision(6) 
        << duration.count() << " seconds" << std::endl;
}

void TestRateLimit()
{
    using namespace ratelimit;
    std::atomic<int> speed = 0;
    auto rate = 10000.0;
    auto burst = 10000;
    RateLimiter limiter(rate, burst);
    auto thread = std::thread([&](){
        while(true)
        {
            INFO("speed: %d", speed.load());
            speed.store(0);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
    while(true)
    {
        speed++;
        limiter.Wait();
    }
}

int main()
{
    INFO("测试开始");
    //TestRateLimit();
    //TestTrie();
    //TestEncoding();
    //TestLocalCache();
    //TestExpireCache();
    //TestPhoneData();
    //TestRabbitMq();
    //TestDateTime();
    //TestJsonSerialize();
    //TestParamSerialize();
    TestXmlSerialize();
    //std::cout << XmlDirectory << std::endl;
    //TestTemplateSerialize();
    return 0;
}

