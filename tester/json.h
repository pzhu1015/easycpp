#pragma once
#include "json_serialize.h"
#include "datetime.h"
#include "obj.h"

REGIST_CLASS_JSON(::datetime::DateTime);

REGIST_MEMBER_JSON(
    test::Account,
    PLAIN(ID),
    PLAIN(Domain),
    PLAIN(Password)
);

REGIST_MEMBER_JSON(
    test::DirectoryInDTO,
    PLAIN(EventCallingFunction),
    PLAIN(UserAgent),
    PLAIN(Ip),
    PLAIN(AuthName),
    PLAIN(Domain)
);

REGIST_MEMBER_JSON(
    test::DirectoryOutDTO,
    PLAIN(Account)
);

REGIST_MEMBER_JSON(
    test::SubObject,
    NAME(String, "string"),
    NAME(Int32, "int32")
);

//注册时要放到全局位置注册
REGIST_MEMBER_JSON(
    test::Object, 
    NAME(Sub,"sub"),
    NAME(SubPtr,"sub_ptr"),
    NAME(VectorSub,"vector_sub"),
    NAME(VectorSubPtr,"vector_sub_ptr"),
    NAME(DateTime,"datetime"),
    NAME(String,"string"),
    NAME(Float,"float"),
    NAME(Bool,"bool"),
    NAME(Double,"double"),
    NAME(Int8,"int8"),
    NAME(Int16,"int16"),
    NAME(Int32,"int32"),
    NAME(Int64,"int64"),
    NAME(UInt8,"uint8"),
    NAME(UInt16,"uint16"),
    NAME(UInt32,"uint32"),
    NAME(UInt64,"uint64"),
    NAME(VectorString,"vector_string"),
    NAME(VectorFloat,"vector_float"),
    NAME(VectorDouble,"vector_double"),
    NAME(VectorInt8,"vector_int8"),
    NAME(VectorInt16,"vector_int16"),
    NAME(VectorInt32,"vector_int32"),
    NAME(VectorInt64,"vector_int64"),
    NAME(VectorUInt8,"vector_uint8"),
    NAME(VectorUInt16,"vector_uint16"),
    NAME(VectorUInt32,"vector_uint32"),
    NAME(VectorUInt64,"vector_uint64"),
    NAME(ListString,"list_string"),
    NAME(ListFloat,"list_float"),
    NAME(ListDouble,"list_double"),
    NAME(ListInt8,"list_int8"),
    NAME(ListInt16,"list_int16"),
    NAME(ListInt32,"list_int32"),
    NAME(ListInt64,"list_int64"),
    NAME(ListUInt8,"list_uint8"),
    NAME(ListUInt16,"list_uint16"),
    NAME(ListUInt32,"list_uint32"),
    NAME(ListUInt64,"list_uint64"),
    NAME(SetString,"set_string"),
    NAME(SetFloat,"set_float"),
    NAME(SetDouble,"set_double"),
    NAME(SetInt8,"set_int8"),
    NAME(SetInt16,"set_int16"),
    NAME(SetInt32,"set_int32"),
    NAME(SetInt64,"set_int64"),
    NAME(SetUInt8,"set_uint8"),
    NAME(SetUInt16,"set_uint16"),
    NAME(SetUInt32,"set_uint32"),
    NAME(SetUInt64,"set_uint64")
);

