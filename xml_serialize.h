#pragma once
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <set>
#include <pugixml.hpp>
#include "reflection.h"

#ifdef EASYCPP_LOGGING
#include "logger.h"
#else
#define DEBUG(...)  ((void)0)
#define INFO(...) ((void)0)
#define WARNING(...) ((void)0)
#define ERROR(...) ((void)0)
#endif

#define XML_SERIALIZE_PTR(name, nick) ::serialize::XmlSerialize::Serialize(entity->name, nick, xml);
#define XML_DESERIALIZE_PTR(name, nick) ::serialize::XmlSerialize::DeSerialize(entity->name, nick, xml); 

#define XML_SERIALIZE(name, nick) ::serialize::XmlSerialize::Serialize(entity.name, nick, xml);
#define XML_DESERIALIZE(name, nick) ::serialize::XmlSerialize::DeSerialize(entity.name, nick, xml); 

#define XML_SERIALIZE_ITEM(x) XML_SERIALIZE(EXTRACT_MEMBER(x), EXTRACT_NICK(x))
#define XML_DESERIALIZE_ITEM(x) XML_DESERIALIZE(EXTRACT_MEMBER(x), EXTRACT_NICK(x))

#define XML_SERIALIZE_ITEM_PTR(x) XML_SERIALIZE_PTR(EXTRACT_MEMBER(x), EXTRACT_NICK(x))
#define XML_DESERIALIZE_ITEM_PTR(x) XML_DESERIALIZE_PTR(EXTRACT_MEMBER(x), EXTRACT_NICK(x))


#define REGIST_CLASS_XML(T, ROOT) \
namespace serialize \
{\
    template<>\
	class XmlSerializer<T>\
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
        static ::serialize::Xml ToXml(const std::shared_ptr<T> &entity, const std::string &name = ROOT) \
		{\
            ::pugi::xml_document doc;\
            auto xml = doc.append_child(name.data());\
            xml.text().set(entity->ToString().data());\
            return xml;\
		}\
        static ::serialize::Xml ToXml(const T &entity, const std::string &name = ROOT) \
		{\
            ::pugi::xml_document doc;\
            auto xml = doc.append_child(name.data());\
            xml.text().set(entity.ToString().data());\
            return xml;\
		}\
		static std::shared_ptr<T> FromStringPtr(const std::string &str) \
		{\
		    return std::make_shared<T>(str);\
		}\
		static T FromString(const std::string &str) \
		{\
		    return T(str);\
		}\
		static std::shared_ptr<T> FromXmlPtr(const ::serialize::Xml &xml) \
		{\
		    return std::make_shared<T>(xml.text().as_string());\
		}\
		static T FromXml(const ::serialize::Xml &xml) \
		{\
		    return T(xml.text().as_string());\
		}\
	};\
}

