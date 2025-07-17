#pragma once
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <iconv.h>

#ifdef EASYCPP_LOGGING
#include "logger.h"
#else
#define DEBUG(...)  ((void)0)
#define INFO(...) ((void)0)
#define WARNING(...) ((void)0)
#define ERROR(...) ((void)0)
#endif

namespace encoding
{
const static std::string UTF8Name = "UTF-8";
const static std::string UCS2Name = "UTF-16LE";
const static std::string GB18030Name = "GB18030";
const static std::string GBKName = "GBK";
const static std::string GB2312Name = "GB2312";
const static std::string ASCIIName = "ASCII";
const static std::string GSM7Name = "GSM7";
const static std::string LATIN1Name = "LATIN1";

class EncodingException : public std::runtime_error 
{
public:
    using std::runtime_error::runtime_error;
};

class IconvWrapper {
public:
    IconvWrapper(const char* tocode, const char* fromcode) 
    {
        cd = iconv_open(tocode, fromcode);
        if (cd == reinterpret_cast<iconv_t>(-1)) 
        {
            throw EncodingException("iconv_open failed");
        }
    }

    ~IconvWrapper() 
    {
        if (cd != reinterpret_cast<iconv_t>(-1)) 
        {
            iconv_close(cd);
        }
    }

    std::string convert(const std::string& input) 
    {
        std::vector<char> in_buf(input.begin(), input.end());
        char* in_ptr = in_buf.data();
        size_t in_left = input.size();

        // 初始分配输出缓冲区 (输入大小的2倍)
        size_t out_left = input.size() * 2;
        std::vector<char> out_buf(out_left);
        char* out_ptr = out_buf.data();

        while (in_left > 0) 
        {
            size_t result = iconv(cd, &in_ptr, &in_left, &out_ptr, &out_left);
            if (result == static_cast<size_t>(-1)) 
            {
                if (errno == E2BIG) 
                {
                    // 扩展缓冲区
                    size_t used = out_ptr - out_buf.data();
                    out_left += out_buf.size();  // 双倍扩展
                    out_buf.resize(out_buf.size() * 2);
                    out_ptr = out_buf.data() + used;
                } 
                else 
                {
                    throw EncodingException("iconv conversion error");
                }
            }
        }
        return std::string(out_buf.data(), out_ptr - out_buf.data());
    }

private:
    iconv_t cd;

};

class Encoding;
using EncodingPtr = std::shared_ptr<Encoding>;

class Encoding : public std::enable_shared_from_this<Encoding>
{
public:
    Encoding(const std::string &name)
    :
    _name(name)
    {
    }
    virtual ~Encoding() = default;
    std::string Name()
    {
        return this->_name;
    }
    virtual std::string ToUtf8(const std::string &input)
    {
        IconvWrapper converter(UTF8Name.data(), this->_name.data());
        return converter.convert(input);
    }

    virtual std::string FromUtf8(const std::string &input)
    {
        IconvWrapper converter(this->_name.data(), UTF8Name.data());
        return converter.convert(input);
    }

    static EncodingPtr GetEndoding(const std::string &code)
    {
        auto local_code  = code;
        std::transform(local_code.begin(), local_code.end(), local_code.begin(),
        [](unsigned char c) 
        { 
            return std::tolower(c);  
        });
        auto itr = __encodings.find(local_code);
        if (itr == __encodings.end())
        {
            ERROR("not found encoding: %s", local_code.data());
            return nullptr;
        }
        return itr->second;
    }

