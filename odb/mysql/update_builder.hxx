#pragma once
#include "odb/mysql/builder.hxx"

namespace odb {
namespace mysql {

template <typename Model>
class update_builder {
public:
    using query_type = odb::query<Model>;
    using traits_type = access::object_traits_impl<Model, id_mysql>;
    using condition_type = std::function<query_type(const query_type&)>;

    // 构造函数 - 获取表名
    update_builder(query_type base_query = query_type(true)) 
        : table_name_(traits_type::table_name),
          where_clause_(base_query) {}  // 初始为true（无条件）

    // 添加更新字段
    template <typename Column, typename Value>
    update_builder& update_column(const update_expression<Column, Value>& expr) {
        static_assert(is_query_column<Column>::value, 
                     "Must use ODB query column for update");
        
        if (!updates_.empty()) {
            updates_ += ", ";
        }
        
        updates_ += expr.get_expression();
        return *this;
    }

    // 支持多个字段更新
    template <typename Column, typename Value, typename... Rest>
    update_builder& update_column(const update_expression<Column, Value>& first, const Rest&... rest) {
        update_column(first);
        update_column(rest...);
        return *this;
    }
    // 添加 AND 条件（单条件）
    update_builder& where(const query_type& condition) {
        where_clause_ = where_clause_ && condition;
        return *this;
    }

    // 添加 AND 条件（多条件组合，内部用 AND 连接）
    template <typename Arg1, typename Arg2, typename... Args>
    update_builder& where(const Arg1& arg1, const Arg2& arg2, const Args&... args) {
        query_type group = arg1 && arg2;  // 先处理前两个条件
        return where_recursive(group, args...);  // 递归处理剩余条件
    }

    // 添加 OR 条件（单条件）
    update_builder& or_where(const query_type& condition) {
        where_clause_ = where_clause_ || condition;
        return *this;
    }

    // 添加 OR 条件（多条件组合，内部用 AND 连接）
    template <typename Arg1, typename Arg2, typename... Args>
    update_builder& or_where(const Arg1& arg1, const Arg2& arg2, const Args&... args) {
        query_type group = arg1 && arg2;  // 先处理前两个条件
        return or_where_recursive(group, args...);  // 递归处理剩余条件
    }

    // LIMIT
    update_builder& limit(uint64_t limit) {
        limit_ = " LIMIT " + std::to_string(limit);
        return *this;
    }

    // ORDER BY（与query_builder风格一致）
    template <typename Column>
    update_builder& order_by(const order_expression<Column>& expr) {
        static_assert(is_query_column<Column>::value, 
                     "Must use ODB query column for order_by");
        
        if (order_by_.empty()) {
            order_by_ = " ORDER BY ";
        } else {
            order_by_ += ", ";
        }
        
        order_by_ += expr.column.column() + std::string(expr.ascending ? " ASC" : " DESC");
        return *this;
    }

    // 支持多个排序条件
    template <typename Column, typename... Rest>
    update_builder& order_by(const order_expression<Column>& first, const Rest&... rest) {
        order_by(first);
        order_by(rest...);
        return *this;
    }

    // 生成SQL
    std::string str() const {
        if (updates_.empty()) {
            throw std::logic_error("No update expressions specified");
        }

        std::string sql = "UPDATE " + std::string(table_name_) + " SET " + updates_ + " ";
        sql += where_clause_.clause() + order_by_ + limit_;
        return sql;
    }

    // 隐式转换为字符串
    operator std::string() const {
        return str();
    }

private:
    // 递归终止条件（无剩余参数）
    update_builder& where_recursive(query_type& group) {
        return where(group);  // 处理完所有条件，调用单条件接口
    }

    // 递归拼接AND条件
    template <typename Arg, typename... Args>
    update_builder& where_recursive(query_type& group, const Arg& arg, const Args&... args) {
        group = group && arg;  // 拼接当前条件
        return where_recursive(group, args...);  // 继续处理剩余条件
    }

    // 递归终止条件（无剩余参数）
    update_builder& or_where_recursive(query_type& group) {
        return or_where(group);  // 处理完所有条件，调用单条件接口
    }

    // 递归拼接OR条件
    template <typename Arg, typename... Args>
    update_builder& or_where_recursive(query_type& group, const Arg& arg, const Args&... args) {
        group = group && arg;  // 注意：OR分组内部仍然用AND连接
        return or_where_recursive(group, args...);  // 继续处理剩余条件
    }

    const char* table_name_;
    mutable query_type where_clause_;  // 直接用 query_type 存储拼接后的条件
    std::string updates_;
    std::string order_by_;
    std::string limit_;
};
} // namespace mysql
} // namespace odb