#define REGIST_MEMBER_XML(T, ROOT, ...) \
namespace serialize \
{\
	template<>\
	class XmlSerializer<T>\
	{\
	public:\
		static std::string ToString(const std::shared_ptr<T> &entity, bool formatted = false) \
		{\
            ::pugi::xml_document doc;\
            auto xml = doc.append_child(ROOT);\
            FOREACH(XML_SERIALIZE_ITEM_PTR, __VA_ARGS__);\
            if (!formatted)\
            {\
                return xml_document_to_string_fast(doc);\
            }\
            return xml_document_to_string_formatted(doc);\
        }\
		static std::string ToString(const T &entity, bool formatted = false) \
		{\
            ::pugi::xml_document doc;\
            auto xml = doc.append_child(ROOT);\
            FOREACH(XML_SERIALIZE_ITEM, __VA_ARGS__);\
            if (!formatted)\
            {\
                return xml_document_to_string_fast(xml);\
            }\
            return xml_document_to_string_formatted(xml);\
		}\
        static ::serialize::Xml ToXml(const std::shared_ptr<T> &entity, Xml &xml) \
		{\
            FOREACH(XML_SERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return xml;\
		}\
        static ::serialize::Xml ToXml(const T &entity, Xml &xml) \
		{\
            FOREACH(XML_SERIALIZE_ITEM, __VA_ARGS__);\
            return xml;\
		}\
		static std::shared_ptr<T> FromStringPtr(const std::string &str) \
		{\
            auto entity = std::make_shared<T>();\
            ::pugi::xml_document doc;\
            doc.load_string(str.data());\
            auto xml = doc.document_element();\
            FOREACH(XML_DESERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return entity;\
		}\
		static T FromString(const std::string &str) \
		{\
            T entity;\
            ::pugi::xml_document doc;\
            doc.load_string(str.data());\
            auto xml = doc.document_element();\
            FOREACH(XML_DESERIALIZE_ITEM, __VA_ARGS__);\
            return entity;\
		}\
		static std::shared_ptr<T> FromXmlPtr(const ::serialize::Xml &xml) \
		{\
            auto entity = std::make_shared<T>();\
            FOREACH(XML_DESERIALIZE_ITEM_PTR, __VA_ARGS__);\
            return entity;\
		}\
		static T FromXml(const ::serialize::Xml &xml) \
        {\
            T entity;\
            FOREACH(XML_DESERIALIZE_ITEM, __VA_ARGS__);\
            return entity;\
		}\
        static std::string Root() \
        {\
            return ROOT;\
        }\
	};\
}

namespace serialize
{
using Xml = pugi::xml_node;

// 方法2：使用自定义 writer（更高性能）
inline std::string xml_document_to_string_fast(Xml &xml) 
{
    struct xml_string_writer : pugi::xml_writer 
    {
        std::string result;
        virtual void write(const void* data, size_t size) override 
        {
            result.append(static_cast<const char*>(data), size);
        }
    };
    xml_string_writer writer;
    xml.print(writer);
    return writer.result;
}

// 方法3：格式化控制（带缩进）
inline std::string xml_document_to_string_formatted(Xml &xml) 
{
    struct xml_string_writer : pugi::xml_writer 
    {
        std::string result;
        virtual void write(const void* data, size_t size) override 
        {
            result.append(static_cast<const char*>(data), size);
        }
    };
    xml_string_writer writer;
    xml.print(writer, "  ", pugi::format_indent); // 2空格缩进
    return writer.result;
}

template<class T>
class XmlSerializer
{
public:
    static std::string ToString(const std::shared_ptr<T> &entity, bool formatted = false) 
    {
        return "";
    }
	static std::string ToString(const T &entity, bool formatted = false) 
    {
        return std::string();
    }
    static Xml ToXml(const std::shared_ptr<T> &entity) 
    {
        return Xml();
    }
    static Xml ToXml(const T &entity) 
    {
        return Xml();
    }

    static std::shared_ptr<T> FromStringPtr(const std::string &str)
    {
        return nullptr;
    }

    static T FromString(const std::string &str)
    {
        return T();
    }

    static std::shared_ptr<T> FromXmlPtr(const Xml &xml) 
    {
        return nullptr;
    }

