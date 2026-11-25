#ifndef UTILS_H
#define UTILS_H

#include "uletypes.h"

/* blend src color to dst color. returns result of their blending */
ule_u32 maus_blend(ule_u32 src, ule_u32 dst);

/* duplicate a string. returns pointer to address of
   newly allocated string buffer. if NULL, allocation
   failed or src is NULL
   note: you must free this newly allocated string */
char* maus_strdup(const char* src);

#endif /* UTILS_H */

