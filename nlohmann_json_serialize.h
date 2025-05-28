#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <set>
#include <nlohmann/json.hpp>
#include "reflection.h"

#define JSON_SERIALIZE_PTR(name, nick) ::serialize::JsonSerialize::Serialize(entity->name, nick, json);
#define JSON_DESERIALIZE_PTR(name, nick) ::serialize::JsonSerialize::DeSerialize(entity->name, nick, json); 

#define JSON_SERIALIZE(name, nick) ::serialize::JsonSerialize::Serialize(entity.name, nick, json);
#define JSON_DESERIALIZE(name, nick) ::serialize::JsonSerialize::DeSerialize(entity.name, nick, json); 

#define JSON_SERIALIZE_ITEM(x) JSON_SERIALIZE(EXTRACT_MEMBER(x), EXTRACT_NICK(x))
#define JSON_DESERIALIZE_ITEM(x) JSON_DESERIALIZE(EXTRACT_MEMBER(x), EXTRACT_NICK(x))

#define JSON_SERIALIZE_ITEM_PTR(x) JSON_SERIALIZE_PTR(EXTRACT_MEMBER(x), EXTRACT_NICK(x))
#define JSON_DESERIALIZE_ITEM_PTR(x) JSON_DESERIALIZE_PTR(EXTRACT_MEMBER(x), EXTRACT_NICK(x))

