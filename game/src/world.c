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
    for(i=0; i<n; ++i)
        d[i] = val;
    return data;
}

void create_monster(int type, vec pos)
{
    entity_id id = allocate_monster();
    game_memset(&ent[id], 0, sizeof(ent[id]));

    monster mi = get_monster_info(type);
    ent[id].hp = mi.hp;
    ent[id].speed = mi.speed;
    ent[id].radius = 30;
    ent[id].pos   = pos;
    ent[id].type  = type;
    ent[id].valid = 1;
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

void world_init(void)
{
    // player
    if (program_mode == MODE_game) {
        player_id = 0;
        ent[player_id].pos.x = 0;
        ent[player_id].pos.y = 0;
        ent[player_id].type = ET_player;
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
    for (i = 0; i<25; ++i) {
        int type = get_random_int_range(ET__monsters_start, ET__monsters_end);
        create_monster_in_random_side(type, cfg.player_start_pos);
    }
    
}

int create_entity(int type, vec pos)
{
    entity_id id = allocate_entity();
    game_memset(&ent[id], 0, sizeof(ent[id]));
    ent[id].pos = pos;
    ent[id].valid = 1;
    ent[id].type = type;

    return id;
}

int create_tile(int type, vec pos)
{
    entity_id id = allocate_tile();
    game_memset(&ent[id], 0, sizeof(ent[id]));
    ent[id].pos = pos;
    ent[id].valid = 1;
    ent[id].type = type;

    return id;
}


int create_bullet(int type, vec pos)
{
    //assert(type >= ET__bullets_start && type <= ET__bullets_end);
    entity_id id = allocate_bullet();
    game_memset(&ent[id], 0, sizeof(ent[id]));
    ent[id].pos = pos;
    ent[id].valid = 1;
    ent[id].type = type;

    return id;
}

int world_to_tile_pos(float world_pos)
{
    return world_pos / TILE_SIZE;
}
