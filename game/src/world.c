#include "game_funcs.h"
#include <stdlib.h> // rand
#include <math.h> // cos sin

#ifndef M_PI
#define M_PI  3.14159265358979323846264f
#endif

static double world_timer = 0;

entity_id player_id;

static void *game_memset(void *data, int c, size_t n)
{
    uint8 *d  = data;
    uint8 val = (uint8)c;

    int i;
    for (i=0; i<n; ++i)
        d[i] = val;
    return data;
}

inline void set_entity_size(int id)
{
    ent[id].size = get_scaled_sprite_size(ent[id].type);
}


void world_init(void)
{
    // player
    if (program_mode == MODE_game) {
        player_id = 0;
        ent[player_id].valid = 1;
        ent[player_id].pos.x = 0;
        ent[player_id].pos.y = 0;
        ent[player_id].type = ET_player;
        set_entity_size(player_id);
    }

    // ground tiles
    int sz_x = TILE_SIZE;
    int sz_y = TILE_SIZE;
    int i, j;
    for (i = 0; i<50; ++i)
        for (j = 0; j<50;++j) {
            vec pos = {i * sz_x, j * sz_y};
            int id = create_tile(ET_ground3, pos);
        }

    // monsters
#if 1
    for (i = 0; i<100; ++i) {
        int type = get_random_int_range(ET__monsters_start, ET__monsters_end);
        create_monster_in_random_side(type, cfg.player_start_pos);
    }
#else
    int type = ET_robot;
    create_monster_in_random_side(type, cfg.player_start_pos);
#endif
}

int world_to_tile_pos(float world_pos)
{
    return world_pos / TILE_SIZE;
}

int create_entity(int type, vec pos)
{
    entity_id id = allocate_entity();
    game_memset(&ent[id], 0, sizeof(ent[id]));
    ent[id].pos = pos;
    ent[id].valid = 1;
    ent[id].type = type;
    set_entity_size(id);
    return id;
}

int create_tile(int type, vec pos)
{
    entity_id id = allocate_tile();
    game_memset(&ent[id], 0, sizeof(ent[id]));
    ent[id].pos = pos;
    ent[id].valid = 1;
    ent[id].type = type;
    set_entity_size(id);
    return id;
}


int create_bullet(int type, vec pos)
{
    entity_id id = allocate_bullet();
    game_memset(&ent[id], 0, sizeof(ent[id]));
    ent[id].pos = pos;
    ent[id].valid = 1;
    ent[id].type = type;
    set_entity_size(id);
    return id;
}


void create_monster(int type, vec pos)
{
    entity_id id = allocate_monster();
    game_memset(&ent[id], 0, sizeof(ent[id]));

    monster mi     = get_monster_info(type);
    ent[id].hp     = mi.hp;
    ent[id].speed  = mi.speed;
    ent[id].radius = 30;
    ent[id].u      = vec2(1,0);
    ent[id].pos    = pos;
    ent[id].type   = type;
    ent[id].valid  = 1;
    set_entity_size(id);
}

void create_monster_in_random_side(int type, vec origin)
{
    vec pos = get_random_spawn_pos(origin);
    create_monster(type, pos);
}

void create_monster_in_distance(int type, vec origin, int dist_min, int dist_max)
{
    double angle = ((double)rand() / RAND_MAX) * 2 * M_PI;
    double radius = dist_min + ((double)rand() / RAND_MAX) * (dist_max - dist_min);

    double x = origin.x + radius * cos(angle);
    double y = origin.y + radius * sin(angle);
    vec pos = {x, y};

    create_monster(type, pos);
}

game_text damage_texts[MAX_DAMAGE_TEXTS];
int max_damage_text_id;
void add_game_text(vec pos, int dmg, double duration, int color_id)
{
    int i;
    for (i=0; i<MAX_DAMAGE_TEXTS; ++i)
        if (!damage_texts[i].valid) {
            if (i >= max_damage_text_id)
                max_damage_text_id = i+1;
            game_text *t = &damage_texts[i];
            t->valid = True;
            t->pos = pos;
            t->dmg = dmg;
            t->color = color_id;
            t->create_time = world_timer;
            t->duration = duration;
            return;
        }
 
}

void process_tick_raw(float dt)
{
    int i;
    for (i=0; i<max_damage_text_id; ++i) {
        if(damage_texts[i].valid) {
            game_text *t = &damage_texts[i];
            if(world_timer - t->create_time > t->duration) {
                t->valid = 0; continue;
            }
            t->pos.y += 50*dt;
        }
    }
}