    static T FromXml(const Xml &xml) 
    {
        return T();
    }
};


class XmlSerialize
{
public:
    static void Serialize(const std::string &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        node.text().set(value.data());
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
    static Serialize(T value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        node.text().set(value);
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
    static Serialize(T value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        node.text().set(value);
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static Serialize(const std::shared_ptr<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        XmlSerializer<T>::ToXml(value, node);
    }

    //自定义类型
    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static Serialize(const T &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        XmlSerializer<T>::ToXml(value, node);
    }

    //枚举类型
    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type
    static Serialize(const T &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        node.text().set(value);
    }

    static void Serialize(const std::vector<std::string> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub_node = node.append_child(name.data());
            sub_node.text().set(v.data());
        }
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
    static Serialize(const std::vector<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub_node = node.append_child(name.data());
            sub_node.text().set(v);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
    static Serialize(const std::vector<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub_node = node.append_child(name.data());
            sub_node.text().set(v);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static Serialize(const std::vector<std::shared_ptr<T>> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub = node.append_child(XmlSerializer<T>::Root());
            XmlSerializer<T>::ToXml(v, sub);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static Serialize(const std::vector<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub = node.append_child(XmlSerializer<T>::Root());
            XmlSerializer<T>::ToXml(v, sub);
        }
    }

    static void Serialize(const std::list<std::string> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub_node = node.append_child(name.data());
            sub_node.text().set(v.data());
        }
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
    static Serialize(const std::list<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub_node = node.append_child(name.data());
            sub_node.text().set(v);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
    static Serialize(const std::list<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub_node = node.append_child(name.data());
            sub_node.text().set(v);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static Serialize(const std::list<std::shared_ptr<T>> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub = node.append_child(XmlSerializer<T>::Root());
            XmlSerializer<T>::ToXml(v, sub);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static Serialize(const std::list<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub = node.append_child(XmlSerializer<T>::Root());
            XmlSerializer<T>::ToXml(v, sub);
        }
    }

    static void Serialize(const std::set<std::string> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub_node = node.append_child(name.data());
            sub_node.text().set(v.data());
        }
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
    static Serialize(const std::set<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub_node = node.append_child(name.data());
            sub_node.text().set(v);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
    static Serialize(const std::set<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub_node = node.append_child(name.data());
            sub_node.text().set(v);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static Serialize(const std::set<std::shared_ptr<T>> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub = node.append_child(XmlSerializer<T>::Root());
            XmlSerializer<T>::ToXml(v, sub);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static Serialize(const std::set<T> &value, const std::string &name, Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) node = xml.append_child(name.data());
        for (const auto &v : value)
        {
            auto sub = node.append_child(XmlSerializer<T>::Root());
            XmlSerializer<T>::ToXml(v, sub);
        }
    }

    static void DeSerialize(std::string &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;

        value = node.text().as_string();
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
    static DeSerialize(T &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;

        if constexpr(std::is_same<T, bool>::value)
        {
            value = node.text().as_bool();
        }
        else if constexpr(std::is_signed<T>::value)
        {
            if constexpr(sizeof(T) <= sizeof(int))
            {
                value = static_cast<T>(node.text().as_int());
            }
            else
            {
                value = static_cast<T>(node.text().as_llong());
            }
        }
        else
        {
            if constexpr(sizeof(T) <= sizeof(unsigned int))
            {
                value = static_cast<T>(node.text().as_uint());
            }
            else
            {
                value = static_cast<T>(node.text().as_ullong());
            }
        }
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
    static DeSerialize(T &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        if constexpr(std::is_same<T, float>::value)
        {
            value = static_cast<float>(node.text().as_float());    
        }
        else
        {
            value = static_cast<float>(node.text().as_double());    
        }
    }

    //对象指针类型
    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static DeSerialize(std::shared_ptr<T> &value, const std::string &name, const Xml &xml) 
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        value = XmlSerializer<T>::FromXmlPtr(xml);
    }

    //对象类型
    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static DeSerialize(T &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        value = XmlSerializer<T>::FromXml(xml);
    }

    //枚举类型
    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type
    static DeSerialize(T &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        value = node.text().as_int();
    }

    static void DeSerialize(std::vector<std::string> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            value.emplace_back(sub.text().as_string());
        }
    }
    
    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
    static DeSerialize(std::vector<T> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            if constexpr(std::is_same<T, bool>::value)
            {
                value.emplace_back(sub.text().as_bool());
            }
            else if constexpr(std::is_signed<T>::value)
            {
                if constexpr(sizeof(T) <= sizeof(int))
                {
                    value.emplace_back(static_cast<T>(sub.text().as_int()));
                }
                else
                {
                    value.emplace_back(static_cast<T>(sub.text().as_llong()));
                }
            }
            else
            {
                if constexpr(sizeof(T) <= sizeof(unsigned int))
                {
                    value.emplace_back(static_cast<T>(sub.text().as_uint()));
                }
                else
                {
                    value.emplace_back(static_cast<T>(sub.text().as_ullong()));
                }
            }
        }
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
    static DeSerialize(std::vector<T> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            if constexpr(std::is_same<T, float>::value)
            {
                value.emplace_back(sub.text().as_float());
            }
            else
            {
                value.emplace_back(sub.text().as_double());
            }
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static DeSerialize(std::vector<std::shared_ptr<T>> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            value.emplace_back(XmlSerializer<T>::FromXmlPtr(sub));
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static DeSerialize(std::vector<T> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            value.emplace_back(XmlSerializer<T>::FromXml(sub));
        }
    }

    static void DeSerialize(std::list<std::string> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            value.emplace_back(sub.text().as_string());
        }
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
    static DeSerialize(std::list<T> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            if constexpr(std::is_same<T, bool>::value)
            {
                value.emplace_back(sub.text().as_bool());
            }
            else if constexpr(std::is_signed<T>::value)
            {
                if constexpr(sizeof(T) <= sizeof(int))
                {
                    value.emplace_back(static_cast<T>(sub.text().as_int()));
                }
                else
                {
                    value.emplace_back(static_cast<T>(sub.text().as_llong()));
                }
            }
            else
            {
                if constexpr(sizeof(T) <= sizeof(unsigned int))
                {
                    value.emplace_back(static_cast<T>(sub.text().as_uint()));
                }
                else
                {
                    value.emplace_back(static_cast<T>(sub.text().as_ullong()));
                }
            }
        }
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
    static DeSerialize(std::list<T> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            if constexpr(std::is_same<T, float>::value)
            {
                value.emplace_back(sub.text().as_float());
            }
            else
            {
                value.emplace_back(sub.text().as_double());
            }
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static DeSerialize(std::list<std::shared_ptr<T>> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            value.emplace_back(XmlSerializer<T>::FromXmlPtr(sub));
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static DeSerialize(std::list<T> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            value.emplace_back(XmlSerializer<T>::FromXml(sub));
        }
    }

    static void DeSerialize(std::set<std::string> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            value.emplace(sub.text().as_string());
        }
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
    static DeSerialize(std::set<T> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            if constexpr(std::is_same<T, bool>::value)
            {
                value.emplace(sub.text().as_bool());
            }
            else if constexpr(std::is_signed<T>::value)
            {
                if constexpr(sizeof(T) <= sizeof(int))
                {
                    value.emplace(static_cast<T>(sub.text().as_int()));
                }
                else
                {
                    value.emplace(static_cast<T>(sub.text().as_llong()));
                }
            }
            else
            {
                if constexpr(sizeof(T) <= sizeof(unsigned int))
                {
                    value.emplace(static_cast<T>(sub.text().as_uint()));
                }
                else
                {
                    value.emplace(static_cast<T>(sub.text().as_ullong()));
                }
            }
        }
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
    static DeSerialize(std::set<T> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            if constexpr(std::is_same<T, float>::value)
            {
                value.emplace(sub.text().as_float());
            }
            else
            {
                value.emplace(sub.text().as_double());
            }
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static DeSerialize(std::set<std::shared_ptr<T>> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            value.emplace(XmlSerializer<T>::FromXmlPtr(sub));
        }
    }

    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    static DeSerialize(std::set<T> &value, const std::string &name, const Xml &xml)
    {
        INFO("[%s][node: %s][name: %s][value: %s]", __func__, xml.name(), name.data(), type_name<decltype(value)>().data());
        auto node = xml.child(name.data());
        if (!node) return;
        for (auto sub = node.first_child(); sub; sub = sub.next_sibling())
        {
            value.emplace(XmlSerializer<T>::FromXml(sub));
        }
    }
};
}
