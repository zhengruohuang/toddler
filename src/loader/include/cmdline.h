#ifndef __LOADER_INCLUDE_CMDLINE__
#define __LOADER_INCLUDE_CMDLINE__


extern int query_cmdline(char *key, char **value_out, int *len_out);
extern int check_cmdline(char *key, char *value);
extern void parse_cmdline(char *base, int size);


#endif
