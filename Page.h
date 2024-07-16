
#pragma once

#include <iostream>

class Page {
public:
    int recordsPerPage;

private:
    int recordSize;
    int pageSize;

    int fileOffset;
    int validBytes;

    int reads;
    int writes;

    char fileName[32];
    char array[1024];

public:
    Page();
    Page(const char* name, int recordSize, int perPage);

    int writeFile(int recordOffset);
    int readFile(int recordOffset);

    char* getRecord(int recordOffset);

    void modifyRecord(int recordOffset, int* key, char* data, int* pointer);
    void appendRecord(int key, char* data, int pointer);

    int fileSize();

    void reset();

    int pageOffset(int key);
    void createIndex(int recordOffset, int key); // append index

    void resetOperations();
    int getReads();
    int getWrites();
    char* getFileName();

    ~Page();
};

