#ifndef UTILS_H
#define UTILS_H

/* duplicate a string. returns pointer to address of
   newly allocated string buffer. if NULL, allocation
   failed or src is NULL
   note: you must free this newly allocated string */
char* maus_strdup(const char* src);

#endif /* UTILS_H */

