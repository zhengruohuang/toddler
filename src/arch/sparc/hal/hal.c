#include "common/include/data.h"

struct boot_parameters {
    int a;
};

void entry_func hal_entry(struct boot_parameters *boot_param)
{
    // Should not reach here
//     halt();
    while (1);
}
