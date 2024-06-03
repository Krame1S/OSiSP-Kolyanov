#define _POSIX_C_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#define INITIAL_QUEUE_SIZE 10
#define MAX_THREADS 50

typedef struct _message {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    char data[259];
} Message;

typedef struct _ring {
    Message *messages;
    size_t head, tail, count, capacity;
    size_t add, get;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} Ring;

Ring ring;
pthread_t producer_threads[MAX_THREADS];
pthread_t consumer_threads[MAX_THREADS];
size_t producer_count = 0, consumer_count = 0;
volatile sig_atomic_t running = 1;

void handle_signal(int signo) {
    if (signo == SIGINT) {
        running = 0;
    }
}

void *producer(void *arg) {
    while (running) {
        Message message;
        message.type = rand() % 256;
        message.size = rand() % 257;
        for (int i = 0; i < message.size; i++) {
            message.data[i] = 'A' + (rand() % 26);
        }
        message.data[message.size] = '\0';
        message.hash = 0;

        pthread_mutex_lock(&ring.mutex);

        while (ring.count == ring.capacity && running) {
            pthread_cond_wait(&ring.not_full, &ring.mutex);
        }

        if (!running) {
            pthread_mutex_unlock(&ring.mutex);
            break;
        }

        ring.messages[ring.head] = message;
        ring.head = (ring.head + 1) % ring.capacity;
        ring.count++;
        ring.add++;

        printf("Produced message: %s\n", message.data);

        pthread_cond_signal(&ring.not_empty);
        pthread_mutex_unlock(&ring.mutex);

        sleep(2);
    }
    return NULL;
}

void *consumer(void *arg) {
    while (running) {
        pthread_mutex_lock(&ring.mutex);

        while (ring.count == 0 && running) {
            pthread_cond_wait(&ring.not_empty, &ring.mutex);
        }

        if (!running) {
            pthread_mutex_unlock(&ring.mutex);
            break;
        }

        Message message = ring.messages[ring.tail];
        ring.tail = (ring.tail + 1) % ring.capacity;
        ring.count--;
        ring.get++;

        printf("Consumed message: %s\n", message.data);

        pthread_cond_signal(&ring.not_full);
        pthread_mutex_unlock(&ring.mutex);

        sleep(2);
    }
    return NULL;
}

void resize_ring(size_t new_capacity) {
    pthread_mutex_lock(&ring.mutex);

    if (new_capacity < ring.count) {
        printf("Cannot resize: new capacity %ld is less than current count %ld\n", new_capacity, ring.count);
        pthread_mutex_unlock(&ring.mutex);
        return;
    }

    Message *new_messages = malloc(new_capacity * sizeof(Message));
    for (size_t i = 0; i < ring.count; i++) {
        new_messages[i] = ring.messages[(ring.tail + i) % ring.capacity];
    }
    free(ring.messages);
    ring.messages = new_messages;
    ring.head = ring.count;
    ring.tail = 0;
    ring.capacity = new_capacity;

    pthread_cond_broadcast(&ring.not_full); // Уведомить производителей о наличии свободного места

    pthread_mutex_unlock(&ring.mutex);
}

void create_producer() {
    if (producer_count < MAX_THREADS) {
        pthread_create(&producer_threads[producer_count++], NULL, producer, NULL);
        printf("Created producer, total producers: %ld\n", producer_count);
    } else {
        printf("Maximum number of producers reached.\n");
    }
}

void create_consumer() {
    if (consumer_count < MAX_THREADS) {
        pthread_create(&consumer_threads[consumer_count++], NULL, consumer, NULL);
        printf("Created consumer, total consumers: %ld\n", consumer_count);
    } else {
        printf("Maximum number of consumers reached.\n");
    }
}

void terminate_producer() {
    if (producer_count > 0) {
        pthread_cancel(producer_threads[--producer_count]);
        printf("Terminated producer, total producers: %ld\n", producer_count);
    } else {
        printf("No producers to terminate.\n");
    }
}

void terminate_consumer() {
    if (consumer_count > 0) {
        pthread_cancel(consumer_threads[--consumer_count]);
        printf("Terminated consumer, total consumers: %ld\n", consumer_count);
    } else {
        printf("No consumers to terminate.\n");
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, handle_signal);

    ring.capacity = INITIAL_QUEUE_SIZE;
    ring.messages = malloc(ring.capacity * sizeof(Message));
    ring.head = ring.tail = ring.count = ring.add = ring.get = 0;
    pthread_mutex_init(&ring.mutex, NULL);
    pthread_cond_init(&ring.not_full, NULL);
    pthread_cond_init(&ring.not_empty, NULL);

    printf("Commands:\n"
           "-p -> Terminate a producer\n"
           "-c -> Terminate a consumer\n"
           "p -> Create a new producer\n"
           "c -> Create a new consumer\n"
           "+ -> Increase queue size\n"
           "- -> Decrease queue size\n"
           "s -> Display statistics\n"
           "q -> Exit the program\n");

    char input[3];
    while (running) {
        if (scanf("%s", input) == 1) {
            switch (input[0]) {
                case 'p':
                    create_producer();
                    break;
                case 'c':
                    create_consumer();
                    break;
                case '-':
                    if (input[1] == 'p') {
                        terminate_producer();
                    } else if (input[1] == 'c') {
                        terminate_consumer();
                    } else {
                        if (ring.capacity > 1) {
                            resize_ring(ring.capacity - 1);
                            printf("Decreased queue size to %ld\n", ring.capacity);
                        } else {
                            printf("Queue size cannot be less than 1\n");
                        }
                    }
                    break;
                case '+':
                    resize_ring(ring.capacity + 1);
                    printf("Increased queue size to %ld\n", ring.capacity);
                    break;
                case 's':
                    pthread_mutex_lock(&ring.mutex);
                    printf("---Statistics---\n");
                    printf("Maximum count: %ld\n", ring.capacity);
                    printf("Current count: %ld\n", ring.count);
                    printf("Messages added: %ld\n", ring.add);
                    printf("Messages retrieved: %ld\n", ring.get);
                    printf("Number of producers: %ld\n", producer_count);
                    printf("Number of consumers: %ld\n", consumer_count);
                    pthread_mutex_unlock(&ring.mutex);
                    break;
                case 'q':
                    running = 0;
                    pthread_cond_broadcast(&ring.not_full);
                    pthread_cond_broadcast(&ring.not_empty);
                    break;
                default:
                    printf("Invalid command. Please try again.\n");
                    break;
            }
        }
    }

    for (size_t i = 0; i < producer_count; i++) {
        pthread_cancel(producer_threads[i]);
    }
    for (size_t i = 0; i < consumer_count; i++) {
        pthread_cancel(consumer_threads[i]);
    }

    free(ring.messages);
    pthread_mutex_destroy(&ring.mutex);
    pthread_cond_destroy(&ring.not_full);
    pthread_cond_destroy(&ring.not_empty);

    return 0;
}
