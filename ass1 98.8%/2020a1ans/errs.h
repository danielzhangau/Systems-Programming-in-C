#ifndef ERRS_H
#define ERRS_H

typedef enum {
    OK = 0,
    ARGS = 1,
    BADTYPE = 2,
    NOFILE = 3,
    FILECONTENTS = 4,
    PLAYEREOF = 5,
    FULLBOARD = 6
} Errs;

typedef enum {
    T0 = 0,
    T1 = 1,
    HUMAN = 2,
    ERR_TYPE = 3
} PType;

Errs err_msg(Errs type);

#endif
