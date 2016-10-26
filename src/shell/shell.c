#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"


static int is_space(char c)
{
    return c == ' ' || c == '\t';
}

static int parse_cmd(char *in, char **cmd, int *argc, char ***argv)
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
            }
        }
    }
    
    if (argv) {
        *argv = arg_list;
    }
    
    return EOK;
}

static int free_cmd(int argc, char **argv)
{
    int i;
    
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


int main(int argc, char *argv[])
{
    int i = 0;
    char buf[64];
    
    kprintf("Toddler shell started!\n");
    
    kapi_process_started(0);
    
    // Block here
    do {
        syscall_yield();
    } while (1);
    
    return 0;
}
