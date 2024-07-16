
#include "ISQ.h"
#include "Functions.h"

ISQ::ISQ() {
    main = new Page("plik.txt", 25, 5);
    ovf = new Page("ovf.txt", 25, 5);
    index = new Page("index.txt", 17, 5);
    this->alfa = 0.5;
    this->threshold = 0.2;
}

ISQ::ISQ(double alfa, double threshold) {
    main = new Page("testmain.txt", 25, 4);
    ovf = new Page("testovf.txt", 25, 4);
    index = new Page("testindex.txt", 17, 4);
    this->alfa = alfa;
    this->threshold = threshold;
}

ISQ::~ISQ() {
    delete main;
    delete ovf;
    delete index;
}


ISQ::recordInfo * ISQ::findRecord(int key, int exact) {
    recordInfo* info = new recordInfo;

    int pageOffset = index->pageOffset(key);
    int next = pageOffset, offset = next, _key, lastkey;
    char* record = main->getRecord(next);

    while (true) {

        next++;

        if (next % main->recordsPerPage == 0)
            break;

        lastkey = getKey(record);
        record = main->getRecord(next);
        _key = getKey(record);

        if (record == nullptr)
            break;

        if (exact == 1) {
            if (_key == key) {
                offset = next;
                break;
            }

            if (_key > key) {
                break;
            }

            if (_key != 0)
                offset = next;
        }
        else {
            if (_key > key)
                break;

            if (lastkey > key)
                break;

            if (lastkey < _key && _key < key)
                offset = next;

            if (lastkey > 0 && _key == 0)
                offset = next;
        }
    }

    record = main->getRecord(offset);
    info->mainOffset = offset;
    info->mainKey = getKey(record);
    info->mainPtr = getPointer(record);

    if (info->mainKey == key) {
        return info;
    }
    else {
        next = offset = info->mainPtr;
        while (true) {

            record = ovf->getRecord(next);

            if (record == nullptr)
                break;

            if (exact == 1) {
                if (getKey(record) == key)
                    break;
            }
            else {
                if (getKey(record) > key)
                    break;
            }

            int pointer = getPointer(record);
            if (pointer == -1)
                break;
            else {
                offset = next;
                next = pointer;
            }
        }

        if (exact == 1) {
            if (getKey(record) == key) {
                record = ovf->getRecord(offset);
                info->ovfOffset = offset;
                info->ovfKey = getKey(record);
                info->ovfPtr = getPointer(record);
            }
        }
        else {
            record = ovf->getRecord(offset);
            info->ovfOffset = offset;
            info->ovfKey = getKey(record);
            info->ovfPtr = getPointer(record);
        }
    }
    return info;
}

void ISQ::addRecord(char *data, int key) {
    if (key <= 0) {
        printf("(!) Nie poprawny klucz !!! (!)\n");
        return;
    }
    recordInfo* info = findRecord(key, 1);
    if (info->mainKey == key || info->ovfKey == key) {
        printf("(!) Rekord o podanym kluczu juz istnieje (!)\n");
        return;
    }

    info = findRecord(key, 0);
    char* record;
    int ptr = -1;

    if (info->mainKey == 0) { // puste miejsce
        main->modifyRecord(info->mainOffset, &key, data, &ptr);
        main->writeFile(info->mainOffset);
    }
    else if (info->mainOffset % main->recordsPerPage == 0 && info->mainKey > key) { // wpisz przed pierwszy na stronie
        ptr = ovf->fileSize();
        char tmp[32];
        record = main->getRecord(info->mainOffset);
        getData(tmp, record);
        ovf->appendRecord(getKey(record), tmp, getPointer(record));
        ovf->writeFile(ptr);
        main->modifyRecord(info->mainOffset, &key, data, &ptr);
        main->writeFile(info->mainOffset);
    }
    else if (info->mainPtr != -1) { // jest mniejszy niz pierwszy w lancuchu overflow
        if (info->mainPtr == info->ovfOffset && info->ovfKey > key) {
            ptr = ovf->fileSize();
            ovf->appendRecord(key, data, info->mainPtr);
            ovf->writeFile(ptr);
            main->modifyRecord(info->mainOffset, nullptr, nullptr, &ptr);
            main->writeFile(info->mainOffset);
        }
        else {
            char* ovfnext = ovf->getRecord(info->ovfPtr);
            int nextKey = getKey(ovfnext);
            int nextPtr = getPointer(ovfnext);

            if (nextKey > key ) { // pomiedzy znalezionym rekordem a nastepnym
                ptr = ovf->fileSize();
                ovf->appendRecord(key, data, info->ovfPtr);
                ovf->writeFile(ptr);
                ovf->modifyRecord(info->ovfOffset, nullptr, nullptr, &ptr);
                ovf->writeFile(info->ovfOffset);
            }
            else { // na koniec
                if (nextKey != -1) {
                    ptr = ovf->fileSize();
                    ovf->appendRecord(key, data, nextPtr);
                    ovf->writeFile(ptr);
                    ovf->modifyRecord(info->ovfPtr, nullptr, nullptr, &ptr);
                    ovf->writeFile(info->ovfPtr);
                }
                else {
                    ptr = ovf->fileSize();
                    ovf->appendRecord(key, data, nextPtr);
                    ovf->writeFile(ptr);
                    ovf->modifyRecord(info->ovfOffset, nullptr, nullptr, &ptr);
                    ovf->writeFile(info->ovfOffset);
                }
            }
        }
    }
    else { // zacznij lanuch overflow
        ptr = ovf->fileSize();
        ovf->appendRecord(key, data, -1);
        ovf->writeFile(ptr);
        main->modifyRecord(info->mainOffset, nullptr, nullptr, &ptr);
        main->writeFile(info->mainOffset);
    }


}

