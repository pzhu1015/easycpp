#pragma once

#include <odb/pre.hxx>
#include <datetime.h>
#include <odb/mysql/traits.hxx>

namespace odb 
{
namespace mysql
{
    template <>
    class value_traits<::datetime::DateTime, id_datetime> {
    public:
        typedef ::datetime::DateTime value_type;
        typedef value_type query_type;
        typedef MYSQL_TIME image_type;

        static void set_value(value_type& v, const image_type &i, size_t n, bool is_null) 
        {
            if (is_null)
            {
                v = value_type();
                return;
            }
            std::tm tm = {};
            tm.tm_year = i.year - 1900;
            tm.tm_mon = i.month - 1;
            tm.tm_mday = i.day;
            tm.tm_hour = i.hour;
            tm.tm_min = i.minute;
            tm.tm_sec = i.second;
            std::time_t t = std::mktime(&tm);
            v = datetime::DateTime(t);
        }
        
        static void set_image(image_type &i, bool& is_null, const value_type& v) 
        {
            if (v.IsZero())
            {
                is_null = true;
            }
            else
            {
                is_null = false;
                std::time_t t = v.UnixTime();
                std::tm tm = *std::localtime(&t);
                i.year = tm.tm_year + 1900;
                i.month = tm.tm_mon + 1;
                i.day = tm.tm_mday;
                i.hour = tm.tm_hour;
                i.minute = tm.tm_min;
                i.second = tm.tm_sec;
                i.second_part = 0;
                i.neg = 0;
            }
        }
    };
}
}
#include <odb/post.hxx>
