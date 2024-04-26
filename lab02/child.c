#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("I am %s (PID: %d, PPID: %d)\n", argv[0], getpid(), getppid());

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Failed to open the environment file");
        exit(EXIT_FAILURE);
    }

    char var_name[100];
    while (fgets(var_name, sizeof(var_name), file)) {
        var_name[strcspn(var_name, "\n")] = 0;
        char *var_value = getenv(var_name);
        if (var_value)
            printf("%s=%s\n", var_name, var_value);
    }
    fclose(file);

    printf("Waiting for input ('+', '*', '&', 'q'): ");

    return 0;
}
