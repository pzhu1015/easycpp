#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <memory>

#ifdef PHONEDATA_GZIP
//*. yum install zlib-devel
//*. tar -czvf phone.dat phone.dat.gz
//*. xxd -i phone.dat.gz > phone_gz.h
#include <zlib.h>
#include "phone_gz.h"
#else
//*. yum install xz-devel
//*. xz -9 phone.dat
//*. xxd -i phone.dat.xz > phone_xz.h
#include <lzma.h>
#include "phone_xz.h"
#endif

#ifdef EASYCPP_LOGGING
#include "logger.h"
#else
#define DEBUG(...)  ((void)0)
#define INFO(...) ((void)0)
#define WARNING(...) ((void)0)
#define ERROR(...) ((void)0)
#endif

namespace phonedata
{
const uint8_t CMCC = 0x01;
const uint8_t CUCC = 0x02;
const uint8_t CTCC = 0x03;
const uint8_t CTCC_v = 0x04;
const uint8_t CUCC_v = 0x05;
const uint8_t CMCC_v = 0x06;
const uint8_t CBCC = 0x07;
const uint8_t CBCC_v = 0x08;

const int INT_LEN = 4;
const int CHAR_LEN = 1;
const int HEAD_LENGTH = 8;
const int PHONE_INDEX_LENGTH = 9;

class PhoneRecord
{
public:
    PhoneRecord(
        const std::string &number,
        const std::string &province,
        const std::string &city,
        const std::string &zip_code,
        const std::string &area_zone,
        const std::string &card_type)
    :
    Number(number),
    Province(province),
    City(city),
    ZipCode(zip_code),
    AreaZone(area_zone),
    CardType(card_type)
    {
    }
    std::string Number;
    std::string Province;
    std::string City;
    std::string ZipCode;
    std::string AreaZone;
    std::string CardType;

    std::string ToString() const {
        std::ostringstream oss;
        oss << "PhoneNum: " << Number << "\n";
        oss << "AreaZone: " << AreaZone << "\n";
        oss << "CardType: " << CardType << "\n";
        oss << "City: " << City << "\n";
        oss << "ZipCode: " << ZipCode << "\n";
        oss << "Province: " << Province << "\n";
        return oss.str();

    }
};

using PhoneRecordPtr = std::shared_ptr<PhoneRecord>;

class PhoneData
{
public:
    bool Start()
    {
        try
        {
#ifdef PHONEDATA_GZIP
            INFO("init by the gzip");
            this->_data = decompress(phone_dat_gz, phone_dat_gz_len);
#else
            INFO("init by the xz");
            this->_data = decompress(phone_dat_xz, phone_dat_xz_len);
#endif
            return true;
        }
        catch(std::exception &ex)
        {
            ERROR(ex.what());
        }
        return false;
    }

    PhoneRecordPtr Find(const std::string &number)
    {
        if (number.length() < 7 || number.length() > 11) 
        {
            return nullptr;
        }

        uint32_t phone_seven_uint = getN(number.substr(0, 7));
        int32_t phone_seven_int32 = static_cast<int32_t>(phone_seven_uint);

        int32_t total_len = static_cast<int32_t>(this->_data.size());
        int32_t firstoffset = get4(this->_data, INT_LEN);

        int32_t left = 0;
        int32_t right = (total_len - firstoffset) / PHONE_INDEX_LENGTH;

        while (left <= right) 
        {
            int32_t mid = (left + right) / 2;
            int32_t offset = firstoffset + mid * PHONE_INDEX_LENGTH;
            if (offset >= total_len) 
            {
                break;
            }

            int32_t cur_phone = get4(this->_data, offset);
            int32_t record_offset = get4(this->_data, offset + INT_LEN);
            uint8_t card_type = this->_data[offset + INT_LEN * 2];

            if (cur_phone > phone_seven_int32) 
            {
                right = mid - 1;
            } 
            else if (cur_phone < phone_seven_int32) 
            {
                left = mid + 1;
            } 
            else 
            {
                std::vector<uint8_t> cbyte(this->_data.begin() + record_offset, this->_data.end());
                auto end_offset = std::find(cbyte.begin(), cbyte.end(), '\0') - cbyte.begin();
                std::vector<uint8_t> data_bytes(cbyte.begin(), cbyte.begin() + end_offset);
                std::string data_str(data_bytes.begin(), data_bytes.end());

                std::vector<std::string> data;
                std::stringstream ss(data_str);
                std::string token;
                while (std::getline(ss, token, '|')) 
                {
                    data.push_back(token);
                }

                std::string card_str;
                switch (card_type) 
                {
                case CMCC: card_str = "中国移动"; break;
                case CUCC: card_str = "中国联通"; break;
                case CTCC: card_str = "中国电信"; break;
                case CBCC: card_str = "中国广电"; break;
                case CTCC_v: card_str = "中国电信虚拟运营商"; break;
                case CUCC_v: card_str = "中国联通虚拟运营商"; break;
                case CMCC_v: card_str = "中国移动虚拟运营商"; break;
                case CBCC_v: card_str = "中国广电虚拟运营商"; break;
                default: card_str = "未知电信运营商";
                }
                auto pr = std::make_shared<PhoneRecord>(number, data[0], data[1], data[2], data[3], card_str);
                return pr;
            }
        }
        return nullptr;
    }
    
