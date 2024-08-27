#include "game_funcs.h"

weapon weapon_info[] =
{   // weapon_type  icon            bullet:type     count speed    mode   rate range, min max
    { WT_pistol,   UI_WT_pistol,    ET_bullet01,      1,  14.0f, FM_normal, 6,   125,  5,  10  },
    { WT_auto,     UI_WT_auto,      ET_bullet02,      1,  18.0f, FM_normal, 10,  300, 15,  40  },
    { WT_shotgun,  UI_WT_shotgun,   ET_kunai,         5,  12.0f, FM_spread, 3,   90,   5,  10  },
    { WT_lazer,    UI_WT_pistol,    ET_bullet_lazer,  3,  30.0f, FM_spread, 60,  400,  1,   3  },
    { WT_ion,      UI_WT_auto,      ET_shuriken,      8,   8.0f, FM_cycle,   1,  120,  15, 30  }
};
