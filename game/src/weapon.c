#include "game_funcs.h"

weapon weapon_info[] =
{   // weapon_type  bullet:type count  speed mode      rate   min max
    { WT_pistol,    ET_bullet00,  1,   14.0f,  FM_normal, 2,   5, 10  },
    { WT_secondary, ET_bullet01,  1,   10.0f,  FM_normal, 2,  10, 20  },
    { WT_SpreadGun, ET_bullet02,  5,   14.0f,  FM_Spread, 2,  15, 30  }
};
