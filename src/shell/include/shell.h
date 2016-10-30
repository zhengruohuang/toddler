#ifndef __SHELL_INCLUDE_SHELL__
#define __SHELL_INCLUDE_SHELL__


#include "common/include/data.h"


/*
 * Main
 */
extern void welcome();


/*
 * Control routines
 */
extern int exec_cmd(char *cmd, int argc, char **argv);
extern int parse_cmd(char *in, char **cmd, int *argc, char ***argv);
extern int free_cmd(char *cmd, int argc, char **argv);


#endif
