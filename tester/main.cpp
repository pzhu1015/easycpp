#include <iostream>
#include "json_serialize.h"
#include "param_serialize.h"

namespace serialize
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
}

//注册时要放到全局位置注册
REGIST_MEMBER_JSON(
    Test, 
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
    Test,
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

void TestSerialize()
{
    serialize::Test t;
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

    auto json = serialize::JsonSerializer<serialize::Test>::ToString(t);
    std::cout << "json =  " << json << std::endl;

    auto params = serialize::ParamSerializer<serialize::Test>::ToString(t);
    std::cout << "params = " << params << std::endl;

    serialize::Test t2 = serialize::JsonSerializer<serialize::Test>::FromString(json);
    std::cout << "t2.String = " << t2.String << std::endl;

    serialize::Test t3 = serialize::ParamSerializer<serialize::Test>::FromString(params);
    std::cout << "t3.String = " << t3.String << std::endl;

    std::shared_ptr<serialize::Test> t4 = serialize::JsonSerializer<serialize::Test>::FromStringPtr(json);
    auto json4 = serialize::JsonSerializer<serialize::Test>::ToString(t4);
    std::cout << "json4 = " << json4 << std::endl;

    std::shared_ptr<serialize::Test> t5 = serialize::ParamSerializer<serialize::Test>::FromStringPtr(params);
    auto param5 = serialize::ParamSerializer<serialize::Test>::ToString(t5);
    std::cout << "param5 = " << param5 << std::endl;

    std::set<std::string> sets = {"a", "b", "c"};
    auto setstr = serialize::JsonSerializer<std::set<std::string>>::ToString(sets);
    std::cout << "sets = " << setstr << std::endl;

    auto sets1 = serialize::JsonSerializer<std::set<std::string>>::FromString(setstr);
    sets1.emplace("d");
    auto setstr1 = serialize::JsonSerializer<std::set<std::string>>::ToString(sets1);
    std::cout << "set1 = " << setstr1 << std::endl;
}

int main()
{
    TestSerialize();
    return 0;
}

