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
extern int parse_cmd(char *in, char **cmd, int *argc, char ***argv, char **stdin, char **stdout, char **stderr);
extern int free_cmd(char *cmd, int argc, char **argv, char *in, char *out, char *err);


/*
 * Path manipulation
 */
extern int is_valid_path(const char *path);
extern int is_absolute_path(const char *path);
extern int is_relative_path(const char *path);
extern char *join_path(const char *a, const char *b);
extern char *normalize_path(const char *path);


/*
 * Working directory
 */
extern void init_cwd();
extern char *get_cwd();
extern int change_cwd(char *path);


/*
 * Path operation helpers
 */
extern unsigned long open_path(char *name, unsigned int flags);


#endif