void ISQ::removeRecord(int key) {
    recordInfo* info;

    if (key <= 0) {
        printf("(!) Nie poprawny klucz !!! (!)\n");
        return;
    }
    info = findRecord(key, 1);
    if (info->mainKey != key && info->ovfKey != key) {
        printf("(!) Nie mozna usunac rekordu poniewaz podany klucz nie istnieje (!)\n");
        return;
    }


    if (info->mainKey == key) {
        if (info->mainPtr != -1) {
            char* ovfrecord = ovf->getRecord(info->mainPtr);
            char copy[32];
            char* record = main->getRecord(info->mainOffset);
            memcpy(copy, ovfrecord, 25);
            memcpy(record, copy, 25);
            main->writeFile(info->mainOffset);

            int tkey = 0, tptr = -1;
            ovf->modifyRecord(info->mainPtr, &tkey, nullptr, &tptr);
            ovf->writeFile(info->mainPtr);
        }
        else {
            int tkey = 0, tptr = -1;
            main->modifyRecord(info->mainOffset, &tkey, nullptr, &tptr);
            main->writeFile(info->mainOffset);
        }
    }
    else {
        char* ovfrecord = ovf->getRecord(info->ovfOffset);
        int ovfPtr = getPointer(ovfrecord);

        if (info->mainPtr == info->ovfOffset && info->ovfKey == key) {
            int tkey = 0, tptr = -1;
            main->modifyRecord(info->mainOffset, nullptr, nullptr, &ovfPtr);
            main->writeFile(info->mainOffset);
            ovf->modifyRecord(info->ovfOffset, &tkey, nullptr, &tptr);
            ovf->writeFile(info->ovfOffset);
        }
        else {
            char* ovfnext = ovf->getRecord(ovfPtr);
            int ovfnextptr = getPointer(ovfnext);

            int tkey = 0, tptr = -1;
            ovf->modifyRecord(ovfPtr, &tkey, nullptr, &tptr);
            ovf->writeFile(ovfPtr);
            ovf->modifyRecord(info->ovfOffset, nullptr, nullptr, &ovfnextptr);
            ovf->writeFile(info->ovfOffset);
        }
    }
}

