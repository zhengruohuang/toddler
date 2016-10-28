#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "shell/include/shell.h"


/*
 * Built-in commands
 */
#define BUILTIN_CMD_COUNT   (sizeof(builtin_cmd_list) / sizeof(struct builtin_cmd))

struct builtin_cmd {
    char *name;
    int (*func)(int argc, char **argv);
};

static struct builtin_cmd builtin_cmd_list[] = {
    { "hello", hello },
    { "echo", echo },
};


/*
 * Execute commands
 */
int exec_cmd(char *cmd, int argc, char **argv)
{
    int i;
    int found = 0;
    int err = EOK;
    
    // Built-in commands
    for (i = 0; i < BUILTIN_CMD_COUNT; i++) {
        struct builtin_cmd *c = &builtin_cmd_list[i];
        if (!strcmp(c->name, cmd)) {
            found = 1;
            err = c->func(argc, argv);
            break;
        }
    }
    
    // External commands
    if (!found) {
        return -1;
    }
    
    return EOK;
}


/*
 * Parse commands
 */
static int is_space(char c)
{
    return c == ' ' || c == '\t';
}

int parse_cmd(char *in, char **cmd, int *argc, char ***argv)
{
    char *c = in;
    int cmd_pos = 0;
    int arg_count = 0;
    char **arg_list = NULL;
    
    // Extract the command
    while (*c && !is_space(*c)) {
        cmd_pos++;
        c++;
    }
    
    if (cmd) {
        *cmd = (char *)calloc(cmd_pos, sizeof(char));
        memcpy(*cmd, in, cmd_pos);
        (*cmd)[cmd_pos] = '\0';
    }
    
    // Count arguments
    while (*c) {
        int arg_len = 0;
        
        // Skip space
        while (*c && is_space(*c)) {
            c++;
        }
        
        // Skip arg
        arg_len = 0;
        while (*c && !is_space(*c)) {
            arg_len++;
            c++;
        }
        
        // Arg count
        if (arg_len) {
            arg_count++;
        }
    }
    
    if (argc) {
        *argc = arg_count;
    }
    
    // Construct arg list
    if (arg_count) {
        int arg_index = 0;
        int arg_pos = cmd_pos;
        int arg_len = 0;
        
        c = in + cmd_pos;
        arg_list = (char **)calloc(arg_count, sizeof(char *));
        
        while (*c) {
            // Skip space
            while (*c && is_space(*c)) {
                c++;
                arg_pos++;
            }
            
            // Process arg
            arg_len = 0;
            while (*c && !is_space(*c)) {
                arg_len++;
                c++;
            }
            
            // Construct arg
            if (arg_len) {
                char *arg = (char *)calloc(arg_len + 1, sizeof(char));
                memcpy(arg, in + arg_pos, arg_len);
                arg[arg_len] = '\0';
                arg_list[arg_index] = arg;
                
                arg_index++;
                arg_pos += arg_len;
                
//                 kprintf("Arg parsed: %s, len: %d\n", arg, arg_len);
            }
        }
    }
    
    if (argv) {
        *argv = arg_list;
    }
    
    return EOK;
}

int free_cmd(char *cmd, int argc, char **argv)
{
    int i;
    
    if (cmd) {
        free(cmd);
    }
    
    for (i = 0; i < argc; i++) {
        if (argv[i]) {
            free(argv[i]);
        }
    }
    
    if (argv) {
        free(argv);
    }
    
    return EOK;
}
