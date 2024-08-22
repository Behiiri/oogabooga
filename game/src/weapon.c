#include "game_funcs.h"

weapon weapon_info[] =
{   // weapon_type  icon            bullet:type     count speed    mode   rate range, min max
    { WT_pistol,   UI_WT_pistol,    ET_bullet01,      1,  14.0f, FM_normal, 6,   125,  5,  10  },
    { WT_auto,     UI_WT_auto,      ET_bullet02,      1,  18.0f, FM_normal, 10,  400, 15,  40  },
    { WT_shotgun,  UI_WT_shotgun,   ET_bullet00,      5,  15.0f, FM_spread, 3,   75,   2,   8  },
    { WT_lazer,    UI_WT_pistol,    ET_bullet_lazer,  3,  30.0f, FM_spread, 60,  500,  1,   3  },
    { WT_ion,      UI_WT_auto,      ET_bullet05,      8,   8.0f, FM_cycle,   1,  200,  25, 50  }
};
