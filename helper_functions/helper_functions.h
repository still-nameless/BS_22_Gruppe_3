#ifndef BS_22_GRUPPE_3_HELPER_FUNCTIONS_H
#define BS_22_GRUPPE_3_HELPER_FUNCTIONS_H

struct Statement{
    char command[5];
    char key[56];
    char value[1024];
    int commandExists;
    int keyExists;
    int valueExists;
};

struct Statement processInput(char* input);
int* createSharedMemoryForTransactions();
void replaceCharactersInString(char* input, char old, char new);

#endif //BS_22_GRUPPE_3_HELPER_FUNCTIONS_H
