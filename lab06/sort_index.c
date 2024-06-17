#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

struct index_s {
    double time_mark;
    uint64_t recno;
};

struct index_hdr_s {
    uint64_t records;
    struct index_s idx[];
};

typedef struct {
    struct index_s *start;
    size_t size;
} thread_arg_t;

pthread_barrier_t barrier;
pthread_mutex_t mutex;
size_t block_index;
int blocks;
int memsize;
size_t block_size;
int threads;
struct index_s *global_index_array;

int compare(const void *a, const void *b) {
    const struct index_s *ia = (const struct index_s *)a;
    const struct index_s *ib = (const struct index_s *)b;
    if (ia->time_mark < ib->time_mark) return -1;
    if (ia->time_mark > ib->time_mark) return 1;
    return 0;
}

void *sort_blocks(void *arg) {
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    size_t size = thread_arg->size;
    struct index_s *array = thread_arg->start;

    printf("Thread %lu started sorting\n", pthread_self());

    // Wait for all threads to be ready
    pthread_barrier_wait(&barrier);

    while (1) {
        // Sort the assigned block
        printf("Thread %lu sorting block of size %zu\n", pthread_self(), size);
        qsort(array, size, sizeof(struct index_s), compare);

        // Get the next block to sort
        pthread_mutex_lock(&mutex);
        if (block_index >= (size_t)blocks) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        size_t current_block = block_index++;
        pthread_mutex_unlock(&mutex);

        array = global_index_array + current_block * block_size;
        size = (current_block == (size_t)(blocks - 1)) ? (block_size + (global_index_array + (blocks * block_size) - array)) : block_size;
    }

    // Wait for all threads to finish sorting
    pthread_barrier_wait(&barrier);

    printf("Thread %lu finished sorting\n", pthread_self());
    return NULL;
}

void merge(struct index_s *array, size_t size, int num_blocks) {
    while (num_blocks > 1) {
        int half = num_blocks / 2;
        for (int i = 0; i < half; ++i) {
            size_t block_size = size * 2;
            struct index_s *temp = malloc(block_size * sizeof(struct index_s));
            size_t j = 0, k = 0;
            struct index_s *left = array + i * block_size;
            struct index_s *right = left + size;

            for (size_t l = 0; l < block_size; ++l) {
                if (j < size && (k >= size || left[j].time_mark <= right[k].time_mark)) {
                    temp[l] = left[j++];
                } else {
                    temp[l] = right[k++];
                }
            }
            memcpy(left, temp, block_size * sizeof(struct index_s));
            free(temp);
        }
        size *= 2;
        num_blocks /= 2;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s memsize blocks threads filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    memsize = atoi(argv[1]);
    blocks = atoi(argv[2]);
    threads = atoi(argv[3]);
    char *filename = argv[4];

    if (blocks <= threads) {
        fprintf(stderr, "The number of blocks must be greater than the number of threads.\n");
        return EXIT_FAILURE;
    }

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    size_t file_size = st.st_size;
    struct index_hdr_s *header = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (header == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    size_t num_records = header->records;
    global_index_array = header->idx;

    block_size = num_records / blocks;
    pthread_t tid[threads];
    thread_arg_t thread_args[threads];

    pthread_barrier_init(&barrier, NULL, threads);
    pthread_mutex_init(&mutex, NULL);
    block_index = threads; // Starting index for blocks to be processed

    for (int i = 0; i < threads; ++i) {
        thread_args[i].start = global_index_array + i * block_size;
        thread_args[i].size = (i == threads - 1) ? (block_size + num_records % block_size) : block_size;
        pthread_create(&tid[i], NULL, sort_blocks, &thread_args[i]);
    }

    for (int i = 0; i < threads; ++i) {
        pthread_join(tid[i], NULL);
    }

    merge(global_index_array, block_size, blocks);

    munmap(header, file_size);
    close(fd);

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}
