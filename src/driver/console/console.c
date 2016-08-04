#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"
#include "driver/include/console.h"


#define CONSOLE_COUNT   2


static unsigned long activated_id;
static struct console consoles[CONSOLE_COUNT];


/*
 * Console creation
 */
static void init_stdio_buf(struct stdio_buffer *buf)
{
    memzero(&buf->data, STDIO_BUF_SIZE);
    buf->size = STDIO_BUF_SIZE;
    buf->index = 0;
}

unsigned long create_console(char *name)
{
    int i;
    int found = 0;
    
    // Find a free console entry
    for (i = 0; i < CONSOLE_COUNT; i++) {
        if (!consoles[i].enabled) {
            found = 1;
            break;
        }
    }
    
    // Setup the console
    consoles[i].enabled = 1;
    consoles[i].id = (unsigned long)&consoles[i];
    consoles[i].name = name;
    consoles[i].activated = 0;
    init_stdio_buf(&consoles[i].stdin_buf);
    
    // Register in URS
    
    return consoles[i].id;
}


/*
 * Console manipulation
 */
static struct console *find_console(unsigned long console_id)
{
    int i;
    
    for (i = 0; i < CONSOLE_COUNT; i++) {
        if (consoles[i].id == console_id && consoles[i].enabled) {
            return &consoles[i];
        }
    }
    
    return NULL;
}

struct console *get_console(unsigned long console_id)
{
    struct console *con = NULL;
    
    con = find_console(console_id);
    
    return con;
}

int activate_console(unsigned long console_id)
{
    struct console *con = NULL;
    
    con = find_console(console_id);
    if (!con) {
        return -1;
    }
    
    activated_id = console_id;
    
    return 0;
}

unsigned long get_activated_console_id()
{
    return activated_id;
}


/*
 * Init
 */
void init_console()
{
    unsigned long id = create_console("default");
    activate_console(id);
}
