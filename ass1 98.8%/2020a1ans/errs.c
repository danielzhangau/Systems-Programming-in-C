#include <stdio.h>
#include "errs.h"

// Display error message for the error 
Errs err_msg(Errs type)
{
    const char* msg = "";
    switch (type) {
        case OK:
            return OK;
        case ARGS:
            msg = "Usage: push2310 typeO typeX fname";
            break;
        case BADTYPE:
            msg = "Invalid player type";
            break;
        case NOFILE:
            msg = "No file to load from";
            break;
        case FILECONTENTS:
            msg = "Invalid file contents";
            break;
        case PLAYEREOF:
            msg = "End of file";
            break;
        case FULLBOARD:
            msg = "Full board in load";
            break;
    }
    fprintf(stderr, "%s\n", msg);
    return type;
}
