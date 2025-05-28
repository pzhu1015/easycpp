#pragma once
#include <memory>
#include <type_traits>

#ifdef ENABLE_EASYCPP_LOG
    #define EASYCPP_LOG \
        do { \
            std::cout << __FILE__ << ":" << __LINE__ << "(" << __func__ << ")" << std::endl;\
        } while (0)
#else
    #define EASYCPP_LOG
#endif


//判断参数是否空
#define EMPTY(...)
#define NOT_EMPTY(x, ...) x
#define HAS_ARGS(...) NOT_EMPTY(EMPTY(__VA_ARGS__), 1)

// 辅助宏：将参数包装为元组
#define PAIR(...) (__VA_ARGS__)

// NICK 宏：创建成员名和序列化名的元组
#define NAME(name, nick) PAIR(name, nick)

// 普通成员：直接使用成员名作为序列化名
#define PLAIN(name) PAIR(name, #name)

// 提取元组中的第一个元素（成员名）
#define EXTRACT_MEMBER(pair) EXTRACT_FIRST pair
#define EXTRACT_FIRST(a, b) a

// 提取元组中的第二个元素（序列化名）
#define EXTRACT_NICK(pair) EXTRACT_SECOND pair
#define EXTRACT_SECOND(a, b) b

#define EXTAND_ARGS(args) args

#define ARG_N_M( _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10,   \
                _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,   \
                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,   \
                _31, _32, _33, _34, _35, _36, _37, _38, _39, _40,   \
                _41, _42, _43, _44, _45, _46, _47, _48, _49, _50,   \
                _51, _52, _53, _54, _55, _56, _57, _58, _59, _60,   \
                _61, _62, _63, _64, _65, _66, _67, _68, _69, _70,   \
                _71, _72, _73, _74, _75, _76, _77, _78, _79, _80,   \
                _81, _82, _83, _84, _85, _86, _87, _88, _89, _90,   \
                _91, _92, _93, _94, _95, _96, _97, _98, _99, _100,  \
                N,...) N

#define ARG_N_RESQ() 100, 99, 98, 97, 96, 95, 94, 93, 92, 91,        \
                      90, 89, 88, 87, 86, 85, 84, 83, 82, 81,        \
                      80, 79, 78, 77, 76, 75, 74, 73, 72, 71,        \
                      70, 69, 68, 67, 66, 65, 64, 63, 62, 61,        \
                      60, 59, 58, 57, 56, 55, 54, 53, 52, 51,        \
                      50, 49, 48, 47, 46, 45, 44, 43, 42, 41,        \
                      40, 39, 38, 37, 36, 35, 34, 33, 32, 31,        \
                      30, 29, 28, 27, 26, 25, 24, 23, 22, 21,        \
                      20, 19, 18, 17, 16, 15, 14, 13, 12, 11,        \
                      10,  9,  8,  7,  6,  5,  4,  3,  2,  1,        \
                      0

#define ARG_N_(...) \
    EXTAND_ARGS(ARG_N_M(__VA_ARGS__))

#define ARG_N(...) \
    ARG_N_(__VA_ARGS__, ARG_N_RESQ())

#define ARGS_CONCAT(x, y) x##y

#define FOREACH_0(what, x, ...)
#define FOREACH_1(what, x, ...) what(x) 
#define FOREACH_2(what, x, ...) what(x) EXTAND_ARGS(FOREACH_1(what, __VA_ARGS__))
#define FOREACH_3(what, x, ...) what(x) EXTAND_ARGS(FOREACH_2(what, __VA_ARGS__))
#define FOREACH_4(what, x, ...) what(x) EXTAND_ARGS(FOREACH_3(what, __VA_ARGS__))
#define FOREACH_5(what, x, ...) what(x) EXTAND_ARGS(FOREACH_4(what, __VA_ARGS__))
#define FOREACH_6(what, x, ...) what(x) EXTAND_ARGS(FOREACH_5(what, __VA_ARGS__))
#define FOREACH_7(what, x, ...) what(x) EXTAND_ARGS(FOREACH_6(what, __VA_ARGS__))
#define FOREACH_8(what, x, ...) what(x) EXTAND_ARGS(FOREACH_7(what, __VA_ARGS__))
#define FOREACH_9(what, x, ...) what(x) EXTAND_ARGS(FOREACH_8(what, __VA_ARGS__))

