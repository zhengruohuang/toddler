#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/string.h"
#include "shell/include/builtin.h"


#define BUILTIN_CMD_COUNT   (sizeof(builtin_cmd_list) / sizeof(struct builtin_cmd))


struct builtin_cmd {
    char *name;
    int (*func)(int argc, char **argv);
};

static struct builtin_cmd builtin_cmd_list[] = {
    { "hello", hello },
    { "welcome", hello },
    { "toddler", hello },
    { "logo", hello },
    { "echo", echo },
    { "ls", ls },
    { "cat", cat },
    { "touch", touch },
    { "cd", cd },
    { "pwd", pwd },
};


int find_builtin_cmd(char *name)
{
    int i;
    
    // Built-in commands
    for (i = 0; i < BUILTIN_CMD_COUNT; i++) {
        struct builtin_cmd *c = &builtin_cmd_list[i];
        if (!strcmp(c->name, name)) {
            return i;
        }
    }
    
    return -1;
}

int exec_builtin_cmd(int index, int argc, char **argv)
{
    struct builtin_cmd *c = &builtin_cmd_list[index];
    return c->func(argc, argv);
}
