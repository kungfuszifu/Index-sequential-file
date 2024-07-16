
#include "Page.h"
#include "Functions.h"

Page::Page(const char *name, int recordSize, int perPage) {
    strcpy(fileName, name);
    this->recordsPerPage = perPage;
    this->recordSize = recordSize;
    this->pageSize = recordSize * perPage;
    this->validBytes = 0;
    this->fileOffset = -1;
    this->reads = 0;
    this->writes = 0;
}

Page::~Page() = default;

void Page::reset() {
    this->validBytes = 0;
    this->fileOffset = -1;
}

int Page::writeFile(int recordOffset) {
    int offset = (recordOffset * recordSize) / pageSize * pageSize;

    FILE* file = fopen(fileName, "r+");
    if (file == nullptr)
        return -1;

    fseek(file, offset, SEEK_SET);
    fwrite(array, sizeof(char), validBytes, file);

    this->writes++;

    fclose(file);
    return validBytes;
}

int Page::readFile(int recordOffset) {
    int offset = (recordOffset * recordSize) / pageSize * pageSize;

    FILE* file = fopen(fileName, "r");
    if (file == nullptr)
        return -1;

    fseek(file, -offset, SEEK_END);
    int bytesToRead = ftell(file);
    fseek(file, offset, SEEK_SET);
    bytesToRead = std::min(bytesToRead, pageSize);

    if (bytesToRead > 0)
        fread(array, sizeof(char), bytesToRead, file);

    this->validBytes = bytesToRead;
    this->fileOffset = offset;
    this->reads++;

    fclose(file);
    return bytesToRead;
}

char *Page::getRecord(int recordOffset) {
    if (recordOffset == -1)
        return nullptr;

    int page = (recordOffset * recordSize) / pageSize;
    if (page * pageSize != fileOffset)
        readFile(recordOffset);

    int recordOff = (recordOffset * recordSize) % pageSize;
    if (recordOff >= validBytes)
        return nullptr;

    return array + recordOff;
}

void Page::modifyRecord(int recordOffset, int* key, char *data, int* pointer) {
    char* record = getRecord(recordOffset);
    int offset = 0;
    char temp[9];

    if (key != nullptr) {
        snprintf(temp, 9, "%08X", *key);
        memcpy(record + offset, temp, 8);
    }
    offset += 8;
    if (recordSize == 25) {
        if (data != nullptr) {
            snprintf(temp, 9, "%s", data);
            memcpy(record + offset, temp, 8);
        }
        offset += 8;
    }
    if (pointer != nullptr) {
        snprintf(temp, 9, "%08X", *pointer);
        memcpy(record + offset, temp, 8);
    }
}

void Page::appendRecord(int key, char *data, int pointer) {
    char temp[32];
    if (recordSize == 25)
        snprintf(temp, recordSize+2, "%08X%s%08X\n", key, data, pointer);
    else
        snprintf(temp, recordSize+2, "%08X%08X\n", key, pointer);

    getRecord(fileSize());
    memcpy(array + validBytes, temp, recordSize);
    this->validBytes += recordSize;
}

int Page::fileSize() {
    FILE* file = fopen(fileName, "r");
    if (file == nullptr)
        return -1;
    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fclose(file);
    return fileSize / recordSize;
}

int Page::pageOffset(int key) { // pierwsza wieksa od klucza
    int fsize = fileSize()-1;
    int half, a = 0, b = fsize, ikey;
    char* val;

    while (true) {

        half = (a+b)/2;
        val = getRecord(half);

        ikey = getKey(val);

        if (ikey <= key) {
            if (half == fsize) {
                half = fsize;
                break;
            }

            char* next = getRecord(half + 1);
            int nextKey = getKey(next);
            if (nextKey > key)
                break;
        }

        if (ikey <= key)
            if (a + 1 == b)
                a = b;
            else
                a = half;
        else
            b = half;

    }

    return half * recordsPerPage;
}

void Page::createIndex(int recordOffset, int key) {
    appendRecord(key, nullptr, recordOffset+1);
}


void Page::resetOperations() {
    this->reads = 0;
    this->writes = 0;
}

int Page::getWrites() { return writes; }

int Page::getReads() { return reads; }

char *Page::getFileName() { return fileName; }