#pragma once
#include <string>
#include <memory>
#include <iostream>
#include <httplib.h>
#include "reflection.h"

#ifdef EASYCPP_LOGGING
#include "logger.h"
#else
#define DEBUG(...)  ((void)0)
#define INFO(...) ((void)0)
#define WARNING(...) ((void)0)
#define ERROR(...) ((void)0)
#endif

#define PARAM_SERIALIZE_PTR(name, nick) ::serialize::ParamSerialize::Serialize(entity->name, nick, param);
#define PARAM_DESERIALIZE_PTR(name, nick) ::serialize::ParamSerialize::DeSerialize(entity->name, nick, param); 

#define PARAM_SERIALIZE(name, nick) ::serialize::ParamSerialize::Serialize(entity.name, nick, param);
#define PARAM_DESERIALIZE(name, nick) ::serialize::ParamSerialize::DeSerialize(entity.name, nick, param); 

#define PARAM_SERIALIZE_ITEM(x) PARAM_SERIALIZE(EXTRACT_MEMBER(x), EXTRACT_NICK(x))
#define PARAM_DESERIALIZE_ITEM(x) PARAM_DESERIALIZE(EXTRACT_MEMBER(x), EXTRACT_NICK(x))

#define PARAM_SERIALIZE_ITEM_PTR(x) PARAM_SERIALIZE_PTR(EXTRACT_MEMBER(x), EXTRACT_NICK(x))
#define PARAM_DESERIALIZE_ITEM_PTR(x) PARAM_DESERIALIZE_PTR(EXTRACT_MEMBER(x), EXTRACT_NICK(x))

#define REGIST_CLASS_PARAM(T) \
namespace serialize \
{\
    template<>\
	class ParamSerializer<T>\
	{\
	public:\
		static std::string ToString(const std::shared_ptr<T> &entity) \
		{\
            return entity->ToString();\
        }\
		static std::string ToString(const T &entity) \
		{\
            return entity.ToString();\
		}\
        static ::serialize::Params ToParam(const std::shared_ptr<T> &entity) \
		{\
            auto str = entity->ToString();\
            ::serialize::Params param;\
            ::httplib::detail::parse_query_text(str, param);\
            return param;\
		}\
        static ::serialize::Params ToParam(const T &entity) \
		{\
            auto str = entity.ToString();\
            ::serialize::Params param;\
            ::httplib::detail::parse_query_text(str, param);\
            return param;\
		}\
		static std::shared_ptr<T> FromStringPtr(const std::string &str) \
		{\
		    return std::make_shared<T>(str);\
		}\
		static T FromString(const std::string &str) \
		{\
		    return T(str);\
		}\
		static std::shared_ptr<T> FromParamPtr(const ::serialize::Params &param) \
		{\
		    return std::make_shared<T>(::httplib::detail::params_to_query_str(param));\
		}\
		static T FromParam(const ::serialize::Params &param) \
		{\
		    return T(::httplib::detail::params_to_query_str(param));\
		}\
	};\
}

