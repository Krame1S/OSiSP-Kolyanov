#define _POSIX_C_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <signal.h>
#include <fcntl.h>

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

int main(int argc, char** argv) {
    sem_t *producer_sem = sem_open("/semproducer", O_CREAT, 0644, 1),
          *consumer_sem = sem_open("/semconsumer", O_CREAT, 0644, 1),
          *mono_sem = sem_open("/semmono", O_CREAT, 0644, 1);
          
    if (producer_sem == SEM_FAILED || consumer_sem == SEM_FAILED || mono_sem == SEM_FAILED) {
        perror("Semaphore create error");
        sem_close(producer_sem);
        sem_close(consumer_sem);
        sem_close(mono_sem);
        return 1;
    }

    int fd;
    if ((fd = shm_open("/ringmem", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
        perror("shm_open");

    if (ftruncate(fd, sizeof(Ring)) == -1)
        perror("Ftruncate");

    Ring* ring;
    if ((ring = mmap(NULL, sizeof(Ring), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
        perror("mmap");

    ring->count = 0;
    ring->add = 0;
    ring->get = 0;
    ring->head = ring->tail = 0;

    size_t producer_count = 0,
        consumer_count = 0;
    pid_t producer_pids[50] = {(pid_t)0};
    pid_t consumer_pids[50] = {(pid_t)0};

    char* child_argv[2];
    char input[3] = {'\0'};

    puts("Commands:\n"
         "-p/P -> Terminate a producer\n"
         "-c/C -> Terminate a consumer\n"
         "p/P -> Create a new producer\n"
         "c/C -> Create a new consumer\n"
         "s/S -> Display statistics\n"
         "q/Q -> Exit the program\n");
    while(1) {
        scanf("%s", input);

        switch (input[0]) {
        case '-':
            if ((input[1] == 'p' || input[1] == 'P') && producer_count > 0)
                kill(producer_pids[producer_count-- - 1], SIGUSR1);
            else if ((input[1] == 'c' || input[1] == 'C') && consumer_count > 0)
                kill(consumer_pids[consumer_count-- - 1], SIGUSR1);
        break;
        
        case 'P':
        case 'p':
            producer_pids[producer_count++] = fork();
            if (producer_pids[producer_count - 1] == -1 )
               perror("Producer fork");

            else if (producer_pids[producer_count - 1] == 0) {
                child_argv[0] = (char*)malloc(sizeof(char) * 9);
                child_argv[1] = (char*)0;
                sprintf(child_argv[0], "Prod_%ld", producer_count); 
                if (execve("./producer", child_argv, NULL) == -1) {
                    perror("Producer execve");
                    free(child_argv[0]);
                    return 1;
                }
            }
            break;
        
        case 'C':
        case 'c':
            consumer_pids[consumer_count++] = fork();
            if(consumer_pids[consumer_count - 1] == -1 )
               perror("Consumer fork");
        
            else if(consumer_pids[consumer_count - 1] == 0) {
                child_argv[0] = (char*)malloc(sizeof(char) * 13);
                child_argv[1] = (char*)0;
                sprintf(child_argv[0], "Consumer_%ld", consumer_count); 

                if (execve("./consumer", child_argv, (char**)0) == -1) {
                    perror("Consumer execve");
                    free(child_argv[0]);
                    return 1;
                }
            }
            break;
        
        case 'S':
        case 's':
            sem_wait(mono_sem);
        printf("---Statistics---\n"
            "Maximum count: 10\n"
            "Current count: %ld\n"
            "Messages added: %ld\n"
            "Messages retrieved: %ld\n"
            "Number of producers: %ld\n"
            "Number of consumers: %ld\n",
            ring->count, ring->add, ring->get, producer_count, consumer_count);
            sem_post(mono_sem);
        break;

        case 'Q':
        case 'q':
            while (kill(producer_pids[producer_count-- - 1], SIGUSR1) == 0);
            while (kill(consumer_pids[consumer_count-- - 1], SIGUSR1) == 0);
               
            munmap(ring, sizeof(Ring));
            close(fd);
            shm_unlink("/ringmem");

            if (sem_close(producer_sem) == -1) {
                perror("Sem_close producerSem");
                return 1;
            }
            if (sem_close(consumer_sem) == -1) {
                perror("Sem_close consumerSem");
                return 1;
            }
            if (sem_close(mono_sem) == -1) {
                perror("Sem_close monoSem");
                return 1;
            }
            
            sem_unlink("/semmono");
            sem_unlink("/semproducer");
            sem_unlink("/semconsumer");
            
            return 0;
            break;

        default:
            puts("Invalid command. Please try again.");
            break;
        }
    }

    return 0;
}
