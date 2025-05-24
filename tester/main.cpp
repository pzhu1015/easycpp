#include <iostream>
#include <chrono>
#include <ctemplate/template.h>
#include "json_serialize.h"
#include "param_serialize.h"
#include "tpl_serialize.h"

namespace test
{
class Test
{
public:
    std::string String;
    float Float;
    bool Bool;
    double Double;
    int8_t Int8;
    int16_t Int16;
    int32_t Int32;
    int64_t Int64;
    uint8_t UInt8;
    uint16_t UInt16;
    uint32_t UInt32;
    uint64_t UInt64;
    std::vector<int> Array;
    std::set<int> Set;
    std::list<int> List;
};

class Account
{
public:
    int64_t ID;
    bool Register;
    std::string Domain;
    std::string Password;
};

class DirectoryInDTO
{
public:
    std::string EventCallingFunction;
    std::string UserAgent;
    std::string Ip;
    std::string AuthName;
    std::string Domain;
};

class DirectoryOutDTO
{
public:
    std::shared_ptr<DirectoryInDTO> InDTO;
    std::shared_ptr<test::Account> Account;
};


static const std::string XmlDirectory = 
R"(
{{#Account}}
<document type="freeswitch/xml">
    <section name="directory">
        <domain name="{{Domain}}">
            <params>
                <param name="dial-string" value="{presence_id=${dialed_user}@${dialed_domain}}${sofia_contact(${dialed_user}@${dialed_domain})}"/>
            </params>
            <user id="{{ID}}" cacheable="60000">
                <params>
                    <param name="password" value="{{Password}}"/>
                </params>
                <variables>
                    <variable name="toll_allow" value="domestic,international,local"/>
                    <variable name="user_context" value="default"/>
                    <variable name="callgroup" value="default"/>
                    <variable name="effective_caller_id_name" value="{{ID}}"/>
                    <variable name="effective_caller_id_number" value="{{ID}}"/>
                    <variable name="outbound_caller_id_name" value="{{ID}}"/>
                    <variable name="outbound_caller_id_number" value="{{ID}}"/>
                    <variable name="register" value="{{Register}}"/>
                </variables>
            </user>
        </domain>
    </section>
</document>
{{/Account}}
)";
}

REGIST_MEMBER_TPL(
    test::Account,
    PLAIN(ID),
    PLAIN(Register),
    PLAIN(Domain),
    PLAIN(Password)
);

REGIST_MEMBER_TPL(
    test::DirectoryInDTO,
    PLAIN(EventCallingFunction),
    PLAIN(UserAgent),
    PLAIN(Ip),
    PLAIN(AuthName),
    PLAIN(Domain)
);

REGIST_MEMBER_TPL(
    test::DirectoryOutDTO,
    PLAIN(InDTO),
    PLAIN(Account)
);

//注册时要放到全局位置注册
REGIST_MEMBER_JSON(
    test::Test, 
    NAME(String,"string"),
    NAME(Float,"float"),
    NAME(Bool,"bool"),
    NAME(Double,"double"),
    NAME(Int8,"int8"),
    NAME(Int16,"int16"),
    NAME(Int32,"int32"),
    NAME(Int64,"int64"),
    NAME(UInt8,"uint8"),
    NAME(UInt16,"uint16"),
    NAME(UInt32,"uint32"),
    NAME(UInt64,"uint64"),
    NAME(Array,"array"),
    NAME(Set,"set"),
    NAME(List,"list")
);

REGIST_MEMBER_PARAM(
    test::Test,
    NAME(String,"string"),
    NAME(Float,"float"),
    NAME(Bool,"bool"),
    NAME(Double,"double"),
    NAME(Int8,"int8"),
    NAME(Int16,"int16"),
    NAME(Int32,"int32"),
    NAME(Int64,"int64"),
    NAME(UInt8,"uint8"),
    NAME(UInt16,"uint16"),
    NAME(UInt32,"uint32"),
    NAME(UInt64,"uint64")
);

void TestJsonSerialize()
{
    test::Test t;
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
    t.Array = {1, 2, 3};
    t.Set = {4, 5, 6, 7};
    t.List = {8, 9, 10};

    auto json = serialize::JsonSerializer<test::Test>::ToString(t);
    std::cout << "json =  " << json << std::endl;

    auto t2 = serialize::JsonSerializer<test::Test>::FromString(json);
    std::cout << "t2.String = " << t2.String << std::endl;

    auto t4 = serialize::JsonSerializer<test::Test>::FromStringPtr(json);
    auto json4 = serialize::JsonSerializer<test::Test>::ToString(t4);
    std::cout << "json4 = " << json4 << std::endl;

    std::set<std::string> sets = {"a", "b", "c"};
    auto setstr = serialize::JsonSerializer<std::set<std::string>>::ToString(sets);
    std::cout << "sets = " << setstr << std::endl;

    auto sets1 = serialize::JsonSerializer<std::set<std::string>>::FromString(setstr);
    sets1.emplace("d");
    auto setstr1 = serialize::JsonSerializer<std::set<std::string>>::ToString(sets1);
    std::cout << "set1 = " << setstr1 << std::endl;
}

void TestParamSerialize()
{
    test::Test t;
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

    auto params = serialize::ParamSerializer<test::Test>::ToString(t);
    std::cout << "params = " << params << std::endl;

    auto t3 = serialize::ParamSerializer<test::Test>::FromString(params);
    std::cout << "t3.String = " << t3.String << std::endl;

    auto t5 = serialize::ParamSerializer<test::Test>::FromStringPtr(params);
    auto param5 = serialize::ParamSerializer<test::Test>::ToString(t5);
    std::cout << "param5 = " << param5 << std::endl;
}

void TestTemplateSerialize()
{
    std::string out;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i=0; i < 100000; i++)
    {
        auto outdto = std::make_shared<test::DirectoryOutDTO>();
        outdto->InDTO = std::make_shared<test::DirectoryInDTO>();
        outdto->Account = std::make_shared<test::Account>();
        outdto->Account->ID = 8001;
        outdto->Account->Register = true;
        outdto->Account->Domain = "sbc.shyzhy.com";
        outdto->Account->Password = "123456";
        ctemplate::Template::StringToTemplateCache("directory.tpl", test::XmlDirectory);
        auto dict = std::make_shared<ctemplate::TemplateDictionary>("");
        tpl::TplSerializer<test::DirectoryOutDTO>::ToDictionary(outdto, dict.get());
        auto tpl = ctemplate::Template::GetTemplate("directory.tpl", ctemplate::DO_NOT_STRIP);
        out = "";
        tpl->Expand(&out, dict.get());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << out << std::endl;
    std::cout << "Time elapsed: " << std::fixed << std::setprecision(6) 
        << duration.count() << " seconds" << std::endl;
}

int main()
{
    TestJsonSerialize();
    TestParamSerialize();
    std::cout << test::XmlDirectory << std::endl;
    TestTemplateSerialize();
    return 0;
}