#define REGIST_MEMBER_PARAM(T, ...) \
namespace serialize \
{\
	template<>\
	class ParamSerializer<T>\
	{\
	public:\
		static std::string ToString(const std::shared_ptr<T> &entity) \
		{\
            ::serialize::Params param;\
            FOREACH(PARAM_SERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return ::httplib::detail::params_to_query_str(param);\
        }\
		static std::string ToString(const T &entity) \
		{\
            ::serialize::Params param;\
            FOREACH(PARAM_SERIALIZE_ITEM, __VA_ARGS__);\
            return ::httplib::detail::params_to_query_str(param);\
		}\
        static ::serialize::Params ToParam(const std::shared_ptr<T> &entity) \
		{\
            ::serialize::Params param;\
            FOREACH(PARAM_SERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return param;\
		}\
        static ::serialize::Params ToParam(const T &entity) \
		{\
            ::serialize::Params param;\
            FOREACH(PARAM_SERIALIZE_ITEM, __VA_ARGS__);\
            return param;\
		}\
		static std::shared_ptr<T> FromStringPtr(const std::string &str) \
		{\
            auto entity = std::make_shared<T>();\
            ::serialize::Params param;\
            ::httplib::detail::parse_query_text(str, param);\
            FOREACH(PARAM_DESERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return entity;\
		}\
		static T FromString(const std::string &str) \
		{\
            T entity;\
            ::serialize::Params param; \
            ::httplib::detail::parse_query_text(str, param);\
            FOREACH(PARAM_DESERIALIZE_ITEM, __VA_ARGS__);\
            return entity;\
		}\
		static std::shared_ptr<T> FromParamPtr(const ::serialize::Params &param) \
		{\
            auto entity = std::make_shared<T>();\
            FOREACH(PARAM_DESERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return entity;\
		}\
		static T FromParam(const ::serialize::Params &param) \
        {\
            T entity;\
            FOREACH(PARAM_DESERIALIZE_ITEM, __VA_ARGS__);\
            return entity;\
		}\
	};\
}

namespace serialize
{
typedef httplib::Params Params;

template<class T>
class ParamSerializer
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
    static Params ToParam(const std::shared_ptr<T> &entity) 
    {
        return Params();
    }
    static Params ToParam(const T &entity) 
    {
        return Params();
    }

    static std::shared_ptr<T> FromStringPtr(const std::string &str) 
    {
        return nullptr;
    }

    static T FromString(const std::string &str) 
    {
        return T();
    }

    static std::shared_ptr<T> FromParamPtr(const Params &param) 
    {
        return nullptr;
    }

    static T FromParam(const Params &param) 
    {
        return T();
    }
};

class ParamSerialize
{
public:
    static void Serialize(const std::string &value, const std::string &name, Params &param)
    {
        param.insert({name, value});
    }

    static void Serialize(bool value, const std::string &name, Params &param)
    {

        param.insert({name, std::to_string(value)});
    }

    static void Serialize(float value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void Serialize(double value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void Serialize(int8_t value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void Serialize(int16_t value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void Serialize(int32_t value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void Serialize(int64_t value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void Serialize(uint8_t value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void Serialize(uint16_t value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void Serialize(uint32_t value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void Serialize(uint64_t value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }


    //枚举类型
    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type
    static Serialize(const T &value, const std::string &name, Params &param)
    {
        param.insert({name, std::to_string(value)});
    }

    static void DeSerialize(std::string &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = itr->second;
    }

    static void DeSerialize(bool &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        auto string_to_bool = [](const std::string& str) -> bool {
            auto trim = [](std::string s) -> std::string {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                    return !std::isspace(ch);
                }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                    return !std::isspace(ch);
                }).base(), s.end());
                return s;
            };

            std::string s = trim(str);
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);

            if (s == "true" || s == "1" || s == "yes") return true;
            if (s == "false" || s == "0" || s == "no") return false;
            return false;
        };
        value = string_to_bool(itr->second);
    }

    static void DeSerialize(float &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stof(itr->second);
    }

    static void DeSerialize(double &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stold(itr->second);
    }

    static void DeSerialize(int8_t &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stoll(itr->second);
    }

    static void DeSerialize(int16_t &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stoll(itr->second);
    }

    static void DeSerialize(int32_t &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stoll(itr->second);
    }

    static void DeSerialize(int64_t &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stoll(itr->second);
    }

    static void DeSerialize(uint8_t &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stoull(itr->second);
    }

    static void DeSerialize(uint16_t &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stoull(itr->second);
    }

    static void DeSerialize(uint32_t &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stoull(itr->second);
    }

    static void DeSerialize(uint64_t &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = std::stoull(itr->second);
    }

    //枚举类型
    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type
    static DeSerialize(T &value, const std::string &name, const Params &param)
    {
        auto itr = param.find(name);
        if (itr == param.end()) return;
        value = static_cast<T>(std::stoll(itr->second));
    }
};
}
