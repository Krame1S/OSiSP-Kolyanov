#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

extern char **environ;

int env_compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void sort_and_print_env(char **env) {
    printf("Environment Variables in LC_COLLATE=C order:\n");
    int count;
    for (count = 0; env[count] != NULL; count++);

    char **sorted_env = malloc(sizeof(char *) * count);
    for (int i = 0; i < count; i++) {
        sorted_env[i] = env[i];
    }

    qsort(sorted_env, count, sizeof(char *), env_compare);

    for (int i = 0; i < count; i++) {
        printf("%s\n", sorted_env[i]);
    }

    free(sorted_env);
}

void create_reduced_env(char *filename, char ***new_envp) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the environment file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int num_vars = 0;
    while (fgets(line, sizeof(line), file)) {
        num_vars++;
    }

    *new_envp = malloc((num_vars + 1) * sizeof(char *));
    fseek(file, 0, SEEK_SET);

    int i = 0;
    while(fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char *value = getenv(line);
        if (value) {
            char *env_var = malloc(strlen(line) + strlen(value) + 2);
            sprintf(env_var, "%s=%s", line, value);
            (*new_envp)[i++] = env_var;
        }
    }
    (*new_envp)[i] = NULL;

    fclose(file);
}

int main(int argc, char *argv[], char *envp[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <env_file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sort_and_print_env(envp);

    int child_count = 0;
    char child_env_path[256];
    strcpy(child_env_path, getenv("CHILD_PATH"));

    printf("Waiting for input ('+', '*', '&', 'q'): ");

    while (1) {
        char cmd = getchar();
        getchar();

        if (cmd == 'q') {
            printf("Exiting...\n");
            break;
        }

        child_count++;

        pid_t pid = fork();
        if (pid == 0) {
            char child_name[256];
            snprintf(child_name, sizeof(child_name), "child_%02d", child_count - 1);

            char *args[] = {child_name, "environment.txt", NULL};

            char *filename = "environment.txt";
            char **new_envp;
            if (cmd == '+') {
                create_reduced_env(filename, &new_envp);
            } else if (cmd == '*') {
                new_envp = envp;
            } else if (cmd == '&') {
                new_envp = environ;
            }

            execve(child_env_path, args, new_envp);
            exit(EXIT_SUCCESS);
        }
    }


    return 0;
}
