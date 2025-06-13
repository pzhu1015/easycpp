#pragma once
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <licensecc/licensecc.h>
#include <licensecc/DEFAULT/licensecc_properties.h>
#include <licensecc/DEFAULT/public_key.h>

namespace lic
{
static const std::unordered_map<LCC_EVENT_TYPE, std::string> Events = {
    {LICENSE_OK,                       "OK "},
    {LICENSE_FILE_NOT_FOUND,           "许可证文件不存在 "},
    {LICENSE_SERVER_NOT_FOUND,         "无法联系许可证服务器 "},
    {ENVIRONMENT_VARIABLE_NOT_DEFINED, "未定义环境变量 "},
    {FILE_FORMAT_NOT_RECOGNIZED,       "许可证文件格式无效（不是 .ini 文件） "},
    {LICENSE_MALFORMED,                "缺少某些必需字段，或无法完整读取数据 "},
    {PRODUCT_NOT_LICENSED,             "此产品未获得许可 "},
    {PRODUCT_EXPIRED,                  "许可证已过期 "},
    {LICENSE_CORRUPTED,                "许可证签名与当前许可证不匹配 "},
    {IDENTIFIERS_MISMATCH,             "已计算标识符与许可证中提供的标识符不匹配"},
};

static void Acquire(const std::string &license_file, const std::string &module)
{
    try
    {
        std::cout << "验证许可文件: " << license_file << ", 模块: " << module << std::endl;
        LicenseInfo info;
        LicenseLocation location;
        location.license_data_type = LICENSE_PATH;
        strcpy(location.licenseData, license_file.data());
        CallerInformations caller;
        auto result = acquire_license(&caller, &location, &info);
        if (result == LICENSE_OK)
        {
            std::cout << "许可证正常!!!" << std::endl;
            if (!info.linked_to_pc)
            { 
                std::cerr << "许可证中没有硬件签名。这是一个“演示”许可证，适用于所有电脑。" << std::endl;
                std::cerr << "要生成“单一电脑”许可证，请使用选项 -s 调用“发行许可证”，并使用之前获得的硬件标识符。" << std::endl;
                exit(0);
            }
            else
            {
                if (module != caller.feature_name)
                {
                    std::cerr << "证书模块不匹配: " << module << std::endl;
                    exit(0);
                }
                std::cout << "是否过期: " << info.has_expiry << std::endl;
                std::cout << "到期时间: " << info.expiry_date << std::endl;
                std::cout << "还剩时间: " << info.days_left << std::endl;
                if (!info.has_expiry)
                {
                    std::cerr << "许可证过期" << std::endl;
                    exit(0);
                }
            }
        }
        else
        {
            size_t pc_id_sz = LCC_API_PC_IDENTIFIER_SIZE;
            char pc_identifier[pc_id_sz + 1];
            std::cout << "许可证错误：" << Events.at(result) << std::endl;
            if (identify_pc(STRATEGY_DISK, pc_identifier, &pc_id_sz, nullptr)) 
            {
                std::cout << "硬件签名为：[" << pc_identifier << "]" << std::endl;
            } 
            else 
            {
                std::cerr << "获取硬件签名出现错误" << std::endl;
            }
            exit(0);
        }
    }
    catch(std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        exit(0);
    }
}
}


