
#pragma once

#include <iostream>
#include "Page.h"

class ISQ {
public:
    int rebuilds = 0;

private:
    Page* main;
    Page* ovf;
    Page* index;

    double alfa;
    double threshold;

    int reads;
    int writes;

    int lastSrc;
    int lastOvfRecord;
    int lastMainRecord;
    int lastPage;

    struct recordInfo {
        int mainOffset = -1;
        int mainKey = -1;
        int mainPtr = -1;

        int ovfOffset = -1;
        int ovfKey = -1;
        int ovfPtr = -1;
    };

public:
    ISQ();
    ISQ(double alfa, double threshold);

    recordInfo* findRecord(int key, int exact = 0);

    void addRecord(char* data, int key);
    void removeRecord(int key);
    void updateRecord(char* data, int key, int newKey);

    void rebuild();
    int autoRebuild();
    char* getNextRecord();

    void resetForGeneration();

    void fillUpBlanks(Page* page, int &count, int end);

    void printFile();
    void printIndex();
    void printOperations();
    void resetOperations();
    int getWrites();

    void clearFile(const char* name);

    ~ISQ();
};

