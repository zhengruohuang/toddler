#ifndef __SHELL_INCLUDE_BUILTIN__
#define __SHELL_INCLUDE_BUILTIN__


#include "common/include/data.h"


extern int find_builtin_cmd(char *name);
extern int exec_builtin_cmd(int index, int argc, char **argv);

extern int hello(int argc, char **argv);
extern int echo(int argc, char **argv);
extern int ls(int argc, char **argv);
extern int cat(int argc, char **argv);
extern int touch(int argc, char **argv);
extern int cd(int argc, char **argv);


#endif
