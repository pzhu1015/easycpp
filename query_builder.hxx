// odb/query-builder.hxx
#pragma once
#include <odb/mysql/query.hxx>
#include <odb/mysql/traits.hxx>
#include <functional>
#include <vector>
#include <type_traits>
#include <utility>
#include <cstdint>
#include <string>

namespace odb {
namespace mysql {

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

template <typename Model>
class query_builder {
public:
    using query_type = odb::query<Model>;
    using condition_type = std::function<query_type(const query_type&)>;

    explicit query_builder(query_type base_query = query_type(true))
        : base_query_(base_query) {}

    // 添加 AND 条件（单条件）
    query_builder& where(const query_type& condition) {
        conditions_.push_back([condition](const query_type& q) {
            return q && condition;
        });
        return *this;
    }

    // 添加 AND 条件（多条件组合）
    template <typename... Args>
    query_builder& where(const Args&... conditions) {
        return add_condition_group(true, conditions...);
    }

    // 添加 OR 条件
    query_builder& or_where(const query_type& condition) {
        conditions_.push_back([condition](const query_type& q) {
            return q || condition;
        });
        return *this;
    }

    // 添加 OR 条件（多条件组合）
    template <typename... Args>
    query_builder& or_where(const Args&... conditions) {
        return add_condition_group(false, conditions...);
    }

    // 排序方法（支持asc()和desc()表达式）
    template <typename Column>
    query_builder& order_by(const order_expression<Column>& expr) {
        static_assert(is_query_column<Column>::value, "Must use ODB query column for order_by");
        
        if (order_by_.empty()) {
            order_by_ = " ORDER BY ";
        } else {
            order_by_ += ", ";
        }
        
        order_by_ += expr.column.column();
        order_by_ += expr.ascending ? " ASC" : " DESC";
        return *this;
    }

    // 多列排序（支持多个asc()和desc()表达式）
    template <typename Column, typename... Rest>
    query_builder& order_by(const order_expression<Column>& first, const Rest&... rest) {
        order_by(first);
        order_by(rest...);
        return *this;
    }

    // 类型安全的分组（单列）
    template <typename Column>
    query_builder& group_by(const Column& column) {
        static_assert(is_query_column<Column>::value, "Must use ODB query column for group_by");
        
        if (group_by_.empty()) {
            group_by_ = " GROUP BY ";
        } else {
            group_by_ += ", ";
        }
        
        group_by_ += column.column();
        return *this;
    }

    // 类型安全的分组（多列）
    template <typename... Columns>
    query_builder& group_by(const Columns&... columns) { 
        static_assert(sizeof...(columns) > 0, "At least one column must be provided");
        
        if (group_by_.empty()) {
            group_by_ = " GROUP BY ";
        } else {
            group_by_ += ", ";
        }
        
        build_group_clause(columns...);
        return *this;
    }

    // 添加分页
    query_builder& limit_offset(uint64_t limit, uint64_t offset = 0) {
        limit_ = " LIMIT " + std::to_string(limit);
        offset_ = offset > 0 ? " OFFSET " + std::to_string(offset) : "";
        return *this;
    }

    // 获取最终查询
    operator query_type() const {
        query_type final_query = base_query_;
        for (const auto& condition : conditions_) {
            final_query = condition(final_query);
        }
        return final_query + group_by_ + order_by_ + limit_ + offset_;
    }

private:
    // 类型特征：检测是否为ODB查询列
    template <typename T>
    struct is_query_column : std::false_type {};
    
    template <typename T, database_type_id ID>
    struct is_query_column<odb::mysql::query_column<T, ID>> : std::true_type {};

    // 递归终止函数
    void build_group_clause() {}

    // 递归构建分组子句
    template <typename First, typename... Rest>
    void build_group_clause(const First& first, const Rest&... rest) {
        static_assert(is_query_column<First>::value, "Must use ODB query column for group_by");
        
        group_by_ += first.column();
        
        if constexpr (sizeof...(rest) > 0) {
            group_by_ += ", ";
            build_group_clause(rest...);
        }
    }

    // 递归添加条件到组
    void add_to_group(query_type&) {} // 基础情况：无操作

    template <typename T, typename... Args>
    void add_to_group(query_type& group, const T& first, const Args&... rest) {
        group = group && first;
        add_to_group(group, rest...);
    }

    // 添加条件组（AND 或 OR 逻辑）
    template <typename... Args>
    query_builder& add_condition_group(bool is_and, const Args&... conditions) {
        // 创建初始查询
        query_type group = is_and ? query_type(true) : query_type(false);
        
        // 递归添加条件到组
        add_to_group(group, conditions...);
        
        // 添加到条件列表
        conditions_.push_back([group, is_and](const query_type& q) {
            return is_and ? (q && group) : (q || group);
        });
        
        return *this;
    }

    query_type base_query_;
    std::vector<condition_type> conditions_;
    std::string order_by_;
    std::string group_by_;
    std::string limit_;
    std::string offset_;
};

} // namespace mysql
} // namespace odb
