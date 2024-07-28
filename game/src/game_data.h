#ifndef INCLUDE_GAME_DATA_H
#define INCLUDE_GAME_DATA_H

#define internal static
#define global   static

typedef unsigned char   uint8;
typedef   signed char    int8;
typedef unsigned short uint16;
typedef   signed short  int16;
typedef unsigned int   uint32;
typedef   signed int    int32;
typedef unsigned int   uint32;
typedef          int     Bool;

#ifndef True
#define True 1
#define False 0
#endif

#ifdef _MSC_VER
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef long long int64;
typedef unsigned long long uint64;
#endif

typedef struct { float x,y    ; } vec;
typedef struct { float x,y,z  ; } vec3;
typedef struct { float x,y,z,w; } vec4;
typedef struct { vec m[3]     ; } mat3;
typedef struct { vec4 m[4]    ; } mat4;
typedef struct { float x,y,z,w; } quat;
typedef struct { int x,y      ; } vec2i;

typedef int entity_id;

#define TILE_SIZE 16

typedef struct
{
    vec pos;
    vec velocity; // only used for bullets
    vec rotation; // only used for bullets
    int type;
    int hp;
    uint32 valid;
    int sprite_id;
} entity;

enum
{
    MODE_game,
    Mode_menu
};

// entity types
enum
{
    ET__none,
    ET_player,
    ET_ground,
    ET_ground2,
    ET_ground3,

    ET__bullets_start,
    ET_bullet00 = ET__bullets_start,
    ET_bullet01,
    ET_bullet02,
    ET_bullet03,
    ET_bullet04,
    ET_bullet05,
    ET_bullet_tank,
    ET__bullets_end = ET_bullet_tank,
    
    ET_mummy,
    
    ET_pickup_a,
    ET_pickup_b,
    ET_pickup_c,
    ET_pickup_m,
    ET_pickup_s,

    UI_special_ammo,
    
    ET__count
};

struct
{
    int type;
    float scale;
    char *filename;
    int x;
    int y;
} sprite_files[] =
{
    { ET__none,            0.0f,  "", 0, 0 },
    { ET_player,           1.0f, "../dat/art/player.png",              10,  15  },
    { ET_ground,           1.0f, "../dat/art/ground.png",              16,  16  },
    { ET_ground2,          1.0f, "../dat/art/ground2.png",             16,  16  },
    { ET_ground3,          1.0f, "../dat/art/ground3.png",             16,  16  },
    { ET_bullet00,         1.0f, "../dat/art/bullet00.png",             3,   3  },
    { ET_bullet01,         0.6f, "../dat/art/bullet01.png",             5,   5  },
    { ET_bullet02,         1.0f, "../dat/art/bullet02.png",             4,   4  },
    { ET_bullet03,         0.4f, "../dat/art/bullet03.png",             5,   5  },
    { ET_bullet04,         0.6f, "../dat/art/bullet04.png",             5,   5  },
    { ET_bullet05,         0.6f, "../dat/art/bullet05.png",             6,   6  },
    { ET_bullet_tank,      1.5f, "../dat/art/bullet_tank.png",         12,   5  },
    { ET_mummy,            1.0f, "../dat/art/mummy.png",               10,  13  },
    { ET_pickup_a,         0.5f, "../dat/art/pickup_a.png",            14,  14  },
    { ET_pickup_b,         0.5f, "../dat/art/pickup_b.png",            14,  14  },
    { ET_pickup_c,         0.5f, "../dat/art/pickup_c.png",            14,  14  },
    { ET_pickup_m,         0.5f, "../dat/art/pickup_m.png",            14,  14  },
    { ET_pickup_s,         0.5f, "../dat/art/pickup_s.png",            14,  14  },
    { UI_special_ammo,     1.0f, "../dat/art/ui_special_ammo.png",     16,  16  },
    
};

struct
{
    float zoom;
    float player_speed;
    vec   player_start_pos;
    float bullet_speed;
    int mummy_hp;
    float mummy_speed;
    int bullet;
    float fire_rate;
} cfg =
{
    2.5f,
    100.0f,
    (vec){180, 180},
    3.0f,
    5,
    25.0f,
    ET_bullet05,
    5.0f
};

#define TILE_ENTITY_MAX   3000
#define BULLET_ENTITY_MAX 4000
#define MAX_ENTITIES      5000

extern entity ent[MAX_ENTITIES];

extern entity_id max_entity_id;
extern entity_id max_tile_id;
extern entity_id max_bullet_id;

extern int program_mode;
extern double world_timer;
// extern float zoom;
#endif
