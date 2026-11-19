#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>

static volatile sig_atomic_t need_rotate = 0;

static void sighup_handler(int sig) {
    need_rotate = 1;
}



static int get_max_log_index(const char *prefix) {
    DIR *dir = opendir(".");
    if (dir == NULL) {
        return 0;
    }

    int max_index = 0;
    size_t prefix_len = strlen(prefix);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;

        if (strncmp(name, prefix, prefix_len) != 0) {
            continue;
        }

        if (name[prefix_len] != '.') {
            continue;
        }

        const char *suffix = name + prefix_len + 1;

        int index = atoi(suffix);
        if (index > max_index) {
            max_index = index;
        }
    }

    closedir(dir);
    return max_index;
}

static char *make_name(const char *prefix, int index) {
    if (index == 0) {
        char *result = malloc(strlen(prefix) + 1);
        if (result == NULL) {
            return NULL;
        }

        strcpy(result, prefix);
        return result;
    }

    int size = snprintf(NULL, 0, "%s.%d", prefix, index);
    if (size < 0) {
        return NULL;
    }

    char *result = malloc((size_t)size + 1);
    if (result == NULL) {
        return NULL;
    }

    snprintf(result, (size_t)size + 1, "%s.%d", prefix, index);
    return result;
}

static int rotate_logs(const char *prefix, FILE **log_file) {
    if (*log_file != NULL) {
        if (fflush(*log_file) == EOF) {
            return -1;
        }

        if (fclose(*log_file) == EOF) {
            *log_file = NULL;
            return -1;
        }

        *log_file = NULL;
    }

    int max_index = get_max_log_index(prefix);

    for (int i = max_index; i >= 1; i--) {
        char *old_name = make_name(prefix, i);
        char *new_name = make_name(prefix, i + 1);

        if (old_name == NULL || new_name == NULL) {
            free(old_name);
            free(new_name);
            return -1;
        }

        if (rename(old_name, new_name) == -1) {
            free(old_name);
            free(new_name);
            return -1;
        }

        free(old_name);
        free(new_name);
    }

    char *new_first = make_name(prefix, 1);
    if (new_first == NULL) {
        return -1;
    }

    if (rename(prefix, new_first) == -1) {
        if (errno != ENOENT) {
            free(new_first);
            return -1;
        }
    }

    free(new_first);

    *log_file = fopen(prefix, "w");
    if (*log_file == NULL) {
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {

    const char *log_path = argv[1];

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = sighup_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    FILE *log_file = fopen(log_path, "w");
    if (log_file == NULL) {
        perror("fopen");
        return 1;
    }

    char buffer[4096];

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (need_rotate) {
            need_rotate = 0;

            if (rotate_logs(log_path, &log_file) == -1) {
                perror("rotate_logs");
                if (log_file != NULL) {
                    fclose(log_file);
                }
                return 1;
            }
        }

        if (fputs(buffer, log_file) == EOF) {
            perror("fputs");
            fclose(log_file);
            return 1;
        }

        fflush(log_file);

        if (need_rotate) {
            need_rotate = 0;

            if (rotate_logs(log_path, &log_file) == -1) {
                perror("rotate_logs");
                if (log_file != NULL) {
                    fclose(log_file);
                }
                return 1;
            }
        }
    }

    if (need_rotate) {
        need_rotate = 0;

        if (rotate_logs(log_path, &log_file) == -1) {
            perror("rotate_logs");
            if (log_file != NULL) {
                fclose(log_file);
            }
            return 1;
        }
    }

    if (fclose(log_file) == EOF) {
        perror("fclose");
        return 1;
    }

    return 0;
}
