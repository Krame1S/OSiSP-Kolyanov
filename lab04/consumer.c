#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <unistd.h>

typedef struct _message {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    char data[259];
} Message;

typedef struct _ring {
    Message messages[10];
    size_t head, tail, count, add, get;
} Ring;

bool flag = true;

void sig1_handler(int signo) {
    if (signo != SIGUSR1)
        return;

    flag = false;
}

int main(int argc, char** argv) {
    signal(SIGUSR1, sig1_handler);

    sem_t* consumer_sem = sem_open("/semconsumer", 0);
    if (consumer_sem == SEM_FAILED) {
        perror("Consumer sem_open");
        return 1;
    };
    sem_t* mono_sem = sem_open("/semmono", 0);
    if (mono_sem == SEM_FAILED) {
        perror("Consumer sem_open");
        return 1;
    };

    int file_descriptor;
    Ring* ring;

    if ((file_descriptor = shm_open("/ringmem", O_RDWR, S_IRUSR | S_IWUSR)) == -1)
        perror("shm_open");
    if ((ring = mmap(NULL, sizeof(Ring), PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0)) == (void*)-1)
        perror("Mmap");

    while (flag) {
        sem_wait(consumer_sem);
        sem_wait(mono_sem);

        if (ring->count > 0) {
            printf("%s Received message: %s\n\n", argv[0], ring->messages[ring->tail].data);    
            ring->tail = ring->tail == 9 ? 0 : ring->tail + 1;

            ring->get++;
            ring->count--;
        }

        if (sem_post(mono_sem) != 0)
            perror("Producer semaphore unlock error");
        if (sem_post(consumer_sem) != 0)
            perror("Producer semaphore unlock error");

        sleep(2);
    }

    munmap(ring, sizeof(Ring));

    if (sem_close(consumer_sem) == -1)
        perror("Consumer semaphore close");
    if (sem_close(mono_sem) == -1)
        perror("Consumer semaphore close");
    return 0;
}