#define REGIST_MEMBER_JSON(T, ...) \
namespace serialize \
{\
	template<>\
	class JsonSerializer<T>\
	{\
	public:\
		static std::string ToString(const std::shared_ptr<T> &entity) \
		{\
            auto json = nlohmann::json::object();\
            FOREACH(JSON_SERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return json.dump(4);\
        }\
		static std::string ToString(const T &entity) \
		{\
            auto json = nlohmann::json::object();\
            FOREACH(JSON_SERIALIZE_ITEM, __VA_ARGS__);\
            return json.dump(4);\
		}\
        static ::serialize::Json ToJson(const std::shared_ptr<T> &entity) \
		{\
            auto json = nlohmann::json::object();\
            FOREACH(JSON_SERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return json;\
		}\
        static ::serialize::Json ToJson(const T &entity) \
		{\
            auto json = nlohmann::json::object();\
            FOREACH(JSON_SERIALIZE_ITEM, __VA_ARGS__);\
            return json;\
		}\
		static std::shared_ptr<T> FromStringPtr(const std::string &str) \
		{\
            auto entity = std::make_shared<T>();\
            auto json = nlohmann::json::parse(str);\
            FOREACH(JSON_DESERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return entity;\
		}\
		static T FromString(const std::string &str) \
		{\
            T entity;\
            auto json = nlohmann::json::parse(str);\
            FOREACH(JSON_DESERIALIZE_ITEM, __VA_ARGS__);\
            return entity;\
		}\
		static std::shared_ptr<T> FromJsonPtr(const ::serialize::Json &json) \
		{\
            auto entity = std::make_shared<T>();\
            FOREACH(JSON_DESERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return entity;\
		}\
		static T FromJson(const ::serialize::Json &json) \
        {\
            T entity;\
            FOREACH(JSON_DESERIALIZE_ITEM, __VA_ARGS__);\
            return entity;\
		}\
	};\
}

namespace serialize
{
using Json = nlohmann::json;
template<class T>
class JsonSerializer
{
public:
    static std::string ToString(const std::shared_ptr<T> &entity) 
    {
        return "";
    }
	static std::string ToString(const T &entity) 
    {
        return std::string();
    }
    static Json ToJson(const std::shared_ptr<T> &entity) 
    {
        return Json();
    }
    static Json ToJson(const T &entity) 
    {
        return Json();
    }

    static std::shared_ptr<T> FromStringPtr(const std::string &str)
    {
        return nullptr;
    }

    static T FromString(const std::string &str)
    {
        return T();
    }

    static std::shared_ptr<T> FromJsonPtr(const Json &json) 
    {
        return nullptr;
    }

    static T FromJson(const Json &json) 
    {
        return T();
    }
};

template<>
class JsonSerializer<std::set<std::string>>
{
public:
    static std::string ToString(const std::set<std::string> &entity)
    {
        EASYCPP_LOG;
        Json json = entity;
        return json.dump(4);
    }

    static Json ToJson(const std::set<std::string> &entity) 
    {
        EASYCPP_LOG;
        return entity;
    }

    static std::set<std::string> FromString(const std::string &str)
    {
        EASYCPP_LOG;
        auto json = Json::parse(str);
        if (!json.is_null() && json.is_array())
        {
            return json.get<std::set<std::string>>();
        }
        return {};
    }

    static std::set<std::string> FromJson(const Json &json)
    {
        EASYCPP_LOG;
        if (!json.is_null() && json.is_array())
        {
            return json.get<std::set<std::string>>();
        }
        return {};
    }
};

template<>
class JsonSerializer<std::vector<std::string>>
{
public:
    static std::string ToString(const std::vector<std::string> &entity)
    {
        EASYCPP_LOG;
        Json json = entity;
        return json.dump(4);
    }

    static Json ToJson(const std::vector<std::string> &entity) 
    {
        EASYCPP_LOG;
        return entity;
    }

    static std::vector<std::string> FromString(const std::string &str)
    {
        auto json = Json::parse(str);
        if (!json.is_null() && json.is_array())
        {
            return json.get<std::vector<std::string>>();
        }
        return {};
    }

    static std::vector<std::string> FromJson(const Json &json)
    {
        EASYCPP_LOG;
        if (!json.is_null() && json.is_array())
        {
            return json.get<std::vector<std::string>>();
        }
        return {};
    }
};

template<class T>
class JsonSerializer<std::vector<std::shared_ptr<T>>>
{
public:
    static std::string ToString(const std::vector<std::shared_ptr<T>> &entity)
    {
        EASYCPP_LOG;
        auto array = Json::array();
        for (const auto &v : entity)
        {
            array.emplace_back(JsonSerializer<T>::ToJson(v));
        }
        return array.dump(4);
    }

    static Json ToJson(const std::vector<std::shared_ptr<T>> &entity) 
    {
        EASYCPP_LOG;
        auto array = Json::array();
        for (const auto &v : entity)
        {
            array.emplace_back(JsonSerializer<T>::ToJson(v));
        }
        return array;
    }

    static std::vector<std::shared_ptr<T>> FromString(const std::string &str)
    {
        EASYCPP_LOG;
        std::vector<std::shared_ptr<T>> value;
        auto json = Json::parse(str);
        if (!json.is_null() && json.is_array())
        {
            for (const auto &item : json)
            {
                value.emplace_back(JsonSerializer<T>::FromJsonPtr(item));
            }
        }
        return value;
    }

    static std::vector<std::shared_ptr<T>> FromJson(const Json &json)
    {
        EASYCPP_LOG;
        std::vector<std::shared_ptr<T>> value;
        if (!json.is_null() && json.is_array())
        {
            for (const auto &item : json)
            {
                value.emplace_back(JsonSerializer<T>::FromJsonPtr(item));
            }
            return value;
        }
        return value;
    }
};


template<>
class JsonSerializer<std::list<std::string>>
{
public:
    static std::string ToString(const std::list<std::string> &entity)
    {
        EASYCPP_LOG;
        Json json = entity;
        return json.dump(4);
    }

    static Json ToJson(const std::list<std::string> &entity) 
    {
        EASYCPP_LOG;
        return entity;
    }

    static std::list<std::string> FromString(const std::string &str)
    {
        auto json = Json::parse(str);
        if (!json.is_null() && json.is_array())
        {
            return json.get<std::list<std::string>>();
        }
        return {};
    }

    static std::list<std::string> FromJson(const Json &json)
    {
        EASYCPP_LOG;
        if (!json.is_null() && json.is_array())
        {
            return json.get<std::list<std::string>>();
        }
        return {};
    }
};

class JsonSerialize
{
public:
    static void Serialize(const std::string &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(bool value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }
    static void Serialize(float value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(double value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(int8_t value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(int16_t value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(int32_t value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(int64_t value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(uint8_t value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(uint16_t value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(uint32_t value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(uint64_t value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    template<typename T>
    static void Serialize(const std::shared_ptr<T> &value, const std::string &name, Json &json)
    {
        if (value != nullptr && !json.is_null() && json.is_object())
        {
            json[name] = JsonSerializer<T>::ToJson(value);
        }
    }

    //非枚举类型
    template<typename T>
    typename std::enable_if<!std::is_enum<T>::value, void>::type
    static Serialize(const T &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = JsonSerializer<T>::ToJson(value);
        }
    }

    //枚举类型
    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type
    static Serialize(const T &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<std::string> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<float> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<double> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<int8_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<int16_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<int32_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<int64_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<uint8_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<uint16_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<uint32_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::vector<uint64_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    template<typename T>
    static void Serialize(const std::vector<std::shared_ptr<T>> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            auto array = nlohmann::json::array();
            for (const auto &v : value)
            {
                array.emplace_back(JsonSerializer<T>::ToJson(v));
            }
            json[name] = array;
        }
    }

    template<typename T>
    static void Serialize(const std::vector<T> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            auto array = nlohmann::json::array();
            for (const auto &v : value)
            {
                array.emplace_back(JsonSerializer<T>::ToJson(v));
            }
            json[name] = array;
        }
    }

    static void Serialize(const std::list<std::string> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<float> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<double> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<int8_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<int16_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<int32_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<int64_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<uint8_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<uint16_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<uint32_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::list<uint64_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }


    template<typename T>
    static void Serialize(const std::list<std::shared_ptr<T>> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            auto array = nlohmann::json::array();
            for (const auto &v : value)
            {
                array.emplace_back(JsonSerializer<T>::ToJson(v));
            }
            json[name] = array;
        }
    }

    template<typename T>
    static void Serialize(const std::list<T> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            auto array = nlohmann::json::array();
            for (const auto &v : value)
            {
                array.emplace_back(JsonSerializer<T>::ToJson(v));
            }
            json[name] = array;
        }
    }

    static void Serialize(const std::set<std::string> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<float> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<double> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<int8_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<int16_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<int32_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<int64_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<uint8_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<uint16_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<uint32_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    static void Serialize(const std::set<uint64_t> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            json[name] = value;
        }
    }

    template<typename T>
    static void Serialize(const std::set<std::shared_ptr<T>> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            auto array = nlohmann::json::array();
            for (const auto &v : value)
            {
                array.emplace_back(JsonSerializer<T>::ToJson(v));
            }
            json[name] = array;
        }
    }

    template<typename T>
    static void Serialize(const std::set<T> &value, const std::string &name, Json &json)
    {
        if (!json.is_null() && json.is_object())
        {
            auto array = nlohmann::json::array();
            for (const auto &v : value)
            {
                array.emplace_back(JsonSerializer<T>::ToJson(v));
            }
            json[name] = array;
        }
    }

    static void DeSerialize(std::string &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_string())
        {
            value = v.get<std::string>();
        }
    }

    static void DeSerialize(bool &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_boolean())
        {
            value = v.get<bool>();
        }
    }

    static void DeSerialize(float &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<float>();
        }
    }

    static void DeSerialize(double &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<double>();
        }
    }

    static void DeSerialize(int8_t &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<int8_t>();
        }
    }

    static void DeSerialize(int16_t &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<int16_t>();
        }
    }

    static void DeSerialize(int32_t &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<int32_t>();
        }
    }

    static void DeSerialize(int64_t &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<int64_t>();
        }
    }

    static void DeSerialize(uint8_t &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<uint8_t>();
        }
    }

    static void DeSerialize(uint16_t &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<uint16_t>();
        }
    }

