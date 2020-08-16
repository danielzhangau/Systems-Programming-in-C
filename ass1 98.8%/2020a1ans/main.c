#include <string.h>
#include <stdlib.h>
#include "errs.h"
#include "board.h"
#include "game.h"

// Convert type param into enum value
// returns ERR_TYPE on error
PType get_type(const char* s)
{
    if (!strcmp(s, "0")) {
        return T0;
    }
    if (!strcmp(s, "1")) {
        return T1;
    }
    if (!strcmp(s, "H")) {
        return HUMAN;
    }
    return ERR_TYPE;
}

int main(int argc, char** argv)
{
    if (argc != 4) {
        return err_msg(ARGS);
    }
    Game g;
    Errs res;
    PType t0 = get_type(argv[1]);
    PType t1 = get_type(argv[2]);
    if (t0 == ERR_TYPE || t1 == ERR_TYPE) {
        return err_msg(BADTYPE);
    }
    init_game(&g, t0, t1);
    res = load_game(argv[3], g.board);
    if (res != OK) {
        return err_msg(res);
    }
    res = run_game(&g);
    if (res != OK) {
        return err_msg(res);
    }
    return 0;
}
