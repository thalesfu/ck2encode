#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <iomanip>


typedef int errno_t;
typedef unsigned char byte;

void printHexW(const wchar_t* wstr, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(4) << std::setfill('0') << wstr[i] << ' ';
    }
    std::cout << std::endl;
}

inline wchar_t UCS2ToCP1252(int cp) {
    wchar_t result = cp;
    switch (cp) {
        case 0x20AC: result = 0x80;
            break;
        case 0x201A: result = 0x82;
            break;
        case 0x0192: result = 0x83;
            break;
        case 0x201E: result = 0x84;
            break;
        case 0x2026: result = 0x85;
            break;
        case 0x2020: result = 0x86;
            break;
        case 0x2021: result = 0x87;
            break;
        case 0x02C6: result = 0x88;
            break;
        case 0x2030: result = 0x89;
            break;
        case 0x0160: result = 0x8A;
            break;
        case 0x2039: result = 0x8B;
            break;
        case 0x0152: result = 0x8C;
            break;
        case 0x017D: result = 0x8E;
            break;
        case 0x2018: result = 0x91;
            break;
        case 0x2019: result = 0x92;
            break;
        case 0x201C: result = 0x93;
            break;
        case 0x201D: result = 0x94;
            break;
        case 0x2022: result = 0x95;
            break;
        case 0x2013: result = 0x96;
            break;
        case 0x2014: result = 0x97;
            break;
        case 0x02DC: result = 0x98;
            break;
        case 0x2122: result = 0x99;
            break;
        case 0x0161: result = 0x9A;
            break;
        case 0x203A: result = 0x9B;
            break;
        case 0x0153: result = 0x9C;
            break;
        case 0x017E: result = 0x9E;
            break;
        case 0x0178: result = 0x9F;
            break;
    }

    return result;
}

errno_t convertWideTextToEscapedText(const wchar_t *from, char **to) {
    errno_t success = 0;
    int toIndex = 0;
    unsigned int size = 0;

    /* */
    if (from == NULL) {
        success = 1;
        goto A;
    }

    /* */
    size = wcslen(from);

    /* 全部エスケープしても３倍を超えることはない。１０はバッファ*/
    *to = (char *) calloc(size * 3 * 2 + 10, sizeof(char));

    if (*to == NULL) {
        success = 2;
        goto A;
    }

    /* */
    toIndex = 0;
    for (unsigned int fromIndex = 0; fromIndex < size; fromIndex++) {
        wchar_t cp = from[fromIndex];

        /* */
        if (UCS2ToCP1252(cp) != cp) {
            (*to)[toIndex++] = (byte) cp;
            continue;
        }

        /* ずらす */
        if (cp > 0x100 && cp < 0xA00) {
            cp = cp + 0xE000;
        }

        /* 上位バイト */
        byte high = (cp >> 8) & 0x000000FF;

        /* 下位バイト */
        byte low = cp & 0x000000FF;

        byte escapeChr = 0x10;

        /* 2byteじゃない */
        if (high == 0) {
            (*to)[toIndex++] = (byte) cp;
            continue;
        }

        /* high byteより決定 */
        switch (high) {
            case 0xA4:
            case 0xA3:
            case 0xA7:
            case 0x24:
            case 0x5B:
            case 0x00:
            case 0x5C:
            case 0x20:
            case 0x0D:
            case 0x0A:
            case 0x22:
            case 0x7B:
            case 0x7D:
            case 0x40:
            case 0x80:
            case 0x7E:
            case 0x2F:
            case 0xBD:
            case 0x3B:
            case 0x5D:
            case 0x5F:
            case 0x3D:
            case 0x23:
                escapeChr += 2;
                break;
            default:
                break;
        }

        /* low byteより決定 */
        switch (low) {
            case 0xA4:
            case 0xA3:
            case 0xA7:
            case 0x24:
            case 0x5B:
            case 0x00:
            case 0x5C:
            case 0x20:
            case 0x0D:
            case 0x0A:
            case 0x22:
            case 0x7B:
            case 0x7D:
            case 0x40:
            case 0x80:
            case 0x7E:
            case 0x2F:
            case 0xBD:
            case 0x3B:
            case 0x5D:
            case 0x5F:
            case 0x3D:
            case 0x23:
                escapeChr++;
                break;
            default:
                break;
        }

        switch (escapeChr) {
            case 0x11:
                low += 15;
                break;
            case 0x12:
                high -= 9;
                break;
            case 0x13:
                low += 15;
                high -= 9;
                break;
            case 0x10:
            default:
                break;
        }

        (*to)[toIndex++] = escapeChr;
        (*to)[toIndex++] = low;
        (*to)[toIndex++] = high;
    }

A:
    return success;
}

errno_t convertTextToWideText(const char *from, wchar_t **to) {
    errno_t success = 0;
    unsigned int err = 0;
    unsigned int wideTextSize = 0;

    /* */
    if (from == NULL) {
        success = 1;
        goto A;
    }

    /* */
    wideTextSize = MultiByteToWideChar(
        CP_UTF8,
        NULL,
        from,
        -1,
        NULL,
        NULL);

    if (wideTextSize == NULL) {
        success = GetLastError();
        goto A;
    }

    /* */
    *to = (wchar_t *) calloc(wideTextSize, sizeof(wchar_t));

    if (*to == NULL) {
        success = 3;
        goto A;
    }

    /* */
    err = MultiByteToWideChar(
        CP_UTF8,
        NULL,
        from,
        -1,
        *to,
        wideTextSize);

    if (err == NULL) {
        success = 4;
        goto B;
    }
    goto A;

B:
    free(*to);
A:
    return success;
}

void printHex(const std::string &str) {
    for (unsigned char c: str) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c) << " ";
    }
    std::cout << std::endl;
}

int main() {
    // 原始UTF-8字符串
    const char *original = "兰英";

    std::string originalStr(original);

    std::cout << "Original string in hex: ";
    printHex(originalStr);

    // 第一步：将UTF-8字符串转换为宽字符串
    wchar_t *wideText;
    errno_t result = convertTextToWideText(original, &wideText);
    if (result != 0) {
        std::cerr << "Failed to convert text to wide text." << std::endl;
        return 1;
    }

    size_t len = wcslen(wideText);
    std::cout << "C++ UTF-16 hex: ";
    printHexW(wideText, len);

    // 第二步：将宽字符串转换为转义后的字符串
    char *escapedText;
    result = convertWideTextToEscapedText(wideText, &escapedText);
    if (result != 0) {
        std::cerr << "Failed to convert wide text to escaped text." << std::endl;
        free(wideText); // 清理
        return 1;
    }

    // 打印转换后的字符串
    std::cout << "Escaped Text: " << escapedText << std::endl;

    // 创建并打开一个文本文件
    std::ofstream outFile("T:\\codes\\github.com\\thalesfu\\cplusplus\\name2.txt");
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        free(wideText);
        free(escapedText);
        return 1;
    }

    // 写入转换后的字符串到文件
    outFile << escapedText;

    // 关闭文件
    outFile.close();

    // 清理分配的内存
    free(wideText);
    free(escapedText);

    return 0;
    // std::cout << "Hello, World!" << std::endl;
    // return 0;
}
