#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FILE_NAME "records.dat"

typedef struct record_s {
    char name[80];
    char address[80];
    uint8_t semester;
} Record;

int main() {
    FILE *file = fopen(FILE_NAME, "wb");
    if (!file) {
        perror("fopen");
        return 1;
    }

    Record records[10];
    for (int i = 0; i < 10; i++) {
        snprintf(records[i].name, sizeof(records[i].name), "Student %d", i + 1);
        snprintf(records[i].address, sizeof(records[i].address), "Address %d", i + 1);
        records[i].semester = (i % 8) + 1;
        fwrite(&records[i], sizeof(Record), 1, file);
    }

    fclose(file);
    return 0;
}
