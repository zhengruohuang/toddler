#include "common/include/data.h"
#include "loader/include/print.h"
#include "loader/include/lib.h"


enum parser_state {
    state_key,
    state_assign,
    state_value,
    state_sep,
};

enum token_types {
    token_empty,
    token_str,
    token_assign,
    token_sep,
};

static char *cmdline = NULL;
static int cmdline_size = 0;


static void print_cmdline(int start, int len)
{
    int end = start + len;
    int i;
    
    for (i = start; i < end && i < cmdline_size; i++) {
        lprintf("%c", cmdline[i]);
    }
}

static int get_cmdline_token(int start, int *at_out, int *len_out)
{
    int idx = start;
    int at = 0, len = 0;
    int token = token_empty;
    int found = 0;
    
    // Skip leading spaces
    char c = cmdline[idx];
    while (idx < cmdline_size && (c == ' ' || c == '\t')) {
        idx++;
        c = cmdline[idx];
    }
    
    // Set the initial state
    at = idx;
    
    // Find the next token
    while (!found && idx < cmdline_size && c) {
        switch (c) {
        case ':':
        case '=':
            if (!len) {
                token = token_assign;
                len = 1;
            }
            found = 1;
            break;
        case ' ':
        case '\t':
            if (!len) {
                token = token_empty;
            }
            found = 1;
            break;
        default:
            token = token_str;
            len++;
            break;
        }
        
        idx++;
        c = cmdline[idx];
    }
    
    // Done
    if (at_out) *at_out = at;
    if (len_out) *len_out = len;
    
    return token;
}

static int get_value_token(int start, int end, int *at_out, int *len_out)
{
    int idx = start;
    int at = idx, len = 0;
    char c = 0;
    int token = token_empty;
    int found = 0;
    
    // Get the first char
    if (idx < end) {
        c = cmdline[idx];
    }
    
    // Find the next token
    while (!found && idx < end && c) {
        switch (c) {
        case ',':
        case ';':
            if (!len) {
                token = token_sep;
                len = 1;
            }
            found = 1;
            break;
        default:
            token = token_str;
            len++;
            break;
        }
        
        idx++;
        c = cmdline[idx];
    }
    
    // Done
    if (at_out) *at_out = at;
    if (len_out) *len_out = len;
    
    return token;
}


/*
 * Obtain the value for a given key
 */
int query_cmdline(char *key, int *at_out, int *len_out)
{
    int idx = 0;
    int cur = 0;
    int len = 0;
    int state = state_sep;
    int token = token_empty;
    
    int key_len = strlen(key) - 1;
    int found_key = 0, found_value = 0;
    
    token = get_cmdline_token(cur, &cur, &len);
    while (token != token_empty && (!found_key || !found_value)) {
        switch (token) {
        case token_str:
            switch (state) {
            case state_key:
                if (found_key) {
                    if (at_out) *at_out = 0;
                    if (len_out) *len_out = 0;
                    found_value = 1;
                    break;
                }
            case state_sep:
                found_key = key_len == len && !memcmp(&cmdline[cur], key, len);
                state = state_key;
                break;
            case state_value:
                if (found_key) {
                    if (at_out) *at_out = cur;
                    if (len_out) *len_out = len;
                    found_value = 1;
                }
                state = state_sep;
                break;
            default:
                break;
            }
            break;
        case token_assign:
            state = state_value;
            break;
        default:
            break;
        }
        
        cur += len;
        token = get_cmdline_token(cur, &cur, &len);
    }
    
    return found_key && found_value;
}


/*
 * For a given key, check if value is in the value list
 */
int check_cmdline(char *key, char *value)
{
    char *vlist = NULL;
    int cur = 0;
    int len = 0;
    
    int value_len = strlen(value) - 1;
    int token = token_empty;
    int found = 0;
    
    if (!query_cmdline(key, &cur, &len)) {
        return 0;
    }
    
    token = get_value_token(cur, cur + len, &cur, &len);
    while (!found && token != token_empty) {
        if (token == token_str) {
            if (len == value_len && !memcmp(&cmdline[cur], value, len)) {
                return 1;
            }
        }
        
        cur += len;
        token = get_value_token(cur, cur + len, &cur, &len);
    }
    
    return 0;
}


/*
 * Iterate through all commands
 */
void parse_cmdline(char *base, int size)
{
    int idx = 0;
    int cur = 0;
    int len = 0;
    int state = state_sep;
    int token = token_empty;
    
    cmdline = base;
    cmdline_size = size;
    
    lprintf("Command line arguments @ %p, size: %d\n", cmdline, cmdline_size);
    
    token = get_cmdline_token(cur, &cur, &len);
    while (token != token_empty) {
        switch (token) {
        case token_str:
            switch (state) {
            case state_key:
                lprintf("\n");
            case state_sep:
                lprintf("\tKey: ");
                print_cmdline(cur, len);
                state = state_key;
                break;
            case state_value:
                lprintf(", Value: ");
                print_cmdline(cur, len);
                lprintf("\n");
                state = state_sep;
                break;
            default:
                break;
            }
            break;
        case token_assign:
            state = state_value;
            break;
        default:
            break;
        }
        
        cur += len;
        token = get_cmdline_token(cur, &cur, &len);
    }
    
    lprintf("\n");
//     while (1);
}
