#pragma once
#include "param_serialize.h"
#include "obj.h"

REGIST_MEMBER_PARAM(
    test::Object,
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
    NAME(UInt64,"uint64")
);

