#ifndef INCLUDE_GAME_FUNCS_H
#define INCLUDE_GAME_FUNCS_H

#include "game_data.h"

// main.c
extern vec vec2(float x, float y);
extern int get_random_int(void);
extern int get_random_int_range(int min, int max);
extern vec screen_to_world(float x, float y);

// monster
extern monster get_monster_info(int type);

// world.c
extern void world_init(void);
extern int  create_entity(int type, vec pos);
extern void ent_to_world(float pos[2], entity_id id);
extern void process_tick_raw(float dt);
extern int  world_to_tile_pos(float world_pos);
extern void create_monster(int type, vec origin, int radius);
extern int  create_bullet(int type, vec pos);
extern entity_id create_entity(int type, vec pos);
extern entity_id create_tile(int type, vec pos);

// entity.c
extern entity_id allocate_entity(void);
extern entity_id allocate_player(void);
extern entity_id allocate_tile(void);
extern entity_id allocate_bullet(void);

// render.c
extern void render_game(void);
extern void render_init(void);
extern void render_tiles(void);

// main.c
extern void toggle_menu(void);
extern void draw_menu_view(void);
extern void process_menu_input(void);
extern void menu_init(void);
#endif