    static void DeSerialize(uint32_t &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<uint32_t>();
        }
    }

    static void DeSerialize(uint64_t &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = v.get<uint64_t>();
        }
    }

    //对象指针类型
    template<typename T>
    static void DeSerialize(std::shared_ptr<T> &value, const std::string &name, const Json &json) 
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_object())
        {
            value = JsonSerializer<T>::FromJsonPtr(v);
        }
    }

    //对象类型
    template<typename T>
    typename std::enable_if<!std::is_enum<T>::value, void>::type
    static DeSerialize(T &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_object())
        {
            value = JsonSerializer<T>::FromJson(v);
        }
    }

    //枚举类型
    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type
    static DeSerialize(T &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_number())
        {
            value = json[name].get<T>();
        }
    }

    static void DeSerialize(std::vector<std::string> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<std::string>>();
        }
    }
    
    static void DeSerialize(std::vector<float> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<float>>();
        }
    }

    static void DeSerialize(std::vector<double> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<double>>();
        }
    }

    static void DeSerialize(std::vector<int8_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<int8_t>>();
        }
    }

    static void DeSerialize(std::vector<int16_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<int16_t>>();
        }
    }

    static void DeSerialize(std::vector<int32_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<int32_t>>();
        }
    }

    static void DeSerialize(std::vector<int64_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<int64_t>>();
        }
    }

    static void DeSerialize(std::vector<uint8_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<uint8_t>>();
        }
    }

    static void DeSerialize(std::vector<uint16_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<uint16_t>>();
        }
    }

    static void DeSerialize(std::vector<uint32_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<uint32_t>>();
        }
    }

    static void DeSerialize(std::vector<uint64_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::vector<uint64_t>>();
        }
    }

    template<typename T>
    static void DeSerialize(std::vector<std::shared_ptr<T>> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            for (const auto &item : v)
            {
                value.emplace_back(JsonSerializer<T>::FromJsonPtr(item));
            }
        }
    }

    template<typename T>
    static void DeSerialize(std::vector<T> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            for (const auto &item : v)
            {
                value.emplace_back(JsonSerializer<T>::FromJson(item));
            }
        }
    }

    static void DeSerialize(std::list<std::string> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<std::string>>();
        }
    }

    static void DeSerialize(std::list<float> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<float>>();
        }
    }

    static void DeSerialize(std::list<double> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<double>>();
        }
    }

    static void DeSerialize(std::list<int8_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<int8_t>>();
        }
    }

    static void DeSerialize(std::list<int16_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<int16_t>>();
        }
    }

    static void DeSerialize(std::list<int32_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<int32_t>>();
        }
    }

    static void DeSerialize(std::list<int64_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<int64_t>>();
        }
    }

    static void DeSerialize(std::list<uint8_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<uint8_t>>();
        }
    }

    static void DeSerialize(std::list<uint16_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<uint16_t>>();
        }
    }

    static void DeSerialize(std::list<uint32_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<uint32_t>>();
        }
    }

    static void DeSerialize(std::list<uint64_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::list<uint64_t>>();
        }
    }

    template<typename T>
    static void DeSerialize(std::list<std::shared_ptr<T>> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            for (const auto &item : v)
            {
                value.emplace_back(JsonSerializer<T>::FromJsonPtr(item));
            }
        }
    }

    template<typename T>
    static void DeSerialize(std::list<T> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            for (const auto &item : v)
            {
                value.emplace_back(JsonSerializer<T>::FromJson(item));
            }
        }
    }

    static void DeSerialize(std::set<std::string> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<std::string>>();
        }
    }

    static void DeSerialize(std::set<float> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<float>>();
        }
    }

    static void DeSerialize(std::set<double> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<double>>();
        }
    }

    static void DeSerialize(std::set<int8_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<int8_t>>();
        }
    }

    static void DeSerialize(std::set<int16_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<int16_t>>();
        }
    }

    static void DeSerialize(std::set<int32_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<int32_t>>();
        }
    }

    static void DeSerialize(std::set<int64_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<int64_t>>();
        }
    }

    static void DeSerialize(std::set<uint8_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<uint8_t>>();
        }
    }

    static void DeSerialize(std::set<uint16_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<uint16_t>>();
        }
    }

    static void DeSerialize(std::set<uint32_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<uint32_t>>();
        }
    }

    static void DeSerialize(std::set<uint64_t> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            value = v.get<std::set<uint64_t>>();
        }
    }

    template<typename T>
    static void DeSerialize(std::set<std::shared_ptr<T>> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            for (const auto &item : v)
            {
                value.emplace_back(JsonSerializer<T>::FromJsonPtr(item));
            }
        }
    }

    template<typename T>
    static void DeSerialize(std::set<T> &value, const std::string &name, const Json &json)
    {
        const Json &v = json[name];
        if (!v.is_null() && v.is_array())
        {
            for (const auto &item : v)
            {
                value.emplace_back(JsonSerializer<T>::FromJson(item));
            }
        }
    }
};
}
