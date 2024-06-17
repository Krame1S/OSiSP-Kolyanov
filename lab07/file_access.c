#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define FILE_NAME "records.dat"

typedef struct record_s {
    char name[80];
    char address[80];
    uint8_t semester;
} Record;

void lock_record(int fd, int rec_no, short lock_type) {
    struct flock lock;
    lock.l_type = lock_type;
    lock.l_whence = SEEK_SET;
    lock.l_start = rec_no * sizeof(Record);
    lock.l_len = sizeof(Record);
    while (fcntl(fd, F_SETLKW, &lock) == -1) {
        if (errno != EINTR) {
            perror("fcntl");
            exit(EXIT_FAILURE);
        }
    }
}

void unlock_record(int fd, int rec_no) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = rec_no * sizeof(Record);
    lock.l_len = sizeof(Record);
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

void list_records(int fd) {
    Record rec;
    int rec_no = 0;
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &rec, sizeof(rec)) > 0) {
        printf("Record %d: Name: %s, Address: %s, Semester: %d\n", rec_no++, rec.name, rec.address, rec.semester);
    }
}

Record get_record(int fd, int rec_no) {
    Record rec;
    lseek(fd, rec_no * sizeof(rec), SEEK_SET);
    read(fd, &rec, sizeof(rec));
    return rec;
}

void put_record(int fd, int rec_no, Record rec) {
    lseek(fd, rec_no * sizeof(rec), SEEK_SET);
    write(fd, &rec, sizeof(rec));
}

int main() {
    int fd = open(FILE_NAME, O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char command[10];
    int rec_no;
    Record rec;

    while (1) {
        printf("Enter command (LST, GET, PUT, MOD, EXIT): ");
        scanf("%s", command);

        if (strcmp(command, "LST") == 0) {
            list_records(fd);
        } else if (strcmp(command, "GET") == 0) {
            printf("Enter record number: ");
            scanf("%d", &rec_no);

            lock_record(fd, rec_no, F_RDLCK);
            rec = get_record(fd, rec_no);

            printf("Record %d: Name: %s, Address: %s, Semester: %d\n", rec_no, rec.name, rec.address, rec.semester);
            unlock_record(fd, rec_no);
        } else if (strcmp(command, "MOD") == 0) {
            printf("Enter record number: ");
            scanf("%d", &rec_no);

            lock_record(fd, rec_no, F_WRLCK);
            Record old_rec = get_record(fd, rec_no);
            Record new_rec = old_rec;
            printf("Enter new name (or - to keep): ");
            scanf("%s", new_rec.name);
            printf("Enter new address (or - to keep): ");
            scanf("%s", new_rec.address);
            printf("Enter new semester (or -1 to keep): ");
            int new_semester;
            scanf("%d", &new_semester);
            if (strcmp(new_rec.name, "-") == 0) {
                strcpy(new_rec.name, old_rec.name);
            }
            if (strcmp(new_rec.address, "-") == 0) {
                strcpy(new_rec.address, old_rec.address);
            }
            if (new_semester != -1) {
                new_rec.semester = new_semester;
            }

            put_record(fd, rec_no, new_rec);
            unlock_record(fd, rec_no);
            
        } else if (strcmp(command, "PUT") == 0) {
            lock_record(fd, rec_no, F_WRLCK);
            put_record(fd, rec_no, rec);
            unlock_record(fd, rec_no);
        } else if (strcmp(command, "EXIT") == 0) {
            break;
        } else {
            printf("Unknown command.\n");
        }
    }

    close(fd);
    return 0;
}