    static void RegisterEncoding(const EncodingPtr &encoding)
    {
        if (encoding->Name() == UTF8Name)
        {
            UTF8 = encoding;
        }
        else if (encoding->Name() == UCS2Name)
        {
            UCS2 = encoding;
        }
        else if (encoding->Name() == GB18030Name)
        {
            GB18030 = encoding;
        }
        else if (encoding->Name() == GBKName)
        {
            GBK = encoding;
        }
        else if (encoding->Name() == GB2312Name)
        {
            GB2312 = encoding;
        }
        else if (encoding->Name() == ASCIIName)
        {
            ASCII = encoding;
        }
        else if (encoding->Name() == GSM7Name)
        {
            GSM7 = encoding;
        }
        else if (encoding->Name() == LATIN1Name)
        {
            LATIN1 = encoding;
        }
        else
        {
            ERROR("not support encoding");
            return;
        }
        __encodings.insert({encoding->Name(), encoding});
        INFO("regist encoding [%s]", encoding->Name().data());
    }

protected:
    std::string _name;
public:
    static EncodingPtr UTF8;
    static EncodingPtr UCS2;
    static EncodingPtr GB18030;
    static EncodingPtr GBK;
    static EncodingPtr GB2312;
    static EncodingPtr ASCII;
    static EncodingPtr GSM7;
    static EncodingPtr LATIN1;
private:
    static std::unordered_map<std::string, EncodingPtr> __encodings;
};

class UTF8 : public Encoding
{
public:
    UTF8()
    :
    Encoding(UTF8Name)
    {
    }

    virtual std::string ToUtf8(const std::string &input) override
    {
        return input;
    }

    virtual std::string FromUtf8(const std::string &input) override
    {
        return input;
    }
};

class UCS2 : public Encoding
{
public:
    UCS2()
    :
    Encoding(UCS2Name)
    {
    }
};

class GB18030 : public Encoding
{
public:
    GB18030()
    :
    Encoding(GB2312Name)
    {
    }
};

class GBK : public Encoding
{
public:
    GBK()
    :
    Encoding(GBKName)
    {
    }

    virtual std::string ToUtf8(const std::string &input) override
    {
        IconvWrapper converter(UTF8Name.data(), GB18030Name.data());
        return converter.convert(input);
    }

    virtual std::string FromUtf8(const std::string &input) override
    {
        IconvWrapper converter(this->_name.data(), UTF8Name.data());
        return converter.convert(input);
    }
};

class GB2312 : public Encoding
{
public:
    GB2312()
    :
    Encoding(GB2312Name)
    {
    }

    virtual std::string ToUtf8(const std::string &input) override
    {
        IconvWrapper converter(UTF8Name.data(), GB18030Name.data());
        return converter.convert(input);
    }

