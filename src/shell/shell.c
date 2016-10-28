#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


unsigned long cmd_block_salloc_id = 0;


/*
 * stdout and stderr tests
 */
static void test_out()
{
    char stdout_str[] = "Printing through stdout\n";
    char stderr_str[] = "Printing through stderr\n";
    
    kapi_stdout_write(0, stdout_str, sizeof(stdout_str));
    kapi_stderr_write(0, stderr_str, sizeof(stderr_str));
}


/*
 * Welcome message
 */
void welcome()
{
    char *welcome_msg[] = {
        "  _______        _     _ _           \n",
        " |__   __|      | |   | | |          \n",
        "    | | ___   __| | __| | | ___ _ __ \n",
        "    | |/ _ \\ / _` |/ _` | |/ _ \\ '__|\n",
        "    | | (_) | (_| | (_| | |  __/ |   \n",
        "    |_|\\___/ \\__,_|\\__,_|_|\\___|_|   \n",
        "                                     \n",
    };
    
    int i;
    for (i = 0; i < 7; i++) {
        kprintf(welcome_msg[i]);
    }
}


/*
 * Command input
 */
#define CMD_BLOCK_SIZE  64

struct cmd_block {
    char buf[CMD_BLOCK_SIZE];
    int count;
    
    struct cmd_block *prev;
    struct cmd_block *next;
};

struct cmd_builder {
    int count;
    
    struct cmd_block *head;
};

static int input(char **cmd)
{
    char stdin_buf[32];
    int i, offset;
    int done = 0;
    
    char *input = NULL;
    
    struct cmd_builder build;
    struct cmd_block *cur_block = (struct cmd_block *)salloc(cmd_block_salloc_id);
    
    build.count = 0;
    build.head = cur_block;
    
    cur_block->count = 0;
    cur_block->prev = NULL;
    cur_block->next = NULL;
    
    // Input
    do {
        unsigned long size = 0;
        kprintf("_");
        
        size = kapi_stdin_read(0, stdin_buf, sizeof(stdin_buf));
        if (size) {
            kprintf("\b");
            
            for (i = 0; i < (int)size; i++) {
                char cur = stdin_buf[i];
                if (cur != '\b' || build.count) {
                    kprintf("%c", cur);
                }
                
                if (cur == '\r' || cur == '\n') {
                    done = 1;
                    break;
                }
                
                else if (cur == '\b') {
                    if (build.head != cur_block && !cur_block->count) {
                        struct cmd_block *rm = cur_block;
                        
                        cur_block = cur_block->prev;
                        cur_block->next = NULL;
                        
                        sfree(rm);
                    }
                    
                    if (cur_block->count) {
                        cur_block->count--;
                        build.count--;
                    }
                }
                
                else {
                    if (cur_block->count >= CMD_BLOCK_SIZE) {
                        struct cmd_block *new_block = (struct cmd_block *)salloc(cmd_block_salloc_id);
                        new_block->count = 0;
                        new_block->prev = cur_block;
                        new_block->next = NULL;
                        
                        cur_block->next = new_block;
                        cur_block = new_block;
                    }
                    
                    cur_block->buf[cur_block->count] = cur;
                    cur_block->count++;
                    build.count++;
                }
            }
        }
    } while (!done);
    
    // Concatinate
    input = calloc(build.count + 1, sizeof(char));
    i = 0;
    
    offset = 0;
    cur_block = build.head;
    while (cur_block && i < build.count) {
        input[i] = cur_block->buf[offset];
        i++;
        offset++;
        
        if (offset >= CMD_BLOCK_SIZE) {
            cur_block = cur_block->next;
        }
    }
    
    input[build.count] = '\0';
    
    // Clean up
    cur_block = build.head;
    while (cur_block) {
        struct cmd_block *rm = cur_block;
        cur_block = cur_block->next;
        sfree(rm);
    }
    
    // Done
    if (cmd) {
        *cmd = input;
    }
    
    return build.count;
}

static void prompt()
{
    int len;
    char *cmd;
    
    char *name;
    int argc;
    char **argv;
    
    do {
        kprintf("system > ");
        len = input(&cmd);
        
        name = NULL;
        argc = 0;
        argv = NULL;
        
        parse_cmd(cmd, &name, &argc, &argv);
        exec_cmd(name, argc, argv);
        
        free_cmd(name, argc, argv);
        free(cmd);
    } while (1);
}


/*
 * Init
 */
static void init_shell()
{
    cmd_block_salloc_id = salloc_create(sizeof(struct cmd_block), 0, NULL, NULL);
}

int main(int argc, char *argv[])
{
    init_shell();
    kprintf("Toddler shell started!\n");
    test_out();
    kapi_process_started(0);
    
    welcome();
    prompt();
    
    // Block here
    do {
        syscall_yield();
    } while (1);
    
    return 0;
}
