#include "game_funcs.h"

weapon weapon_info[] =
{   // weapon_type  icon          bullet:type count speed    mode   rate range, min max
    { WT_pistol,   UI_WT_pistol,  ET_bullet01,  1,  14.0f, FM_normal, 3,  125,  5,  10  },
    { WT_auto,     UI_WT_auto,    ET_bullet02,  1,  18.0f, FM_normal, 5,  400, 15,  40  },
    { WT_shotgun,  UI_WT_shotgun, ET_bullet00,  5,  12.0f, FM_spread, 1,  75,   2,   8  }
};