    virtual std::string FromUtf8(const std::string &input) override
    {
        IconvWrapper converter(this->_name.data(), UTF8Name.data());
        return converter.convert(input);
    }
};

// 扩展字符标志
static const uint8_t ESCAPE_CHAR = 0x1B;

// GSM 03.38 完整字符集映射（基本字符+扩展字符）
// 使用宽字符字面量确保正确的Unicode码点
static const std::unordered_map<char32_t, uint8_t> unicode_to_gsm = {
    // 基本字符集 (0x00-0x7F)
    {U'@', 0x00}, {U'£', 0x01}, {U'$', 0x02}, {U'¥', 0x03}, {U'è', 0x04}, {U'é', 0x05}, 
    {U'ù', 0x06}, {U'ì', 0x07}, {U'ò', 0x08}, {U'Ç', 0x09}, {U'\n', 0x0A}, {U'Ø', 0x0B},
    {U'ø', 0x0C}, {U'\r', 0x0D}, {U'Å', 0x0E}, {U'å', 0x0F}, {U'Δ', 0x10}, {U'_', 0x11},
    {U'Φ', 0x12}, {U'Γ', 0x13}, {U'Λ', 0x14}, {U'Ω', 0x15}, {U'Π', 0x16}, {U'Ψ', 0x17},
    {U'Σ', 0x18}, {U'Θ', 0x19}, {U'Ξ', 0x1A}, {ESCAPE_CHAR, 0x1B}, {U'Æ', 0x1C}, {U'æ', 0x1D},
    {U'ß', 0x1E}, {U'É', 0x1F}, {U' ', 0x20}, {U'!', 0x21}, {U'"', 0x22}, {U'#', 0x23},
    {U'¤', 0x24}, {U'%', 0x25}, {U'&', 0x26}, {U'\'', 0x27}, {U'(', 0x28}, {U')', 0x29},
    {U'*', 0x2A}, {U'+', 0x2B}, {U',', 0x2C}, {U'-', 0x2D}, {U'.', 0x2E}, {U'/', 0x2F},
    {U'0', 0x30}, {U'1', 0x31}, {U'2', 0x32}, {U'3', 0x33}, {U'4', 0x34}, {U'5', 0x35},
    {U'6', 0x36}, {U'7', 0x37}, {U'8', 0x38}, {U'9', 0x39}, {U':', 0x3A}, {U';', 0x3B},
    {U'<', 0x3C}, {U'=', 0x3D}, {U'>', 0x3E}, {U'?', 0x3F}, {U'¡', 0x40}, {U'A', 0x41},
    {U'B', 0x42}, {U'C', 0x43}, {U'D', 0x44}, {U'E', 0x45}, {U'F', 0x46}, {U'G', 0x47},
    {U'H', 0x48}, {U'I', 0x49}, {U'J', 0x4A}, {U'K', 0x4B}, {U'L', 0x4C}, {U'M', 0x4D},
    {U'N', 0x4E}, {U'O', 0x4F}, {U'P', 0x50}, {U'Q', 0x51}, {U'R', 0x52}, {U'S', 0x53},
    {U'T', 0x54}, {U'U', 0x55}, {U'V', 0x56}, {U'W', 0x57}, {U'X', 0x58}, {U'Y', 0x59},
    {U'Z', 0x5A}, {U'Ä', 0x5B}, {U'Ö', 0x5C}, {U'Ñ', 0x5D}, {U'Ü', 0x5E}, {U'§', 0x5F},
    {U'¿', 0x60}, {U'a', 0x61}, {U'b', 0x62}, {U'c', 0x63}, {U'd', 0x64}, {U'e', 0x65},
    {U'f', 0x66}, {U'g', 0x67}, {U'h', 0x68}, {U'i', 0x69}, {U'j', 0x6A}, {U'k', 0x6B},
    {U'l', 0x6C}, {U'm', 0x6D}, {U'n', 0x6E}, {U'o', 0x6F}, {U'p', 0x70}, {U'q', 0x71},
    {U'r', 0x72}, {U's', 0x73}, {U't', 0x74}, {U'u', 0x75}, {U'v', 0x76}, {U'w', 0x77},
    {U'x', 0x78}, {U'y', 0x79}, {U'z', 0x7A}, {U'ä', 0x7B}, {U'ö', 0x7C}, {U'ñ', 0x7D},
    {U'ü', 0x7E}, {U'à', 0x7F},
    
    // 扩展字符集 (通过ESCAPE_CHAR前缀)
    {U'\f', 0x0A}, // 换页符 (ESC 0x0A)
    {U'^',  0x14}, // 脱字符 (ESC 0x14)
    {U'{',  0x28}, // 左花括号 (ESC 0x28)
    {U'}',  0x29}, // 右花括号 (ESC 0x29)
    {U'\\', 0x2F}, // 反斜杠 (ESC 0x2F)
    {U'[',  0x3C}, // 左方括号 (ESC 0x3C)
    {U'~',  0x3D}, // 波浪号 (ESC 0x3D)
    {U']',  0x3E}, // 右方括号 (ESC 0x3E)
    {U'|',  0x40}, // 竖线 (ESC 0x40)
    {U'€',  0x65}  // 欧元符号 (ESC 0x65)
};

static const std::unordered_map<uint8_t, char32_t> gsm_to_unicode = {
    // 基本字符集
    {0x00, U'@'}, {0x01, U'£'}, {0x02, U'$'}, {0x03, U'¥'}, {0x04, U'è'}, {0x05, U'é'},
    {0x06, U'ù'}, {0x07, U'ì'}, {0x08, U'ò'}, {0x09, U'Ç'}, {0x0A, U'\n'}, {0x0B, U'Ø'},
    {0x0C, U'ø'}, {0x0D, U'\r'}, {0x0E, U'Å'}, {0x0F, U'å'}, {0x10, U'Δ'}, {0x11, U'_'},
    {0x12, U'Φ'}, {0x13, U'Γ'}, {0x14, U'Λ'}, {0x15, U'Ω'}, {0x16, U'Π'}, {0x17, U'Ψ'},
    {0x18, U'Σ'}, {0x19, U'Θ'}, {0x1A, U'Ξ'}, {0x1B, ESCAPE_CHAR}, {0x1C, U'Æ'}, {0x1D, U'æ'},
    {0x1E, U'ß'}, {0x1F, U'É'}, {0x20, U' '}, {0x21, U'!'}, {0x22, U'"'}, {0x23, U'#'},
    {0x24, U'¤'}, {0x25, U'%'}, {0x26, U'&'}, {0x27, U'\''}, {0x28, U'('}, {0x29, U')'},
    {0x2A, U'*'}, {0x2B, U'+'}, {0x2C, U','}, {0x2D, U'-'}, {0x2E, U'.'}, {0x2F, U'/'},
    {0x30, U'0'}, {0x31, U'1'}, {0x32, U'2'}, {0x33, U'3'}, {0x34, U'4'}, {0x35, U'5'},
    {0x36, U'6'}, {0x37, U'7'}, {0x38, U'8'}, {0x39, U'9'}, {0x3A, U':'}, {0x3B, U';'},
    {0x3C, U'<'}, {0x3D, U'='}, {0x3E, U'>'}, {0x3F, U'?'}, {0x40, U'¡'}, {0x41, U'A'},
    {0x42, U'B'}, {0x43, U'C'}, {0x44, U'D'}, {0x45, U'E'}, {0x46, U'F'}, {0x47, U'G'},
    {0x48, U'H'}, {0x49, U'I'}, {0x4A, U'J'}, {0x4B, U'K'}, {0x4C, U'L'}, {0x4D, U'M'},
    {0x4E, U'N'}, {0x4F, U'O'}, {0x50, U'P'}, {0x51, U'Q'}, {0x52, U'R'}, {0x53, U'S'},
    {0x54, U'T'}, {0x55, U'U'}, {0x56, U'V'}, {0x57, U'W'}, {0x58, U'X'}, {0x59, U'Y'},
    {0x5A, U'Z'}, {0x5B, U'Ä'}, {0x5C, U'Ö'}, {0x5D, U'Ñ'}, {0x5E, U'Ü'}, {0x5F, U'§'},
    {0x60, U'¿'}, {0x61, U'a'}, {0x62, U'b'}, {0x63, U'c'}, {0x64, U'd'}, {0x65, U'e'},
    {0x66, U'f'}, {0x67, U'g'}, {0x68, U'h'}, {0x69, U'i'}, {0x6A, U'j'}, {0x6B, U'k'},
    {0x6C, U'l'}, {0x6D, U'm'}, {0x6E, U'n'}, {0x6F, U'o'}, {0x70, U'p'}, {0x71, U'q'},
    {0x72, U'r'}, {0x73, U's'}, {0x74, U't'}, {0x75, U'u'}, {0x76, U'v'}, {0x77, U'w'},
    {0x78, U'x'}, {0x79, U'y'}, {0x7A, U'z'}, {0x7B, U'ä'}, {0x7C, U'ö'}, {0x7D, U'ñ'},
    {0x7E, U'ü'}, {0x7F, U'à'},
    
    // 扩展字符映射（实际使用时需要结合ESCAPE_CHAR前缀）
    // 注意：这些映射仅用于解码时的快速查找
    {0x0A | 0x80, U'\f'}, // 标记为扩展字符
    {0x14 | 0x80, U'^'},
    {0x28 | 0x80, U'{'},
    {0x29 | 0x80, U'}'},
    {0x2F | 0x80, U'\\'},
    {0x3C | 0x80, U'['},
    {0x3D | 0x80, U'~'},
    {0x3E | 0x80, U']'},
    {0x40 | 0x80, U'|'},
    {0x65 | 0x80, U'€'}
};


class GSM7 : public Encoding
{
public:
    GSM7()
    :
    Encoding(GSM7Name)
    {
    }

