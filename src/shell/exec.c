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

int parse_cmd(char *in, char **cmd, int *argc, char ***argv, char **stdin, char **stdout, char **stderr)
{
    int i;
    char *c = in;
    char *cmd_name = NULL;
    int cmd_pos = 0;
    int part_count = 0;
    char **part_list = NULL;
    
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
    part_count = 1;
    
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
            part_count++;
        }
    }
    
    // Construct arg list
    if (part_count) {
        int arg_index = 0;
        int arg_pos = cmd_pos;
        int arg_len = 0;
        
        c = in + cmd_pos;
        part_list = (char **)calloc(part_count, sizeof(char *));
        
        part_list[0] = strdup(cmd_name);
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
                part_list[arg_index] = arg;
                
                arg_index++;
                arg_pos += arg_len;
                
//                 kprintf("Arg parsed: %s, len: %d\n", arg, arg_len);
            }
        }
    }
    
    if (!cmd) {
        free(cmd_name);
    }
    
    // Extract stdio redirection
    char *stdin_redirect = NULL;
    char *stdout_redirect = NULL;
    char *stderr_redirect = NULL;
    
    i = 0;
    do {
        int shift = 0;
        
        // stdin
        if (!strcmp(part_list[i], "<")) {
            if (i + 1 < part_count) {
                stdin_redirect = part_list[i + 1];
                free(part_list[i]);
                shift = 2;
            } else {
                shift = 1;
            }
        }
        
        // stdout
        else if (!strcmp(part_list[i], ">")) {
            if (i + 1 < part_count) {
                stdout_redirect = part_list[i + 1];
                free(part_list[i]);
                shift = 2;
            } else {
                shift = 1;
            }
        }
        
        // stderr
        else if (!strcmp(part_list[i], "e>")) {
            if (i + 1 < part_count) {
                stderr_redirect = part_list[i + 1];
                free(part_list[i]);
                shift = 2;
            } else {
                shift = 1;
            }
        }
        
        // Take care of shift
        if (shift) {
            int idx;
            for (idx = i; idx < part_count - shift; idx++) {
                part_list[idx] = part_list[idx + shift];
            }
            
            part_count -= shift;
        } else {
            i++;
        }
    } while (i < part_count);
    
    if (argc) {
        *argc = part_count;
    }
    
    if (argv) {
        *argv = part_list;
    }
    
    if (stdin_redirect || stdout_redirect || stderr_redirect) {
        kprintf("stdio redirection, stdin: %s, stdout: %s, stderr: %s\n",
                stdin_redirect ? stdin_redirect : "no",
                stdout_redirect ? stdout_redirect : "no",
                stderr_redirect ? stderr_redirect : "no"
        );
    }
    
    if (stdin) {
        *stdin = stdin_redirect;
    } else {
        free(stdin_redirect);
    }
    
    if (stdout) {
        *stdout = stdout_redirect;
    } else {
        free(stdout_redirect);
    }
    
    if (stderr) {
        *stderr = stderr_redirect;
    } else {
        free(stderr_redirect);
    }
    
    return EOK;
}

int free_cmd(char *cmd, int argc, char **argv, char *in, char *out, char *err)
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
    
    if (in) {
        free(in);
    }
    
    if (out) {
        free(out);
    }
    
    if (err) {
        free(err);
    }
    
    return EOK;
}
