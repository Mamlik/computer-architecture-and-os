#include "tokenizer.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

void Exec(struct Tokenizer* tokenizer) {
    if (tokenizer->head == NULL) {
        return;
    }
    char* argv[tokenizer->token_count + 1];
    char* filename;
    char* buf = malloc(tokenizer->head->len + 1);
    memcpy(buf, tokenizer->head->start, tokenizer->head->len);
    buf[tokenizer->head->len] = '\0';
    filename = buf;
    argv[0] = filename;
    argv[tokenizer->token_count] = NULL;
    struct Token* token = tokenizer->head->next;
    for (int i = 1; i < tokenizer->token_count; ++i) {
        char* buf = malloc(token->len + 1);
        memcpy(buf, token->start, token->len);
        buf[token->len] = '\0';
        argv[i] = buf;
        token = token->next;
    }
    pid_t pid = fork();
    if (pid < 0) {
        for (int i = 0; i < tokenizer->token_count + 1; ++i) {
            free(argv[i]);
        }
        return;
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        for (int i = 0; i < tokenizer->token_count + 1; ++i) {
            free(argv[i]);
        }
    } else {
        execvp(filename, argv);
        printf("Command not found\n");
        for (int i = 0; i < tokenizer->token_count + 1; ++i) {
            free(argv[i]);
        }
        int status = 1;
        exit(status);
    }
}