    static PhoneData* Instance()
    {
        static PhoneData phonedata;
        return &phonedata;
    }
private:
    std::vector<uint8_t> _data;
    int32_t get4(const std::vector<uint8_t>& b, size_t start) {
        if (start + 4 > b.size()) 
        {
            return 0;
        }
        return static_cast<int32_t>(b[start]) |
            static_cast<int32_t>(b[start + 1]) << 8 |
            static_cast<int32_t>(b[start + 2]) << 16 |
            static_cast<int32_t>(b[start + 3]) << 24;
    }

    uint32_t getN(const std::string& s) {
        uint32_t n = 0;
        for (char c : s) 
        {
            if (c < '0' || c > '9') 
            {
                throw std::invalid_argument("invalid syntax");
            }
            uint32_t digit = c - '0';
            if (n > (UINT32_MAX - digit) / 10) 
            {
                throw std::out_of_range("value out of range");
            }
            n = n * 10 + digit;
        }
        return n;
    }

#ifdef PHONEDATA_GZIP
    std::vector<uint8_t> decompress(const unsigned char* data, size_t data_size) 
    {
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = static_cast<uInt>(data_size);
        strm.next_in = const_cast<Bytef*>(data);

        if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) 
        {
            throw std::runtime_error("Failed to initialize zlib for decompression");
        }

        std::vector<uint8_t> decompressed_data;
        const size_t chunk_size = 1024;
        do 
        {
            decompressed_data.resize(decompressed_data.size() + chunk_size);
            strm.avail_out = static_cast<uInt>(chunk_size);
            strm.next_out = &decompressed_data[decompressed_data.size() - chunk_size];

            int ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR) 
            {
                inflateEnd(&strm);
                throw std::runtime_error("Zlib stream error during decompression");
            }

            switch (ret) 
            {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                throw std::runtime_error("Zlib data or memory error during decompression");
            }

        } while (strm.avail_out == 0);
        decompressed_data.resize(decompressed_data.size() - strm.avail_out);
        inflateEnd(&strm);
        return decompressed_data;
    }
#else
    std::vector<uint8_t> decompress(const unsigned char* data, size_t data_size) 
    {
        lzma_stream strm = LZMA_STREAM_INIT;
        lzma_ret ret;

        // 初始化解码器，支持自动检测文件格式
        ret = lzma_auto_decoder(&strm, UINT64_MAX, 0);
        if (ret != LZMA_OK) 
        {
            throw std::runtime_error("Failed to initialize xz decoder");
        }

        // 设置输入数据
        strm.next_in = data;
        strm.avail_in = data_size;

        std::vector<uint8_t> decompressed_data;
        const size_t chunk_size = 4096;  // 使用更大的块大小提高效率
        do 
        {
            // 调整输出缓冲区大小
            size_t old_size = decompressed_data.size();
            decompressed_data.resize(old_size + chunk_size);

            strm.next_out = &decompressed_data[old_size];
            strm.avail_out = chunk_size;

            // 执行解压
            ret = lzma_code(&strm, LZMA_RUN);

            if (ret == LZMA_STREAM_END) 
            {
                // 解压完成，调整向量大小为实际数据量
                decompressed_data.resize(old_size + (chunk_size - strm.avail_out));
                break;
            }

            if (ret != LZMA_OK) 
            {
                lzma_end(&strm);
                throw std::runtime_error("xz decompression failed");
            }

        } while (strm.avail_in > 0);

        lzma_end(&strm);
        return decompressed_data;
    } 
#endif
};
}
