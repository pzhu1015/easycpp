#include <iostream>
#include <chrono>
#include <inja/inja.hpp>
//#include "json_serialize.h"
#include "nlohmann_json_serialize.h"
#include "param_serialize.h"
#include "license.h"

namespace test
{
class SubObject
{
public:
    std::string String;
    int32_t Int32;
};

class Object
{
public:
    SubObject Sub;
    std::shared_ptr<SubObject> SubPtr;
    std::vector<SubObject> VectorSub;
    std::vector<std::shared_ptr<SubObject>> VectorSubPtr;
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

    std::vector<std::string> VectorString;
    std::vector<float> VectorFloat;
    std::vector<double> VectorDouble;
    std::vector<int8_t> VectorInt8;
    std::vector<int16_t> VectorInt16;
    std::vector<int32_t> VectorInt32;
    std::vector<int64_t> VectorInt64;
    std::vector<uint8_t> VectorUInt8;
    std::vector<uint16_t> VectorUInt16;
    std::vector<uint32_t> VectorUInt32;
    std::vector<uint64_t> VectorUInt64;

    std::list<std::string> ListString;
    std::list<float> ListFloat;
    std::list<double> ListDouble;
    std::list<int8_t> ListInt8;
    std::list<int16_t> ListInt16;
    std::list<int32_t> ListInt32;
    std::list<int64_t> ListInt64;
    std::list<uint8_t> ListUInt8;
    std::list<uint16_t> ListUInt16;
    std::list<uint32_t> ListUInt32;
    std::list<uint64_t> ListUInt64;

    std::set<std::string> SetString;
    std::set<float> SetFloat;
    std::set<double> SetDouble;
    std::set<int8_t> SetInt8;
    std::set<int16_t> SetInt16;
    std::set<int32_t> SetInt32;
    std::set<int64_t> SetInt64;
    std::set<uint8_t> SetUInt8;
    std::set<uint16_t> SetUInt16;
    std::set<uint32_t> SetUInt32;
    std::set<uint64_t> SetUInt64;
};

class Account
{
public:
    int64_t ID;
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
<document type="freeswitch/xml">
    <section name="directory">
        <domain name="{{Account.Domain}}">
            <params>
                <param name="dial-string" value="{presence_id=${dialed_user}@${dialed_domain}}${sofia_contact(${dialed_user}@${dialed_domain})}"/>
            </params>
            <user id="{{Account.ID}}" cacheable="60000">
                <params>
                    <param name="password" value="{{Account.Password}}"/>
                </params>
                <variables>
                    <variable name="toll_allow" value="domestic,international,local"/>
                    <variable name="user_context" value="default"/>
                    <variable name="callgroup" value="default"/>
                    <variable name="effective_caller_id_name" value="{{Account.ID}}"/>
                    <variable name="effective_caller_id_number" value="{{Account.ID}}"/>
                    <variable name="outbound_caller_id_name" value="{{Account.ID}}"/>
                    <variable name="outbound_caller_id_number" value="{{Account.ID}}"/>
                </variables>
            </user>
        </domain>
    </section>
</document>
)";
static const std::string XmlSofia = 
R"(
<document type="freeswitch/xml">
    <section name="configuration" description="Various Configuration">
        <configuration name="sofia.conf" description="sofia Endpoint">
            <profiles>
                <profile name="public">
                    <gateways>
                        {% for gw in Gateways %}
                        <gateway name="{{gw.ID}}">
                            <param name="context" value="{{gw.Context}}"/>
                            <param name="register" value="{{gw.Register}}"/>
                            {% if gw.Register %}
                            <param name="username" value="{{gw.Username}}"/>
                            <param name="auth-username" value="{{gw.AuthUsername}}"/>
                            <param name="password" value="{{gw.Password}}"/>
                            <param name="register-transport" value="{{gw.RegisterTransport}}"/>
                            <param name="realm" value="{{gw.Realm}}"/>
                            <param name="register-proxy" value="{{gw.RegisterProxy}}"/>
                            {% endif %}
                            <param name="proxy" value="{{gw.Proxy}}"/>
                            <param name="outbound-proxy" value="{{gw.OutboundProxy}}"/>
                            <param name="expire-seconds" value="{{gw.ExpireSeconds}}"/>
                            <param name="retry-seconds" value="{{gw.RetrySeconds}}"/>
                            <param name="ping" value="{{gw.Ping}}"/>
                            <param name="caller-id-in-from" value="{{gw.CallerIDInFrom}}"/>
                            <param name="contact-in-ping" value="{{gw.ContactInPing}}"/>
                            {% if gw.FromUser != "" %}
                            <param name="from-user" value="{{gw.FromUser}}"/>
                            {% endif %}
                            {% if gw.FromDomain != "" %}
                            <param name="from-domain" value="{{gw.FromDomain}}"/>
                            {% endif %}
                            {% if gw.ContactParams != "" %}
                            <param name="contact-params" value="{{gw.ContactParams}}"/>
                            {% endif %}
                            <param name="extension-in-contact" value="{{gw.ExtensionInContact}}"/>
                            {% if gw.Extension != "" %}
                            <param name="extension" value="{{gw.Extension}}"/>
                            {% endif %}
                            <variables>
                                {% for var in gw.Variables %}
                                <variable name="{{var.Name}}" value="{{var.Value}}" direction="{{var.Direction}}"/>
                                {% endfor %}
                            </variables>
                        </gateway>
                        {% endfor %}
                    </gateways>
                </profile>
            </profiles>
        </configuration>
    </section>
</document>
)";

