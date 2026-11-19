#include "tokenizer.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

void Exec(struct Tokenizer* tokenizer) {
    if (tokenizer->head == NULL) {
        return;
    }
    //> <
    bool flag_in = false;
    struct Token* token_in = NULL;
    bool flag_out = false;
    struct Token* token_out = NULL;
    bool syntax_error = false;
    
    struct Token* token = tokenizer->head;
    for (int i = 0; i < tokenizer->token_count; ++i) {
        if (token->type == TT_INFILE) {
            if (flag_in) {
                syntax_error = true;
            }
            flag_in = true;
            if (i < tokenizer->token_count - 1 && token->next->type == TT_WORD) {
                token_in = token->next;
            } else {
                syntax_error = true;
            }
        } else if (token->type == TT_OUTFILE) {
            if (flag_out) {
                syntax_error = true;
            }
            flag_out = true;
            if (i < tokenizer->token_count - 1 && token->next->type == TT_WORD) {
                token_out = token->next;
            } else {
                syntax_error = true;
            }
        }
        token = token->next;
    }
    
    if (syntax_error) {
        printf("Syntax error\n");
        return;
    }
    //
    
    int amount = tokenizer->token_count + 1;
    if (flag_in) {
        amount -= 2;
    }
    if (flag_out) {
        amount -= 2;
    }
    char* argv[amount];
    char* filename;
    argv[amount - 1] = NULL;
    
    int argument = 0;
    token = tokenizer->head;
    for (int i = 0; i < tokenizer->token_count; ++i) {
        if (token->type == TT_WORD && token != token_in && token != token_out) {
            char* buf = malloc(token->len + 1);
            memcpy(buf, token->start, token->len);
            buf[token->len] = '\0';
            argv[argument] = buf;
            if (argument == 0) {
                filename = buf;
            }
            ++ argument;
        }
        token = token->next;
        
    }
    pid_t pid = fork();
    if (pid < 0) {
        for (int i = 0; i < amount; ++i) {
            free(argv[i]);
        }
        return;
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        for (int i = 0; i < amount; ++i) {
            free(argv[i]);
        }
    } else {
        int res_in = 0;
        int res_out = 0;
        if (flag_in) {
            char* buf = malloc(token_in->len + 1);
            memcpy(buf, token_in->start, token_in->len);
            buf[token_in->len] = '\0';
            res_in = open(buf, O_RDONLY);
            free(buf);
        }
        if (flag_out) {
            char* buf = malloc(token_out->len + 1);
            memcpy(buf, token_out->start, token_out->len);
            buf[token_out->len] = '\0';
            res_out = open(buf, O_WRONLY | O_TRUNC | O_CREAT, 0644);
            free(buf);
        }
        if (res_out == -1 || res_in == -1) {
            printf("I/O error\n");
            for (int i = 0; i < amount; ++i) {
                free(argv[i]);
            }
            int status = 1;
            exit(status);
            return;
        }
        if (flag_in) {
            int fr_read = dup2(res_in, STDIN_FILENO);
            close(res_in);
            if (fr_read == -1) {
                for (int i = 0; i < amount; ++i) {
                    free(argv[i]);
                }
                int status = 1;
                exit(status);
            }
        }
        
        if (flag_out) {
            int fr_write = dup2(res_out, STDOUT_FILENO);
            close(res_out);
            if (fr_write == -1) {
                for (int i = 0; i < amount; ++i) {
                    free(argv[i]);
                }
                int status = 1;
                exit(status);
            }
        }
        
        execvp(filename, argv);
        printf("Command not found\n");
        for (int i = 0; i < amount; ++i) {
            free(argv[i]);
        }
        int status = 1;
        exit(status);
    }
}

