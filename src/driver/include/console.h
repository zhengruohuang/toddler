#ifndef __DRIVER_INCLUDE_CONSOLE__
#define __DRIVER_INCLUDE_CONSOLE__

#include "common/include/data.h"


#define STDIO_BUF_SIZE  32


struct stdio_buffer {
    char data[STDIO_BUF_SIZE];
    int size;
    int index;
};


struct console {
    unsigned long id;
    char *name;
    int enabled;
    int activated;
    
    // stdio
    struct stdio_buffer stdin_buf;
};


/*
 * Console
 */
unsigned long create_console(char *name);

struct console *get_console(unsigned long console_id);
int activate_console(unsigned long console_id);
unsigned long get_activated_console_id();

void init_console();


/*
 * Stdio
 */
extern int stdin_write(unsigned long console_id, char *buf, size_t size);
extern asmlinkage void stdin_read_handler(msg_t *msg);
extern asmlinkage void stdout_write_handler(msg_t *msg);
extern asmlinkage void stderr_write_handler(msg_t *msg);


#endif
