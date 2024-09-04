#include "game_funcs.h"

weapon weapon_info[] =
{   //   id      slot icon             bullet:type     count speed     mode  rate range,  damage  clip  ammo   reload
    { WT_pistol,  1,  UI_WT_pistol,    ET_bullet01,      1,  14.0f, FM_normal, 6,   125,  5,  10,   7,  100,   1.0f  },
    { WT_auto,    2,  UI_WT_auto,      ET_bullet02,      1,  18.0f, FM_normal, 10,  300, 15,  40,  40,  100,   3.0f  },
    { WT_shotgun, 3,  UI_WT_shotgun,   ET_kunai,         5,  12.0f, FM_spread, 3,   90,   5,  10,   5,  100,   0.5f  },
    { WT_lazer,   4,  UI_WT_pistol,    ET_bullet_lazer,  3,  30.0f, FM_spread, 60,  400,  1,   3, 500,  100,   4.0f  },
    { WT_ion,     5,  UI_WT_auto,      ET_shuriken,      8,   8.0f, FM_cycle,   1,  120,  15, 30,  80,  100,   2.0f  }
};
