#include "Page.h"

int charVal(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

int atoiHex(char* str) {
    int len = strlen(str);
    int mul = 1, sum = 0;
    for (int i=len-1; i>=0; i--) {
        sum = sum + (charVal(str[i]) * mul);
        mul = mul * 16;
    }
    return sum;
}

void getStringFromRecord(char *dst, char *src, int stringNum) {
    memcpy(dst, src+8*stringNum, 8);
    dst[8] = '\0';
}

int getKey(char *src) {
    char dst[9];
    if (src == nullptr)
        return -1;
    getStringFromRecord(dst, src, 0);
    return atoiHex(dst);
}

int isRecordEmpty(char *src) {
    if (src == nullptr)
        return 1;
    for (int i=0; i<8; i++) {
        if (src[i] != '0')
            return 0;
    }
    return 1;
}

void getData(char *dst, char *src) {
    getStringFromRecord(dst, src, 1);
}

int getPointer(char *src) {
    char dst[9];
    if (src == nullptr)
        return -1;
    getStringFromRecord(dst, src, 2);
    return atoiHex(dst);
}

int getPageNo(char *src) {
    char dst[9];
    if (src == nullptr)
        return -1;
    getStringFromRecord(dst, src, 1);
    return atoiHex(dst);
}

