#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

struct index_s {
    double time_mark;
    uint64_t recno;
};

struct index_hdr_s {
    uint64_t records;
    struct index_s idx[];
};

int is_sorted(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return -1;
    }

    size_t file_size = st.st_size;
    struct index_hdr_s *header = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (header == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

    size_t num_records = header->records;
    struct index_s *index_array = header->idx;

    for (size_t i = 1; i < num_records; ++i) {
        if (index_array[i - 1].time_mark > index_array[i].time_mark) {
            munmap(header, file_size);
            close(fd);
            return 0; // File is not sorted
        }
    }

    munmap(header, file_size);
    close(fd);
    return 1; // File is sorted
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    int result = is_sorted(filename);
    if (result == -1) {
        fprintf(stderr, "Error occurred during sorting check.\n");
        return EXIT_FAILURE;
    } else if (result == 0) {
        printf("The file is NOT sorted.\n");
        return EXIT_FAILURE;
    } else {
        printf("The file is sorted.\n");
        return EXIT_SUCCESS;
    }
}