void ISQ::updateRecord(char *data, int key, int newKey) {
    recordInfo* info2, *info;
    if (key <= 0) {
        printf("(!) Nie poprawny klucz !!! (!)\n");
        return;
    }
    info = findRecord(key, 1);
    if (info->mainKey != key && info->ovfKey != key) {
        printf("(!) Nie mozna zmienic rekordu poniewaz podany klucz nie istnieje (!)\n");
        return;
    }
    info2 = findRecord(newKey);
    if (info2->mainKey == newKey && info2->ovfKey == newKey) {
        printf("(!) Klucz o wartosci %i juz istnieje (!)\n", newKey);
        return;
    }

    int tkey = 0, tptr = -1;
    if (info->mainKey == key) {
        if (info->mainPtr != -1) {
            if (key != newKey) {
                char *ovfrecord = ovf->getRecord(info->mainPtr);
                char copy[32];
                char* record = main->getRecord(info->mainOffset);
                memcpy(copy, ovfrecord, 25);
                memcpy(record, copy, 25);
                main->writeFile(info->mainOffset);

                ovf->modifyRecord(info->mainPtr, &tkey, nullptr, &tptr);
                ovf->writeFile(info->mainPtr);
                addRecord(data, newKey);
            }
            else {
                main->modifyRecord(info->mainOffset, nullptr, data, nullptr);
                main->writeFile(info->mainOffset);
            }
        }
        else {
            main->modifyRecord(info->mainOffset, &tkey, nullptr, &tptr);
            main->writeFile(info->mainOffset);
            addRecord(data, newKey);
        }
    }
    else {
        char* ovfrecord = ovf->getRecord(info->ovfOffset);
        int ovfPointer = getPointer(ovfrecord);

        if (info->mainPtr == info->ovfOffset && info->ovfKey == key) {
            if (key != newKey) {
                main->modifyRecord(info->mainOffset, nullptr, nullptr, &ovfPointer);
                main->writeFile(info->mainOffset);
                ovf->modifyRecord(info->ovfOffset, &tkey, nullptr, &tptr);
                ovf->writeFile(info->ovfOffset);
                addRecord(data, newKey);
            }
            else {
                ovf->modifyRecord(info->ovfOffset, nullptr, data, nullptr);
            }
        }
        else {
            char* ovfnext = ovf->getRecord(ovfPointer);
            int ovfnextptr = getPointer(ovfnext);
            if (key != newKey) {
                ovf->modifyRecord(ovfPointer, &tkey, nullptr, &tptr);
                ovf->writeFile(ovfPointer);
                ovf->modifyRecord(info->ovfOffset, nullptr, nullptr, &ovfnextptr);
                ovf->writeFile(info->ovfOffset);
                addRecord(data, newKey);
            }
            else {
                ovf->modifyRecord(ovfPointer, nullptr, data, nullptr);
            }
        }
    }
}

char* ISQ::getNextRecord() {
    char* record;
    if (lastSrc == 0) {
        record = main->getRecord(lastMainRecord);
        lastPage = lastMainRecord / main->recordsPerPage;

        if (isRecordEmpty(record) == 1) {
            if (record == nullptr)
                return nullptr;
            else {
                lastMainRecord++;
                return getNextRecord();
            }
        }
        else {
            int pointer = getPointer(record);
            if (pointer != -1) {
                lastSrc = 1;
                lastOvfRecord = pointer;
                lastMainRecord++;
            }
            else {
                lastMainRecord++;
            }
        }
    }
    else {
        record = ovf->getRecord(lastOvfRecord);
        int pointer;
        if (record != nullptr) {
            pointer = getPointer(record);
            if (pointer != -1)
                lastOvfRecord = pointer;
            else {
                lastSrc = 0;
            }
        }
        else {
            lastSrc = 0;
        }
    }

    return record;
}

void ISQ::clearFile(const char *name) {
    remove(name);
    FILE * file = fopen(name, "w");
    fclose(file);
}

int ISQ::autoRebuild() {
    if (main->fileSize() * threshold < ovf->fileSize())
        return 1;
    return 0;
}