    bool IsValid(const std::string &utf8)
    {
        const char* ptr = utf8.data();
        size_t remaining = utf8.size();

        while (remaining > 0) {
            try {
                const char* start = ptr;
                char32_t cp = utf8_to_codepoint(ptr, remaining);

                // 检查字符是否在GSM7字符集中
                if (get_char_type(cp) == INVALID) {
                    return false;
                }
            } catch (const EncodingException&) {
                return false; // 无效的UTF-8序列
            }
        }
        return true;
    }

    virtual std::string ToUtf8(const std::string &input) override
    {
        if (input.empty()) return "";

        // 计算原始字符数量（考虑转义字符）
        size_t char_count = (input.size() * 8) / 7;
        if ((input.size() * 8) % 7 != 0) {
            char_count++;
        }

        std::vector<uint8_t> unpacked = unpack_7bit(input, char_count);
        std::string result;
        bool escape_next = false;

        for (uint8_t byte : unpacked) {
            if (escape_next) {
                // 标记为扩展字符（最高位置1）
                byte |= 0x80;
                escape_next = false;
            } else if (byte == ESCAPE_CHAR) {
                escape_next = true;
                continue;
            }

            auto it = gsm_to_unicode.find(byte);
            if (it != gsm_to_unicode.end()) {
                codepoint_to_utf8(it->second, result);
            } else {
                // 无效字符，使用替换字符 (U+FFFD)
                codepoint_to_utf8(0xFFFD, result);
            }
        }

        // 处理结尾的转义字符
        if (escape_next) {
            codepoint_to_utf8(0xFFFD, result); // �
        }

        return result;
    }

