#pragma once
#include "odb/mysql/builder.hxx"

namespace odb {
namespace mysql {

template <typename Model>
class query_builder {
public:
    using query_type = odb::query<Model>;

    // 初始化基础查询为 "true"（无初始条件）
    explicit query_builder(query_type base_query = query_type(true))
        : where_clause_(base_query) {}

    // 添加 AND 条件（单条件）
    query_builder& where(const query_type& condition) {
        where_clause_ = where_clause_ && condition;
        return *this;
    }

    // 添加 AND 条件（多条件组合，内部用 AND 连接）
    template <typename Arg1, typename Arg2, typename... Args>
    query_builder& where(const Arg1& arg1, const Arg2& arg2, const Args&... args) {
        query_type group = arg1 && arg2;  // 先处理前两个条件
        return where_recursive(group, args...);  // 递归处理剩余条件
    }

    // 添加 OR 条件（单条件）
    query_builder& or_where(const query_type& condition) {
        where_clause_ = where_clause_ || condition;
        return *this;
    }

    // 添加 OR 条件（多条件组合，内部用 AND 连接）
    template <typename Arg1, typename Arg2, typename... Args>
    query_builder& or_where(const Arg1& arg1, const Arg2& arg2, const Args&... args) {
        query_type group = arg1 && arg2;  // 先处理前两个条件
        return or_where_recursive(group, args...);  // 递归处理剩余条件
    }

    // 排序（单字段）
    template <typename Column>
    query_builder& order_by(const order_expression<Column>& expr) {
        static_assert(is_query_column<Column>::value, "Must use ODB query column for order_by");
        append_order(expr);
        return *this;
    }

    // 排序（多字段）
    template <typename Column, typename... Rest>
    query_builder& order_by(const order_expression<Column>& first, const Rest&... rest) {
        order_by(first);
        order_by(rest...);
        return *this;
    }

    // 分组（单字段）
    template <typename Column>
    query_builder& group_by(const Column& column) {
        static_assert(is_query_column<Column>::value, "Must use ODB query column for group_by");
        append_group(column);
        return *this;
    }

    // 分组（多字段）
    template <typename Column, typename... Rest>
    query_builder& group_by(const Column& first, const Rest&... rest) {
        group_by(first);
        group_by(rest...);
        return *this;
    }

    // 分页
    query_builder& limit_offset(uint64_t limit, uint64_t offset = 0) {
        limit_ = " LIMIT " + std::to_string(limit);
        offset_ = offset > 0 ? " OFFSET " + std::to_string(offset) : "";
        return *this;
    }

    // 构建最终查询
    operator query_type() const {
        return where_clause_ + group_by_ + order_by_ + limit_ + offset_;
    }

private:
    // 递归终止条件（无剩余参数）
    query_builder& where_recursive(query_type& group) {
        return where(group);  // 处理完所有条件，调用单条件接口
    }

    // 递归拼接AND条件
    template <typename Arg, typename... Args>
    query_builder& where_recursive(query_type& group, const Arg& arg, const Args&... args) {
        group = group && arg;  // 拼接当前条件
        return where_recursive(group, args...);  // 继续处理剩余条件
    }

    // 递归终止条件（无剩余参数）
    query_builder& or_where_recursive(query_type& group) {
        return or_where(group);  // 处理完所有条件，调用单条件接口
    }

    // 递归拼接OR条件
    template <typename Arg, typename... Args>
    query_builder& or_where_recursive(query_type& group, const Arg& arg, const Args&... args) {
        group = group && arg;  // 注意：OR分组内部仍然用AND连接
        return or_where_recursive(group, args...);  // 继续处理剩余条件
    }

    // 辅助函数：追加排序条件
    template <typename Column>
    void append_order(const order_expression<Column>& expr) {
        if (order_by_.empty()) {
            order_by_ = " ORDER BY ";
        } else {
            order_by_ += ", ";
        }
        order_by_ += expr.column.column();
        order_by_ += expr.ascending ? " ASC" : " DESC";
    }

    // 辅助函数：追加分组条件
    template <typename Column>
    void append_group(const Column& column) {
        if (group_by_.empty()) {
            group_by_ = " GROUP BY ";
        } else {
            group_by_ += ", ";
        }
        group_by_ += column.column();
    }

    mutable query_type where_clause_;  // 直接用 query_type 存储拼接后的条件
    std::string order_by_;
    std::string group_by_;
    std::string limit_;
    std::string offset_;
};
} // namespace mysql
} // namespace odb
