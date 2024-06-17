#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

struct index_s {
    double time_mark;
    uint64_t recno;
};

struct index_hdr_s {
    uint64_t records;
    struct index_s idx[];
};

double generate_time_mark() {
    int days = 15020 + rand() % (time(NULL) / (24 * 3600) - 15020);
    double fraction = (double)(rand() % 1000000) / 1000000;
    return days + fraction;
}

void generate_index_file(const char *filename, size_t num_records) {
    int fd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    size_t file_size = sizeof(struct index_hdr_s) + num_records * sizeof(struct index_s);
    struct index_hdr_s *header = malloc(file_size);
    if (!header) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    header->records = num_records;
    for (size_t i = 0; i < num_records; ++i) {
        header->idx[i].time_mark = generate_time_mark();
        header->idx[i].recno = i + 1;
    }

    if (write(fd, header, file_size) != file_size) {
        perror("write");
        free(header);
        close(fd);
        exit(EXIT_FAILURE);
    }

    free(header);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <num_records>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    size_t num_records = strtoull(argv[2], NULL, 10);

    srand(time(NULL));
    generate_index_file(filename, num_records);

    return 0;
}
