#pragma once
#include <vector>
#include <list>
#include <set>
#include <memory>
#include "datetime.h"

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
    datetime::DateTime DateTime;

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
}
