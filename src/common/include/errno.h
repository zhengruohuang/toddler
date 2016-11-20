#ifndef __COMMON_INCLUDE_ERRNO__
#define __COMMON_INCLUDE_ERRNO__


// No error
#define EOK         0       // No error

// Permission
#define EPERM       -1      // Permission denied

// Resource
#define ENOENT      -10     // No such entry
#define ELIMIT      -11     // Limit exceeded
#define EBUSY       -12     // Resource busy
#define EBADF       -13     // Bad file number
#define ECLOSED     -14     // File closed

// Memory
#define ENOMEM      -20     // No enough memory
#define EBADMEM     -21     // Bad memory address

// Value
#define EINVAL      -20     // Invalid value
#define EOVERFLOW   -31     // Value overflow

#endif
