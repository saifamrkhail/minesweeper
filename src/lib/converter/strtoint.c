/**
* Created by saif on 1/21/20.
*/
///

#include <errno.h>
#include <stdlib.h>

int str_to_int(char *str) {
    char *rest = "\0";
    errno = 0; // always reset errno
    double integer = strtod(str, &rest);
    if (rest == str || *rest != '\0' || errno) {
        return -1;
    } else {
        if (integer >= 0) {
            return (int) integer;
        } else {
            return -1;
        }
    }
}
