#ifndef INCLUDE_GAME_FUNCS_H
#define INCLUDE_GAME_FUNCS_H

#include "game_data.h"

// main.c
extern vec vec2(float x, float y);
extern vec vec_rotate_u(vec v, vec p, vec u);
extern int get_random_int(void);
extern int get_random_int_range(int min, int max);
extern vec screen_to_world(float x, float y);
extern vec get_random_spawn_pos(vec origin);
extern obb ent_to_obb(entity_id id);
extern box ent_to_box(entity_id id);
extern range ent_to_range(entity_id id);

// monster.c
extern monster get_monster_info(int type);
extern entity_id monster_id(int i);

// world.c
extern void world_init(void);
extern int  create_entity(int type, vec pos);
extern void ent_to_world(float pos[2], entity_id id);
extern void process_tick_raw(float dt);
extern int  world_to_tile_pos(float world_pos);
extern void create_monster(int type, vec pos);
extern void create_monster_in_random_side(int type, vec origin);
extern int  create_bullet(int type, vec pos);
extern entity_id create_entity(int type, vec pos);
extern entity_id create_tile(int type, vec pos);

// entity.c
extern entity_id allocate_entity(void);
extern entity_id allocate_player(void);
extern entity_id allocate_tile(void);
extern entity_id allocate_bullet(void);
extern entity_id allocate_monster(void);

// render.c
extern vec  get_scaled_sprite_size(int type);
extern void render_game(void);
extern void render_init(void);
extern void render_tiles(void);

// main.c
extern void toggle_menu(void);
extern void draw_menu_view(void);
extern void process_menu_input(void);
extern void menu_init(void);
#endif
