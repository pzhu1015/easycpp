#pragma once
#include <odb/mysql/query.hxx>
#include <odb/mysql/traits.hxx>
#include <functional>
#include <vector>
#include <type_traits>
#include <utility>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

#ifdef EASYCPP_LOGGING
#include "logger.h"
#else
#define DEBUG(...)  ((void)0)
#define INFO(...) ((void)0)
#define WARNING(...) ((void)0)
#define ERROR(...) ((void)0)
#endif


namespace odb {
namespace mysql {

// 检查是否为查询列
template <typename T>
struct is_query_column : std::false_type {};

template <typename T, database_type_id ID>
struct is_query_column<odb::mysql::query_column<T, ID>> : std::true_type {};

// 排序方向包装类
template <typename Column>
struct order_expression {
    const Column& column;
    bool ascending;

    order_expression(const Column& col, bool asc) : column(col), ascending(asc) {}
};

template <typename Column>
order_expression<Column> asc(const Column& column) {
    return order_expression<Column>(column, true);
}

template <typename Column>
order_expression<Column> desc(const Column& column) {
    return order_expression<Column>(column, false);
}

// 基础模板：处理数值类型
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type
to_sql_value(const T& value) {
    return std::to_string(value);
}

// 字符串特化：处理转义
std::string to_sql_value(const std::string& value) {
    std::ostringstream oss;
    oss << '\'';
    
    for (char c : value) {
        // 转义特殊字符
        if (c == '\'' || c == '\\') {
            oss << '\\';
        }
        oss << c;
    }
    
    oss << '\'';
    return oss.str();
}

// 布尔类型特化
template <>
std::string to_sql_value<bool>(const bool& value) {
    return value ? "1" : "0";
}

// 空值处理 - 使用重载代替特化
inline std::string to_sql_value(std::nullptr_t) {
    return "NULL";
}

// 字符指针处理 - 使用重载
inline std::string to_sql_value(const char* value) {
    return to_sql_value(std::string(value));
}

// ODB 查询列支持
template <typename Column>
typename std::enable_if<is_query_column<Column>::value, std::string>::type
to_sql_value(const Column& col) {
    return col.column();
}

// 运算类型枚举
enum class operation_type {
    assign,    // =
    add,       // +=
    subtract,  // -=
    multiply,  // *=
    divide     // /=
};

// 字段更新表达式包装类
template <typename Column, typename Value>
struct update_expression {
    const Column& column;
    const Value& value;
    operation_type op;

    update_expression(const Column& col, const Value& val, operation_type operation)
        : column(col), value(val), op(operation) {}
    
    // 生成MySQL兼容的更新表达式
    std::string get_expression() const {
        std::string col_name = column.column();
        std::string val_str = to_sql_value(value);

        switch (op) {
        case operation_type::assign:
            return col_name + " = " + val_str;
        case operation_type::add:
            return col_name + " = " + col_name + " + " + val_str;
        case operation_type::subtract:
            return col_name + " = " + col_name + " - " + val_str;
        case operation_type::multiply:
            return col_name + " = " + col_name + " * " + val_str;
        case operation_type::divide:
            return col_name + " = " + col_name + " / " + val_str;
        default:
            throw std::invalid_argument("Invalid operation type");
        }
    }
};

// 辅助函数：创建更新表达式
template <typename Column, typename Value>
update_expression<Column, Value> set(const Column& column, const Value& value) {
    return update_expression<Column, Value>(column, value, operation_type::assign);
}

template <typename Column, typename Value>
update_expression<Column, Value> add(const Column& column, const Value& value) {
    return update_expression<Column, Value>(column, value, operation_type::add);
}

template <typename Column, typename Value>
update_expression<Column, Value> subtract(const Column& column, const Value& value) {
    return update_expression<Column, Value>(column, value, operation_type::subtract);
}

template <typename Column, typename Value>
update_expression<Column, Value> multiply(const Column& column, const Value& value) {
    return update_expression<Column, Value>(column, value, operation_type::multiply);
}

template <typename Column, typename Value>
update_expression<Column, Value> divide(const Column& column, const Value& value) {
    return update_expression<Column, Value>(column, value, operation_type::divide);
}

} // namespace mysql
} // namespace odb