#define FOREACH_10(what, x, ...) what(x) EXTAND_ARGS(FOREACH_9(what, __VA_ARGS__))
#define FOREACH_11(what, x, ...) what(x) EXTAND_ARGS(FOREACH_10(what, __VA_ARGS__))
#define FOREACH_12(what, x, ...) what(x) EXTAND_ARGS(FOREACH_11(what, __VA_ARGS__))
#define FOREACH_13(what, x, ...) what(x) EXTAND_ARGS(FOREACH_12(what, __VA_ARGS__))
#define FOREACH_14(what, x, ...) what(x) EXTAND_ARGS(FOREACH_13(what, __VA_ARGS__))
#define FOREACH_15(what, x, ...) what(x) EXTAND_ARGS(FOREACH_14(what, __VA_ARGS__))
#define FOREACH_16(what, x, ...) what(x) EXTAND_ARGS(FOREACH_15(what, __VA_ARGS__))
#define FOREACH_17(what, x, ...) what(x) EXTAND_ARGS(FOREACH_16(what, __VA_ARGS__))
#define FOREACH_18(what, x, ...) what(x) EXTAND_ARGS(FOREACH_17(what, __VA_ARGS__))
#define FOREACH_19(what, x, ...) what(x) EXTAND_ARGS(FOREACH_18(what, __VA_ARGS__))

#define FOREACH_20(what, x, ...) what(x) EXTAND_ARGS(FOREACH_19(what, __VA_ARGS__))
#define FOREACH_21(what, x, ...) what(x) EXTAND_ARGS(FOREACH_20(what, __VA_ARGS__))
#define FOREACH_22(what, x, ...) what(x) EXTAND_ARGS(FOREACH_21(what, __VA_ARGS__))
#define FOREACH_23(what, x, ...) what(x) EXTAND_ARGS(FOREACH_22(what, __VA_ARGS__))
#define FOREACH_24(what, x, ...) what(x) EXTAND_ARGS(FOREACH_23(what, __VA_ARGS__))
#define FOREACH_25(what, x, ...) what(x) EXTAND_ARGS(FOREACH_24(what, __VA_ARGS__))
#define FOREACH_26(what, x, ...) what(x) EXTAND_ARGS(FOREACH_25(what, __VA_ARGS__))
#define FOREACH_27(what, x, ...) what(x) EXTAND_ARGS(FOREACH_26(what, __VA_ARGS__))
#define FOREACH_28(what, x, ...) what(x) EXTAND_ARGS(FOREACH_27(what, __VA_ARGS__))
#define FOREACH_29(what, x, ...) what(x) EXTAND_ARGS(FOREACH_28(what, __VA_ARGS__))

#define FOREACH_30(what, x, ...) what(x) EXTAND_ARGS(FOREACH_29(what, __VA_ARGS__))
#define FOREACH_31(what, x, ...) what(x) EXTAND_ARGS(FOREACH_30(what, __VA_ARGS__))
#define FOREACH_32(what, x, ...) what(x) EXTAND_ARGS(FOREACH_31(what, __VA_ARGS__))
#define FOREACH_33(what, x, ...) what(x) EXTAND_ARGS(FOREACH_32(what, __VA_ARGS__))
#define FOREACH_34(what, x, ...) what(x) EXTAND_ARGS(FOREACH_33(what, __VA_ARGS__))
#define FOREACH_35(what, x, ...) what(x) EXTAND_ARGS(FOREACH_34(what, __VA_ARGS__))
#define FOREACH_36(what, x, ...) what(x) EXTAND_ARGS(FOREACH_35(what, __VA_ARGS__))
#define FOREACH_37(what, x, ...) what(x) EXTAND_ARGS(FOREACH_36(what, __VA_ARGS__))
#define FOREACH_38(what, x, ...) what(x) EXTAND_ARGS(FOREACH_37(what, __VA_ARGS__))
#define FOREACH_39(what, x, ...) what(x) EXTAND_ARGS(FOREACH_38(what, __VA_ARGS__))

