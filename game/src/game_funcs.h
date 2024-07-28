#ifndef INCLUDE_GAME_FUNCS_H
#define INCLUDE_GAME_FUNCS_H

#include "game_data.h"

// #define FORWARD_DECLARE(name) typedef struct name name;

// // oogabooga.c
// FORWARD_DECLARE(Gfx_Image);
// FORWARD_DECLARE(string);
// FORWARD_DECLARE(Allocator);
// #define fixed_string STR
// #define STR(s) ((string){ length_of_null_terminated_string((const char*)s), (u8*)s })
// extern Gfx_Image *load_image_from_disk(string path, Allocator allocator);
// extern u64 length_of_null_terminated_string(const char* cstring);

// main.c
extern int get_random_int();
extern int get_random_int_range(int min, int max);
extern Vector2 world_to_screen(float x, float y);

// world.c
extern void world_init(void);
extern int  create_entity(int type, vec pos);
extern void ent_to_world(float pos[2], entity_id id);
extern void process_tick_raw(float dt);
extern int  world_to_tile_pos(float world_pos);
extern void cteate_monster(int type, vec origin, int radius, int hp);
extern int create_bullet(int type, vec pos);
extern entity_id create_entity(int type, vec pos);
extern entity_id create_tile(int type, vec pos);

// entity.c
extern entity_id allocate_entity(void);
extern entity_id allocate_player(void);
extern entity_id allocate_tile(void);
extern entity_id allocate_bullet(void);


extern void render_init(void);
extern void render_tiles(void);
#endif
