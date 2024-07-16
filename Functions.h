
#pragma once

int charVal(char c);
int atoiHex(char* str);
void getStringFromRecord(char *dst, char *src, int stringNum);
int compare(char* next, int key);
int isRecordEmpty(char *src);

int getKey(char *src);
void getData(char *dst, char *src);
int getPointer(char *src);
int getPageNo(char *src);
void setKey(char* dst, int key);