#define FOREACH_40(what, x, ...) what(x) EXTAND_ARGS(FOREACH_39(what, __VA_ARGS__))
#define FOREACH_41(what, x, ...) what(x) EXTAND_ARGS(FOREACH_40(what, __VA_ARGS__))
#define FOREACH_42(what, x, ...) what(x) EXTAND_ARGS(FOREACH_41(what, __VA_ARGS__))
#define FOREACH_43(what, x, ...) what(x) EXTAND_ARGS(FOREACH_42(what, __VA_ARGS__))
#define FOREACH_44(what, x, ...) what(x) EXTAND_ARGS(FOREACH_43(what, __VA_ARGS__))
#define FOREACH_45(what, x, ...) what(x) EXTAND_ARGS(FOREACH_44(what, __VA_ARGS__))
#define FOREACH_46(what, x, ...) what(x) EXTAND_ARGS(FOREACH_45(what, __VA_ARGS__))
#define FOREACH_47(what, x, ...) what(x) EXTAND_ARGS(FOREACH_46(what, __VA_ARGS__))
#define FOREACH_48(what, x, ...) what(x) EXTAND_ARGS(FOREACH_47(what, __VA_ARGS__))
#define FOREACH_49(what, x, ...) what(x) EXTAND_ARGS(FOREACH_48(what, __VA_ARGS__))

#define FOREACH_50(what, x, ...) what(x) EXTAND_ARGS(FOREACH_49(what, __VA_ARGS__))
#define FOREACH_51(what, x, ...) what(x) EXTAND_ARGS(FOREACH_50(what, __VA_ARGS__))
#define FOREACH_52(what, x, ...) what(x) EXTAND_ARGS(FOREACH_51(what, __VA_ARGS__))
#define FOREACH_53(what, x, ...) what(x) EXTAND_ARGS(FOREACH_52(what, __VA_ARGS__))
#define FOREACH_54(what, x, ...) what(x) EXTAND_ARGS(FOREACH_53(what, __VA_ARGS__))
#define FOREACH_55(what, x, ...) what(x) EXTAND_ARGS(FOREACH_54(what, __VA_ARGS__))
#define FOREACH_56(what, x, ...) what(x) EXTAND_ARGS(FOREACH_55(what, __VA_ARGS__))
#define FOREACH_57(what, x, ...) what(x) EXTAND_ARGS(FOREACH_56(what, __VA_ARGS__))
#define FOREACH_58(what, x, ...) what(x) EXTAND_ARGS(FOREACH_57(what, __VA_ARGS__))
#define FOREACH_59(what, x, ...) what(x) EXTAND_ARGS(FOREACH_58(what, __VA_ARGS__))

#define FOREACH_60(what, x, ...) what(x) EXTAND_ARGS(FOREACH_59(what, __VA_ARGS__))
#define FOREACH_61(what, x, ...) what(x) EXTAND_ARGS(FOREACH_60(what, __VA_ARGS__))
#define FOREACH_62(what, x, ...) what(x) EXTAND_ARGS(FOREACH_61(what, __VA_ARGS__))
#define FOREACH_63(what, x, ...) what(x) EXTAND_ARGS(FOREACH_62(what, __VA_ARGS__))
#define FOREACH_64(what, x, ...) what(x) EXTAND_ARGS(FOREACH_63(what, __VA_ARGS__))
#define FOREACH_65(what, x, ...) what(x) EXTAND_ARGS(FOREACH_64(what, __VA_ARGS__))
#define FOREACH_66(what, x, ...) what(x) EXTAND_ARGS(FOREACH_65(what, __VA_ARGS__))
#define FOREACH_67(what, x, ...) what(x) EXTAND_ARGS(FOREACH_66(what, __VA_ARGS__))
#define FOREACH_68(what, x, ...) what(x) EXTAND_ARGS(FOREACH_67(what, __VA_ARGS__))
#define FOREACH_69(what, x, ...) what(x) EXTAND_ARGS(FOREACH_68(what, __VA_ARGS__))

