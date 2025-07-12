#pragma once

#include <set>
#include <vector>
#include <iostream>
#include <json_serialize.h>
#include <odb/mysql/traits.hxx>

namespace odb 
{
namespace mysql
{
    template <>
    class value_traits<std::set<std::string>, id_string> {
    public:
        using value_type = std::set<std::string>;
        using query_type = value_type;
        using image_type = details::buffer;

        static void set_value(value_type& v, const image_type &i, size_t n, bool is_null) 
        {
            if (is_null)
            {
                v = value_type();
                return;
            }
            v = serialize::JsonSerializer<value_type>::FromString(std::string(i.data(), n));
        }
        
        static void set_image(image_type &i, size_t &n, bool& is_null, const value_type& v) 
        {
            is_null = false;
            auto json = serialize::JsonSerializer<value_type>::ToString(v);
            n = json.size();
            if (i.capacity() < n) i.capacity(n);
            std::memcpy(i.data(), json.c_str(), n);
        }
    };

    template<class T>
    class value_traits<std::vector<T>, id_string> {
    public:
        using value_type = std::vector<T>;
        using query_type = value_type;
        using image_type = details::buffer;

        static void set_value(value_type& v, const image_type &i, size_t n, bool is_null) 
        {
            if (is_null)
            {
                v = value_type();
                return;
            }
            auto json = std::string(i.data(), n);
            INFO(json.data());
            v = serialize::JsonSerializer<value_type>::FromString(json);
        }
        
        static void set_image(image_type &i, size_t &n, bool& is_null, const value_type& v) 
        {
            is_null = false;
            auto json = serialize::JsonSerializer<value_type>::ToString(v);
            n = json.size();
            if (i.capacity() < n) i.capacity(n);
            std::memcpy(i.data(), json.c_str(), n);
        }
    };
}
}