static const std::string XmlExtensionGatewayDialplan = 
R"(
<document type="freeswitch/xml">
    <section name="dialplan" description="RE Dial Plan For FreeSwitch">
        <context name="{{InDTO.CallerContext}}">
            <extension name="extension_gateway_dialplan" continue="false">
                <condition field="destination_number" expression="^${destination_number}$" break="on-false">
                    <action application="log" data="用户网关:[{{ReverseAccount}}], 账号主叫[${caller_id_number}} => {{XNumber}}], 被叫[${destination_number}]"/>
                    <action application="set" data="call_timeout=$${call_timeout}"/>
                    <action application="set" data="execute_on_answer_sched=sched_hangup +$${hugup_duration}"/>
                    <action application="set" data="hangup_after_bridge=true"/>
                    <action application="set" data="ringback=$${cn-ring}"/>
                    <action application="set" data="effective_caller_id_name={{XNumber}}"/>
                    <action application="set" data="effective_caller_id_number={{XNumber}}"/>
                    <action application="export" data="nolocal:absolute_codec_string=${global_codec_prefs}"/>
                </condition>
                {% if HangupCause != "" %}
                <condition field="destination_number" expression="^${destination_number}$" break="on-true">
                    <action application="export" data="hangup_cause__={{HangupCause}}"/>
                    <action application="hangup" data="NORMAL_TEMPORARY_FAILURE"/>
                </condition>
                {% endif %}
            </extension>
        </context>
    </section>
</document>
)";
/*

                {{% if CallTimeRange != "" %}}
                <condition wday="1,2,3,4,5,6,7" time-of-day="{{CallTimeRange}}" break="on-false">
                    <anti-action application="export" data="hangup_cause__=时间限制"/>
                    <anti-action application="hangup" data="CALL_REJECTED"/>
                </condition>
                {{% endif %}}
                <condition field="${sofia_contact(default/{{ReverseAccount}})}" expression="^error/user_not_registered$" break="on-true">
                    <action application="hangup" data="GATEWAY_DOWN"/>
                </condition>
                {{% if Recording %}}
                <condition field="${recording_path__}" expression="^$" break="never">
                    <action application="export" data="recording_path__=archive/${strftime(%Y%m%d)}/${uuid}.mp3"/>
                    <action application="set" data="media_bug_answer_req=true"/>
                    <action application="set" data="recording_follow_transfer=true"/>
                    <action application="set" data="enable_file_write_buffering=true"/>
                    <action application="set" data="RECORD_STEREO=true"/>
                    <action application="set" data="RECORD_USE_THREAD=true"/>
                    <action application="set" data="RECORD_APPEND=true"/>
                    <action application="set" data="execute_on_answer_rcd=record_session $${recordings_dir}/${recording_path__}"/>
                </condition>
                {{% endif %}}
                <condition field="destination_number" expression="^${destination_number}$" break="never">
                    <action application="set" data="continue_on_fail=USER_BUSY,NO_USER_RESPONSE"/>
                    <action application="set" data="call_url=${regex(${sofia_contact(default/{{ReverseAccount}})}|^(.+)sip:(.+)@(.+)|%1sip:${destination_number}@%3)}"/>
                    <action application="bridge" data="${call_url}"/>
                    <action application="sleep" data="2000"/>
                    <action application="bridge" data="${call_url}"/>
                </condition>

 */
}

