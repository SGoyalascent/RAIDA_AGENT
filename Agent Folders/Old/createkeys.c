#include <stdio.h>

#define KEYS_SIZE   1600000

void CreateKeys() {

    char key_byte;
    FILE *inp;
    char path[256];

    inp = fopen(path, "wb");
    if(inp == NULL) {
        printf("Keys.bin file cannot be opened\n");
        return;
    }

    for(int i=1; i <= 160000; i++) {

        key_byte = rand();
        fwrite(&key_byte, 1, 1, inp);
    }

    fclose(inp);

}

void CreateKeys() {

    char keys[KEYS_SIZE];

    for(int i=0; i < KEYS_SIZE; i++) {
        keys[i] = rand();
    }

    FILE *inp;
    char path[256];

    inp = fopen(path, "wb");
    if(inp == NULL) {
        printf("Keys.bin file cannot be opened\n");
        return;
    }

    fwrite(keys, 1, KEYS_SIZE, inp);

    fclose(inp);

}