#define FOREACH_70(what, x, ...) what(x) EXTAND_ARGS(FOREACH_69(what, __VA_ARGS__))
#define FOREACH_71(what, x, ...) what(x) EXTAND_ARGS(FOREACH_70(what, __VA_ARGS__))
#define FOREACH_72(what, x, ...) what(x) EXTAND_ARGS(FOREACH_71(what, __VA_ARGS__))
#define FOREACH_73(what, x, ...) what(x) EXTAND_ARGS(FOREACH_72(what, __VA_ARGS__))
#define FOREACH_74(what, x, ...) what(x) EXTAND_ARGS(FOREACH_73(what, __VA_ARGS__))
#define FOREACH_75(what, x, ...) what(x) EXTAND_ARGS(FOREACH_74(what, __VA_ARGS__))
#define FOREACH_76(what, x, ...) what(x) EXTAND_ARGS(FOREACH_75(what, __VA_ARGS__))
#define FOREACH_77(what, x, ...) what(x) EXTAND_ARGS(FOREACH_76(what, __VA_ARGS__))
#define FOREACH_78(what, x, ...) what(x) EXTAND_ARGS(FOREACH_77(what, __VA_ARGS__))
#define FOREACH_79(what, x, ...) what(x) EXTAND_ARGS(FOREACH_78(what, __VA_ARGS__))

#define FOREACH_80(what, x, ...) what(x) EXTAND_ARGS(FOREACH_79(what, __VA_ARGS__))
#define FOREACH_81(what, x, ...) what(x) EXTAND_ARGS(FOREACH_80(what, __VA_ARGS__))
#define FOREACH_82(what, x, ...) what(x) EXTAND_ARGS(FOREACH_81(what, __VA_ARGS__))
#define FOREACH_83(what, x, ...) what(x) EXTAND_ARGS(FOREACH_82(what, __VA_ARGS__))
#define FOREACH_84(what, x, ...) what(x) EXTAND_ARGS(FOREACH_83(what, __VA_ARGS__))
#define FOREACH_85(what, x, ...) what(x) EXTAND_ARGS(FOREACH_84(what, __VA_ARGS__))
#define FOREACH_86(what, x, ...) what(x) EXTAND_ARGS(FOREACH_85(what, __VA_ARGS__))
#define FOREACH_87(what, x, ...) what(x) EXTAND_ARGS(FOREACH_86(what, __VA_ARGS__))
#define FOREACH_88(what, x, ...) what(x) EXTAND_ARGS(FOREACH_87(what, __VA_ARGS__))
#define FOREACH_89(what, x, ...) what(x) EXTAND_ARGS(FOREACH_88(what, __VA_ARGS__))

#define FOREACH_90(what, x, ...) what(x) EXTAND_ARGS(FOREACH_89(what, __VA_ARGS__))
#define FOREACH_91(what, x, ...) what(x) EXTAND_ARGS(FOREACH_90(what, __VA_ARGS__))
#define FOREACH_92(what, x, ...) what(x) EXTAND_ARGS(FOREACH_91(what, __VA_ARGS__))
#define FOREACH_93(what, x, ...) what(x) EXTAND_ARGS(FOREACH_92(what, __VA_ARGS__))
#define FOREACH_94(what, x, ...) what(x) EXTAND_ARGS(FOREACH_93(what, __VA_ARGS__))
#define FOREACH_95(what, x, ...) what(x) EXTAND_ARGS(FOREACH_94(what, __VA_ARGS__))
#define FOREACH_96(what, x, ...) what(x) EXTAND_ARGS(FOREACH_95(what, __VA_ARGS__))
#define FOREACH_97(what, x, ...) what(x) EXTAND_ARGS(FOREACH_96(what, __VA_ARGS__))
#define FOREACH_98(what, x, ...) what(x) EXTAND_ARGS(FOREACH_97(what, __VA_ARGS__))
#define FOREACH_99(what, x, ...) what(x) EXTAND_ARGS(FOREACH_98(what, __VA_ARGS__))
#define FOREACH_100(what, x, ...) what(x) EXTAND_ARGS(FOREACH_99(what, __VA_ARGS__))
#define FOREACH_(N, what, ...) EXTAND_ARGS(ARGS_CONCAT(FOREACH_, N)(what, __VA_ARGS__))

#define FOREACH(what, ...) FOREACH_(ARG_N(__VA_ARGS__), what, __VA_ARGS__)
template<typename U>
struct is_shared_ptr : std::false_type{};

template<typename U>
struct is_shared_ptr<std::shared_ptr<U>> : std::true_type {};