REGIST_MEMBER_JSON(
    test::Account,
    PLAIN(ID),
    PLAIN(Domain),
    PLAIN(Password)
);

REGIST_MEMBER_JSON(
    test::DirectoryInDTO,
    PLAIN(EventCallingFunction),
    PLAIN(UserAgent),
    PLAIN(Ip),
    PLAIN(AuthName),
    PLAIN(Domain)
);

REGIST_MEMBER_JSON(
    test::DirectoryOutDTO,
    PLAIN(Account)
);

REGIST_MEMBER_JSON(
    test::SubObject,
    NAME(String, "string"),
    NAME(Int32, "int32")
);

//注册时要放到全局位置注册
REGIST_MEMBER_JSON(
    test::Object, 
    NAME(Sub,"sub"),
    NAME(SubPtr,"sub_ptr"),
    NAME(VectorSub,"vector_sub"),
    NAME(VectorSubPtr,"vector_sub_ptr"),
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
    NAME(VectorString,"vector_string"),
    NAME(VectorFloat,"vector_float"),
    NAME(VectorDouble,"vector_double"),
    NAME(VectorInt8,"vector_int8"),
    NAME(VectorInt16,"vector_int16"),
    NAME(VectorInt32,"vector_int32"),
    NAME(VectorInt64,"vector_int64"),
    NAME(VectorUInt8,"vector_uint8"),
    NAME(VectorUInt16,"vector_uint16"),
    NAME(VectorUInt32,"vector_uint32"),
    NAME(VectorUInt64,"vector_uint64"),
    NAME(ListString,"list_string"),
    NAME(ListFloat,"list_float"),
    NAME(ListDouble,"list_double"),
    NAME(ListInt8,"list_int8"),
    NAME(ListInt16,"list_int16"),
    NAME(ListInt32,"list_int32"),
    NAME(ListInt64,"list_int64"),
    NAME(ListUInt8,"list_uint8"),
    NAME(ListUInt16,"list_uint16"),
    NAME(ListUInt32,"list_uint32"),
    NAME(ListUInt64,"list_uint64"),
    NAME(SetString,"set_string"),
    NAME(SetFloat,"set_float"),
    NAME(SetDouble,"set_double"),
    NAME(SetInt8,"set_int8"),
    NAME(SetInt16,"set_int16"),
    NAME(SetInt32,"set_int32"),
    NAME(SetInt64,"set_int64"),
    NAME(SetUInt8,"set_uint8"),
    NAME(SetUInt16,"set_uint16"),
    NAME(SetUInt32,"set_uint32"),
    NAME(SetUInt64,"set_uint64")
);

REGIST_MEMBER_PARAM(
    test::Object,
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
    test::Object t;
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
    std::cout << "======================================================" << std::endl;

    auto t2 = serialize::JsonSerializer<test::Object>::FromStringPtr(json);
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
        auto directory_tpl = env.parse(test::XmlDirectory);
        auto sofia_tpl = env.parse(test::XmlSofia);
        auto extension_gateway_tpl = env.parse(test::XmlExtensionGatewayDialplan);
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

int main()
{
    lic::Acquire();
    //TestJsonSerialize();
    //TestParamSerialize();
    //std::cout << test::XmlDirectory << std::endl;
    //TestTemplateSerialize();
    return 0;
}

