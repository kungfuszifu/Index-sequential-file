#include <iostream>
#include "ISQ.h"

void generateFile(ISQ* isq, int keys) {
    isq->resetForGeneration();

    char data[9] = "00000000";
    int key;
    for (int i=0; i<keys; i++) {
        key = rand() % 0xFFFFFF;
        isq->addRecord(data, key);

        if (isq->autoRebuild() == 1) {
            isq->rebuild();
        }
    }
}

int main() {

    srand(time(nullptr));
    ISQ isq;
    char input[32] = "GDA00100";
    char buffor[128];

    setbuf(stdout, nullptr);
    while (true) {
        printf("Podaj co chcesz zrobic: (add|upd|del|print|rebuild|test|gen)\n");
        scanf("%s", input);

        if (strcmp(input, "q") == 0)
            break;
        else if (strcmp(input, "add") == 0) {
            printf("Podaj klucz\n");
            int key;
            scanf("%i", &key);
            printf("Podaj wartosc rekordu ( AA-00000 | AAA00000 )\n");
            scanf("%s", input);
            input[9] = '\0';
            isq.addRecord(input, key);
            isq.printOperations();
            isq.resetOperations();
        }
        else if (strcmp(input, "upd") == 0) {
            int key, newKey;
            printf("Podaj klucz rekordu ktory chesz zmienic\n");
            scanf("%i", &key);
            printf("Podaj nowa wartosc klucza\n");
            scanf("%i", &newKey);
            printf("Podaj nowa wartosc rekordu\n");
            scanf("%s", input);
            isq.updateRecord(input, key, newKey);
            isq.printOperations();
            isq.resetOperations();
        }
        else if (strcmp(input, "del") == 0) {
            int key;
            printf("Podaj klucz rekordu ktory chesz usunac\n");
            scanf("%i", &key);
            isq.removeRecord(key);
            isq.printOperations();
            isq.resetOperations();
        }
        else if (strcmp(input, "print") == 0) {
            isq.printFile();
            isq.resetOperations();
        }
        else if (strcmp(input, "rebuild") == 0) {
            isq.rebuild();
            isq.printOperations();
            isq.resetOperations();
        }
        else if (strcmp(input, "test") == 0) {

            ISQ* testisq;
            int write;
            double alfa, thr;
            FILE* file = fopen("wyniki.txt", "w");

            for (int j=0; j<=2; j++) {
                alfa = 0.5 + 0.25 * j;
                for (int w = 0; w<=2; w++) {
                    thr = 0.25 * (1 + w);

                    testisq = new ISQ(alfa, thr);
                    generateFile(testisq, 10000);

                    int ops = testisq->rebuilds;
                    snprintf(buffor, 128, "%d,%lF,%lF,%d,", 10000, alfa, thr, ops);

                    testisq->resetOperations();
                    testisq->rebuild();
                    write = testisq->getWrites();
                    snprintf(buffor+strlen(buffor), 128, "%d\n", write);
                    fwrite(buffor, sizeof(char), strlen(buffor), file);
                }
            }

            fclose(file);
        }
        else if (strcmp(input, "gen") == 0) {
            int ilosc;
            printf("Podaj ilosc rekordow do wygenerowania\n");
            scanf("%i", &ilosc);
            generateFile(&isq, ilosc);
            isq.resetOperations();
        }
        else {
            printf("(!) Invalid Input (!)\n");
            return 0;
        }


        if (isq.autoRebuild() == 1 ) {
            isq.rebuild();
            printf("(!) File has been rebuilt (!)\n");
            isq.printOperations();
            isq.resetOperations();
        }
    }

    return 0;
}