void ISQ::rebuild() {

    lastSrc = lastMainRecord = lastOvfRecord = 0;
    Page* tmain = new Page("tmain.txt", 25, main->recordsPerPage);
    Page* tidx = new Page("tindex.txt", 17, index->recordsPerPage);
    main->resetOperations();

    clearFile("tmain.txt");
    clearFile("tindex.txt");

    char* record;
    int addIdx = 1, count = 0, page = 0, indexes = 0, key;
    char temp[32];
    while (true) {

        record = getNextRecord();
        key = getKey(record);

        if (record == nullptr)
            break;

        if (addIdx == 1) {
            int indexKey;
            count == 0 ? indexKey = 0 : indexKey = key;

            tidx->createIndex(indexes, indexKey);
            addIdx = 0;
            indexes++;
            if (indexes % tidx->recordsPerPage == 0)
                tidx->writeFile(indexes-1);
        }

        getData(temp, record);
        tmain->appendRecord(key, temp, -1);
        count++;

        fillUpBlanks(tmain, count, 0);

        if (count % tmain->recordsPerPage == 0) {
            tmain->writeFile(count - 1);
            addIdx = 1;
        }

    }
    fillUpBlanks(tmain, count, 1);

    tidx->writeFile(indexes-1);
    tmain->writeFile(count-1);

    clearFile(ovf->getFileName());
    remove(main->getFileName());
    remove(index->getFileName());
    rename("tmain.txt", main->getFileName());
    rename("tindex.txt", index->getFileName());

    index->reset();
    main->reset();
    ovf->reset();

    this->rebuilds ++;
    this->writes = tmain->getWrites() + tidx->getWrites();
}

void ISQ::fillUpBlanks(Page* page, int &count, int end) {
    if (end == 1) {
        if (count % page->recordsPerPage < ceil(alfa * page->recordsPerPage) && count % page->recordsPerPage != 0) {
            char empty[9] = "00000000";
            for (int i = count % page->recordsPerPage; i < page->recordsPerPage; i++) {
                page->appendRecord(0, empty, -1);
                count++;
            }
        }
    }
    else {
        if (count % page->recordsPerPage == ceil(alfa * page->recordsPerPage)) {
            char empty[9] = "00000000";
            for (int i = 0; i<page->recordsPerPage - ceil(alfa * page->recordsPerPage); i++) {
                page->appendRecord(0, empty, -1);
                count++;
            }
        }
    }
}

void ISQ::printIndex() {
    char* record;
    int offset = 0;
    printf("(-) Index (-)\n");
    while (true) {
        record = index->getRecord(offset);

        if (record == nullptr)
            break;

        if (getKey(record) == -1)
            break;

        offset++;

        printf("  Klucz: %i Strona: %i\n", getKey(record), getPageNo(record));
    }
}

void ISQ::printFile() {
    lastSrc = lastMainRecord = lastOvfRecord = lastPage = 0;

    main->readFile(0); index->readFile(0); ovf->readFile(0);
    printIndex();
    printf("\n(-) Zawartość pliku (-)\n");

    char* record;
    char buff[32];
    int printed, delay = 0, prevPointer;
    record = getNextRecord();
    for (int i=0; i<ceil((double) main->fileSize()/main->recordsPerPage); i++) {
        printed = 0;
        printf("  (-) Strona nr: %i (-)\n", i+1);
        while (true) {

            if (record == nullptr)
                break;

            if (i != lastPage) {
                break;
            }

            getData(buff, record);
            printf("    Klucz: %i Wartosc: %s Pointer: %i", getKey(record), buff, getPointer(record));
            if (delay == 1)
                printf("    <-  Overflow address: %i", prevPointer);

            if (lastSrc == 1) {
                delay = 1;
                prevPointer = getPointer(record);
            }
            else {
                delay = 0;
                printed++;
            }
            printf("\n");

            record = getNextRecord();
        }
        printf("    Puste rekordy: %i\n\n", std::max(main->recordsPerPage - printed, 0));
    }
}

void ISQ::resetForGeneration() {
    clearFile(ovf->getFileName());
    clearFile(index->getFileName());
    clearFile(main->getFileName());
    main->reset();
    index->reset();

    index->createIndex(0, 0);

    char data[9] = "00000000";
    for (int i=0; i<main->recordsPerPage; i++) {
        main->appendRecord(0, data, -1);
    }

    index->writeFile(0);
    main->writeFile(0);
}

int ISQ::getWrites() {
    return writes;
}

void ISQ::resetOperations() {
    reads = 0;
    writes = 0;
    main->resetOperations();
    ovf->resetOperations();
    index->resetOperations();
}

void ISQ::printOperations() {
    this->reads += main->getReads() + ovf->getReads() + index->getReads();
    this->writes += main->getWrites() + ovf->getWrites() + index->getWrites();
    printf("(x) Disk reads: %i  Disk writes: %i (x)\n", this->reads, this->writes);
}
