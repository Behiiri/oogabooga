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
    MODE_menu
};

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
    ET__monsters_end = ET_spider,
    
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

typedef struct
{
    float zoom;
    float player_speed;
    vec   player_start_pos;
    float bullet_speed;
    int   mummy_hp;
    float mummy_speed;
    int   bullet;
    int   fire_rate;
} config;

extern config cfg;

#define TILE_ENTITY_MAX   3000
#define BULLET_ENTITY_MAX 4000
#define MAX_ENTITIES      5000

extern entity ent[MAX_ENTITIES];

extern entity_id max_entity_id;
extern entity_id max_tile_id;
extern entity_id max_bullet_id;

extern int program_mode;
extern double world_timer;

extern float dt;
extern double bullet_fire_cd;

extern entity_id player_id;
extern int kill_count;
#endif
