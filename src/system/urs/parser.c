#include "common/include/data.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "system/include/urs.h"


int parse_url_namespace(char *path, char **out)
{
    char c;
    char *str;
    int i = 0;
    int found = 0;
    
    if (!path || path[0] == ':' || path[0] == '\0') {
        return -1;
    }
    
    if (path[0] == '/') {
        if (out) {
            *out = strdup("vfs");
        }
        return 1;
    }
    
    do {
        c = path[i];
        if (c == ':') {
            if (path[i + 1] == '/' && path[i + 2] == '/') {
                found = 1;
                break;
            } else {
                return -1;
            }
        }
        
        i++;
    } while (c);
    
    if (!found) {
        return -1;
    }
    
    str = malloc(sizeof(char) * (i + 1));
    memcpy(str, path, i);
    str[i] = '\0';
    if (out) {
        *out = str;
    }
    
    return i;
}

int parse_url_node(char *path, int start, char **out)
{
    char c;
    char *str;
    int cur = start;
    int found = 0;
    
    if (!path || path[start] == '\0' || path[start] == '/') {
        return -1;
    }
    
    do {
        c = path[cur];
        
        if (c == '/' || c == '\0') {
            found = 1;
            break;
        }
        
        cur++;
    } while (c);
    
    if (!found) {
        return -1;
    }
    
    str = malloc(sizeof(char) * (cur - start + 1));
    memcpy(str, &path[start], cur - start);
    str[cur - start] = '\0';
    if (out) {
        *out = str;
    }
    
    return c == '/' ? cur + 1 : cur;
}