    virtual std::string FromUtf8(const std::string &input) override
    {
        std::vector<uint8_t> unpacked;
        const char* ptr = input.data();
        size_t remaining = input.size();

        while (remaining > 0) 
        {
            const char* start = ptr;
            char32_t cp = utf8_to_codepoint(ptr, remaining);

            auto it = unicode_to_gsm.find(cp);
            if (it == unicode_to_gsm.end()) 
            {
                throw EncodingException("Character not in GSM7 charset: " + std::to_string(cp));
            }

            // 对于扩展字符，添加转义前缀
            if (get_char_type(cp) == EXTENDED) 
            {
                unpacked.push_back(ESCAPE_CHAR);
            }

            unpacked.push_back(it->second);
        }

        return pack_7bit(unpacked);
    }
private:
    // 字符类型识别
    enum CharType 
    { 
        BASIC, 
        EXTENDED, 
        INVALID 
    };

    // UTF-8辅助函数
    static char32_t utf8_to_codepoint(const char*& str, size_t& remaining)
    {
        if (remaining == 0) return 0;

        const uint8_t lead = static_cast<uint8_t>(*str);

        // 单字节字符 (0xxxxxxx)
        if (lead < 0x80) 
        {
            remaining--;
            return *str++;
        }

        // 两字节字符 (110xxxxx 10xxxxxx)
        if ((lead & 0xE0) == 0xC0) 
        {
            if (remaining < 2) throw EncodingException("Incomplete UTF-8 sequence");
            remaining -= 2;
            char32_t cp = ((lead & 0x1F) << 6) | (static_cast<uint8_t>(str[1]) & 0x3F);
            str += 2;
            return cp;
        }

        // 三字节字符 (1110xxxx 10xxxxxx 10xxxxxx)
        if ((lead & 0xF0) == 0xE0) 
        {
            if (remaining < 3) throw EncodingException("Incomplete UTF-8 sequence");
            remaining -= 3;
            char32_t cp = ((lead & 0x0F) << 12) | 
                ((static_cast<uint8_t>(str[1]) & 0x3F) << 6) |
                (static_cast<uint8_t>(str[2]) & 0x3F);
            str += 3;
            return cp;
        }

        // 四字节字符 (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
        if ((lead & 0xF8) == 0xF0) 
        {
            if (remaining < 4) throw EncodingException("Incomplete UTF-8 sequence");
            remaining -= 4;
            char32_t cp = ((lead & 0x07) << 18) | 
                ((static_cast<uint8_t>(str[1]) & 0x3F) << 12) |
                ((static_cast<uint8_t>(str[2]) & 0x3F) << 6) |
                (static_cast<uint8_t>(str[3]) & 0x3F);
            str += 4;
            return cp;
        }

        throw EncodingException("Invalid UTF-8 start byte");
    }

