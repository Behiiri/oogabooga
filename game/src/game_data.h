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

enum
{
    LEFT,
    UP,
    RIGHT,
    DOWN
};

typedef struct
{
    vec pos;
    vec velocity;
    vec rotation;
    int hp;
    int type;
    uint32 valid;
    float speed;
} entity; // 40

enum
{
    MODE_game,
    MODE_menu
};

#if 0
// entity types
enum
{
    ET__none,
    ET_player,
    ET_ground,
    ET_ground2,
    ET_ground3,
    ET_ground4,

    ET__bullets_start,
    ET_bullet00 = ET__bullets_start,
    ET_bullet01,
    ET_bullet02,
    ET_bullet03,
    ET_bullet04,
    ET_bullet05,
    ET_bullet_tank,
    ET__bullets_end = ET_bullet_tank,

    ET__monsters_start,
    ET_mummy = ET__monsters_start,
    ET_spider,
    ET_alien,
    ET_robot,
    ET__monsters_end = ET_robot,
    
    ET_pickup_a,
    ET_pickup_b,
    ET_pickup_c,
    ET_pickup_m,
    ET_pickup_s,

    UI_special_ammo,
    UI_fire_rate,
    UI_skull,
    
    ET__count
};
#endif

//    type               size    layer path                              x    y
#define ENTITY_TYPES_X                                                  \
    X(ET__none,          0.0f,    0,   "../dat/art/error.png",            0,  0) \
    X(ET_player,         1.0f,   21,   "../dat/art/player.png",          10, 15) \
    X(ET_ground,         1.0f,    1,   "../dat/art/ground.png",          16, 16) \
    X(ET_ground2,        1.0f,    1,   "../dat/art/ground2.png",         16, 16) \
    X(ET_ground3,        1.0f,    1,   "../dat/art/ground3.png",         16, 16) \
    X(ET_ground4,        1.0f,    1,   "../dat/art/ground4.png",         16, 16) \
    X(ET_bullet00,       1.0f,   11,   "../dat/art/bullet00.png",         3,  3) \
    X(ET_bullet01,       0.6f,   11,   "../dat/art/bullet01.png",         5,  5) \
    X(ET_bullet02,       1.0f,   11,   "../dat/art/bullet02.png",         4,  4) \
    X(ET_bullet03,       0.4f,   11,   "../dat/art/bullet03.png",         5,  5) \
    X(ET_bullet04,       0.6f,   11,   "../dat/art/bullet04.png",         5,  5) \
    X(ET_bullet05,       0.6f,   11,   "../dat/art/bullet05.png",         6,  6) \
    X(ET_bullet_tank,    1.5f,   11,   "../dat/art/bullet_tank.png",     12,  5) \
    X(ET_mummy,          1.0f,   31,   "../dat/art/mummy.png",           10, 13) \
    X(ET_spider,         1.0f,   31,   "../dat/art/spider.png",          16, 15) \
    X(ET_alien,          1.0f,   31,   "../dat/art/alien.png",            9, 16) \
    X(ET_robot,          1.0f,   31,   "../dat/art/robot.png",           16, 15) \
    X(ET_pickup_a,       0.5f,    1,   "../dat/art/pickup_a.png",        14, 14) \
    X(ET_pickup_b,       0.5f,    1,   "../dat/art/pickup_b.png",        14, 14) \
    X(ET_pickup_c,       0.5f,    1,   "../dat/art/pickup_c.png",        14, 14) \
    X(ET_pickup_m,       0.5f,    1,   "../dat/art/pickup_m.png",        14, 14) \
    X(ET_pickup_s,       0.5f,    1,   "../dat/art/pickup_s.png",        14, 14) \
    X(ET_gem_green,      0.5f,    1,   "../dat/art/pickup_s.png",        14, 14) \
    X(UI_special_ammo,   1.0f,    1,   "../dat/art/ui_special_ammo.png", 16, 16) \
    X(UI_fire_rate,      1.0f,    1,   "../dat/art/ui_fire_rate.png",    16, 16) \
    X(UI_skull,          0.625f,  1,   "../dat/art/ui_skull.png",        16, 16)

enum
{
#define X(name, scale, layer, filename, x, y) name,
    ENTITY_TYPES_X
#undef X
    ET__count
};

enum
{
    ET__bullets_start  = ET_bullet00,
    ET__bullets_end    = ET_bullet_tank,
    ET__monsters_start = ET_mummy,
    ET__monsters_end   = ET_robot
};


#define BULLETS_COUNT (ET__bullets_end - ET__bullets_start + 1)
#define MONSTER_COUNT (ET__monsters_end - ET__monsters_start + 1)

typedef struct
{
    int type;
    int hp;
    int power;
    int speed;
} monster;

enum
{
    BULLET_normal,
    BULLET_tank,
    BULLET_smart
};

enum // fire mode
{
    FM_normal,
    FM_two_bullets,
    FM_three_bullets,
    FM_four_bullets,
    FM_five_bullets,
    FM_cycle,
    FM_burst,
    FM_Spread
};

enum // weapon types
{
    WT_pistol,
    WT_secondary,
    WT_SpreadGun,
    WT__count
};

typedef struct
{
    int type;
    int bullet_type;
    int bullets_per_shot;
    float bullet_speed; // ??
    int fire_mode;
    int fire_rate;
} weapon;

extern weapon cur_weapon;
extern weapon weapon_info[];

typedef struct
{
    float zoom;
    float player_speed;
    vec   player_start_pos;
} config;

extern config cfg;

#define TILE_ENTITY_MAX    4096
#define BULLET_ENTITY_MAX  4352
#define MONSTER_ENTITY_MAX 8192
#define MAX_ENTITIES       65536

extern entity ent[MAX_ENTITIES];

extern entity_id max_entity_id;
extern entity_id max_tile_id;
extern entity_id max_bullet_id;
extern entity_id max_monster_id;

 
extern int program_mode;
extern double world_timer;

extern float dt;
extern double bullet_fire_cd;

extern entity_id player_id;
extern int kill_count;

typedef struct
{
    int   weapon;
    int   hp;
    float speed;
    // char* name;
    // Gfx_Image* tex;
    // Gfx_Image* icon;
} character;

enum
{
    CH_001,
    CH_002,
};
    
#endif

