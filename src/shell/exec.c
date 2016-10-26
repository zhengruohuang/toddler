#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/string.h"
#include "shell/include/shell.h"


struct builtin_cmd {
    char *name;
    int (*func)(int argc, char **argv);
};

static struct builtin_cmd builtin_cmd_list[] = {
    { "hello", hello },
    { "echo", echo },
};

#define BUILTIN_CMD_COUNT   (sizeof(builtin_cmd_list) / sizeof(struct builtin_cmd))


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
    }
    
    return EOK;
}
