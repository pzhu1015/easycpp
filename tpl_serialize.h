#pragma once
#include <ctemplate/template.h> //引用第三方库: https://github.com/OlafvdSpek/ctemplate
#include <vector>
#include <list>
#include <set>
#include "reflection.h"

#define TPL_SERIALIZE_PTR(name, nick) ::tpl::TplSerialize::Serialize(entity->name, nick, dict);

#define TPL_SERIALIZE(name, nick) ::tpl::TplSerialize::Serialize(entity.name, nick, dict);

#define TPL_SERIALIZE_ITEM(x) TPL_SERIALIZE(EXTRACT_MEMBER(x), EXTRACT_NICK(x))

#define TPL_SERIALIZE_ITEM_PTR(x) TPL_SERIALIZE_PTR(EXTRACT_MEMBER(x), EXTRACT_NICK(x))

#define REGIST_MEMBER_TPL(T, ...) \
namespace tpl\
{\
	template<>\
	class TplSerializer<T>\
	{\
	public:\
        static void ToDictionary(const std::shared_ptr<T> &entity, TplDictionary *dict)\
        {\
            FOREACH(TPL_SERIALIZE_ITEM_PTR, __VA_ARGS__);\
        }\
	};\
}

namespace tpl
{
typedef ctemplate::TemplateDictionary TplDictionary;
template<class T>
class TplSerializer
{
public:
    static void ToDictionary(const std::shared_ptr<T> &value, TplDictionary *dict)
    {
    }
};

class TplSerialize
{
public:
    template<class T>
    static void Serialize(const std::shared_ptr<T> &value, const std::string &name, TplDictionary *dict)
    {
        auto sub = dict->AddSectionDictionary(name);
        tpl::TplSerializer<T>::ToDictionary(value, sub);
    }

    template<typename T>
    static void Serialize(const std::vector<std::shared_ptr<T>> &value, const std::string &name, TplDictionary *dict)
    {
        for (const auto &v: value)
        {
            auto sub = dict->AddSectionDictionary(name);
            tpl::TplSerializer<T>::ToDictionary(value, sub);
        }
    }

    template<typename T>
    static void Serialize(const std::list<std::shared_ptr<T>> &value, const std::string &name, TplDictionary *dict)
    {
        for (const auto &v: value)
        {
            auto sub = dict->AddSectionDictionary(name);
            tpl::TplSerializer<T>::ToDictionary(value, sub);
        }
    }

    template<typename T>
    static void Serialize(const std::set<std::shared_ptr<T>> &value, const std::string &name, TplDictionary *dict)
    {
        for (const auto &v: value)
        {
            auto sub = dict->AddSectionDictionary(name);
            tpl::TplSerializer<T>::ToDictionary(value, sub);
        }
    }


    static void Serialize(const std::string &value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
    static void Serialize(bool value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
    static void Serialize(int8_t value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
    static void Serialize(int16_t value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
    static void Serialize(int32_t value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
    static void Serialize(int64_t value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
    static void Serialize(uint8_t value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
    static void Serialize(uint16_t value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
    static void Serialize(uint32_t value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
    static void Serialize(uint64_t value, const std::string &name, TplDictionary *dict)
    {
        (*dict)[name] = value;
    }
};
}
