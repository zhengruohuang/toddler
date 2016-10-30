#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "shell/include/shell.h"
#include "shell/include/builtin.h"


/*
 * Execute commands
 */
int exec_cmd(char *cmd, int argc, char **argv)
{
    int err = EOK;
    int builtin_index = find_builtin_cmd(cmd);
    
    if (builtin_index >= 0) {
        err = exec_builtin_cmd(builtin_index, argc, argv);
    } else {
        err = -1;
    }
    
    return err;
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
    char *cmd_name = NULL;
    int cmd_pos = 0;
    int arg_count = 0;
    char **arg_list = NULL;
    
    // Skip leading space
    while (*in && is_space(*in)) {
        in++;
    }
    
    // Extract the command
    c = in;
    while (*c && !is_space(*c)) {
        cmd_pos++;
        c++;
    }
    
    cmd_name = (char *)calloc(cmd_pos + 1, sizeof(char));
    memcpy(cmd_name, in, cmd_pos);
    cmd_name[cmd_pos] = '\0';
    
    if (cmd) {
        *cmd = cmd_name;
    }
    
//     kprintf("cmd name: %s, pos: %d\n", cmd_name, cmd_pos);
    
    // Count arguments
    arg_count = 1;
    
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
        
        arg_list[0] = strdup(cmd_name);
        arg_index++;
        
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
    
    if (!cmd) {
        free(cmd_name);
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
