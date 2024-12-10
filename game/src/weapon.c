#include "game_funcs.h"

weapon weapon_info[] =
{   //   id      slot name             bullet:type     count speed       mode  rate range,   damage  clip  ammo   reload
    { WT_pistol,  1,  "Handgun",       ET_bullet01,      1,  30.0f, FM_normal,  6,  125,   15,  25,   7,  100,  0.8f  },
    { WT_auto,    2,  "Colt Commando", ET_bullet02,      1,  40.0f, FM_normal, 10,  300,   15,  30,  40,  100,  2.0f  },
    { WT_shotgun, 3,  "Shotgun",       ET_kunai,         5,  25.0f, FM_spread,  3,   90,   20,  40,   5,  100,  1.0f  },
    { WT_lazer,   4,  "DethLazer",     ET_bullet_lazer,  3,  50.0f, FM_spread, 60,  400,    2,   4, 500,  100,  3.0f  },
    { WT_ion,     5,  "Omnionizer",    ET_shuriken,      8,  20.0f, FM_cycle,   1,   60,   25,  35,  80,  100,  2.0f  }
};