    static void codepoint_to_utf8(char32_t cp, std::string& out)
    {
        // 有效Unicode范围检查
        if (cp > 0x10FFFF) 
        {
            throw EncodingException("Code point out of Unicode range");
        }

        // ASCII字符
        if (cp <= 0x7F) 
        {
            out += static_cast<char>(cp);
            return;
        }

        // 两字节序列
        if (cp <= 0x7FF) 
        {
            out += static_cast<char>(0xC0 | (cp >> 6));
            out += static_cast<char>(0x80 | (cp & 0x3F));
            return;
        }

        // 三字节序列
        if (cp <= 0xFFFF) 
        {
            out += static_cast<char>(0xE0 | (cp >> 12));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
            return;
        }

        // 四字节序列
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
    
    // 7-bit打包算法
    static std::string pack_7bit(const std::vector<uint8_t>& unpacked)
    {
        if (unpacked.empty()) return "";

        const size_t bit_count = unpacked.size() * 7;
        const size_t byte_count = (bit_count + 7) / 8;
        std::string packed(byte_count, 0);

        size_t bit_pos = 0;
        for (uint8_t byte : unpacked) 
        {
            const size_t byte_index = bit_pos / 8;
            const size_t bit_offset = bit_pos % 8;
            const size_t space_left = 8 - bit_offset;

            // 当前字节的低位部分
            packed[byte_index] |= (byte << bit_offset) & 0xFF;

            // 需要跨字节存储
            if (space_left < 7) 
            {
                if (byte_index + 1 < byte_count) 
                {
                    packed[byte_index + 1] = byte >> space_left;
                }
            }

            bit_pos += 7;
        }

        return packed;
    }
    
    // 7-bit解包算法
    static std::vector<uint8_t> unpack_7bit(const std::string& packed, size_t char_count)
    {
        if (packed.empty() || char_count == 0) return {};

        const size_t packed_size = packed.size();
        std::vector<uint8_t> unpacked;
        unpacked.reserve(char_count);

        size_t bit_pos = 0;
        for (size_t i = 0; i < char_count; ++i) 
        {
            const size_t byte_index = bit_pos / 8;
            const size_t bit_offset = bit_pos % 8;

            if (byte_index >= packed_size) 
            {
                throw EncodingException("Packed data too short for character count");
            }

            uint8_t value = static_cast<uint8_t>(packed[byte_index]) >> bit_offset;

            // 检查是否需要从下一个字节获取高位部分
            if (bit_offset > 1 && byte_index + 1 < packed_size) 
            {
                value |= static_cast<uint8_t>(packed[byte_index + 1]) << (8 - bit_offset);
            }

            unpacked.push_back(value & 0x7F);
            bit_pos += 7;
        }

        return unpacked;
    }

    static CharType get_char_type(char32_t cp)
    {
        auto it = unicode_to_gsm.find(cp);
        if (it == unicode_to_gsm.end()) return INVALID;

        // 检查是否是基本字符
        if (it->second <= 0x7F) 
        {
            // 排除转义字符本身（0x1B只能用于扩展前缀）
            return (it->second == ESCAPE_CHAR) ? EXTENDED : BASIC;
        }
        return EXTENDED;
    }
};

class ASCII : public Encoding
{
public:
    ASCII()
    :
    Encoding(ASCIIName)
    {
    }

    virtual std::string ToUtf8(const std::string &input) override
    {
        return input;
    }

    virtual std::string FromUtf8(const std::string &input) override
    {
        return input;
    }
};

class LATIN1 : public Encoding
{
public:
    LATIN1()
    :
    Encoding(LATIN1Name)
    {
    }
};

EncodingPtr Encoding::UTF8;
EncodingPtr Encoding::UCS2;
EncodingPtr Encoding::GB18030;
EncodingPtr Encoding::GBK;
EncodingPtr Encoding::GB2312;
EncodingPtr Encoding::ASCII;
EncodingPtr Encoding::GSM7;
EncodingPtr Encoding::LATIN1;
std::unordered_map<std::string, EncodingPtr> Encoding::__encodings;
}

namespace 
{
struct EncodingInitializer 
{
    EncodingInitializer() 
    {
        using namespace encoding;
        Encoding::RegisterEncoding(std::make_shared<UTF8>());
        Encoding::RegisterEncoding(std::make_shared<UCS2>());
        Encoding::RegisterEncoding(std::make_shared<GB18030>());
        Encoding::RegisterEncoding(std::make_shared<GBK>());
        Encoding::RegisterEncoding(std::make_shared<GB2312>());
        Encoding::RegisterEncoding(std::make_shared<ASCII>());
        Encoding::RegisterEncoding(std::make_shared<GSM7>());
        Encoding::RegisterEncoding(std::make_shared<LATIN1>());
    }
} encoding_initializer;
}
