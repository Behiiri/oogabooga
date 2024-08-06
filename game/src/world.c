#include "game_funcs.h"
#include <string.h> // memset
#include <stdlib.h> // rand
#include <math.h> // rand

#define M_PI 3.14159

static double world_timer = 0;

entity_id player_id;

// double distance(float x1, float y1, float x2, float y2) {
//     return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
// }

void create_monster(int type, vec pos)
{
    monster mi = get_monster_info(type);
    entity_id id = create_entity(type, pos);
    ent[id].hp = mi.hp;
    ent[id].speed = mi.speed;
}

void create_monster_in_random_side(int type, vec origin)
{
    vec pos = get_random_pos_on_screen_sides(origin);
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
    for (i = 0; i<20; ++i)
    {
        int type = get_random_int_range(ET__monsters_start, ET__monsters_end);
        ///create_monster(type, cfg.player_start_pos, 250);
        create_monster_in_random_side(type, cfg.player_start_pos);
    }
    
}

int create_entity(int type, vec pos)
{
    int id = allocate_entity();
    memset(&ent[id], 0, sizeof(ent[id]));
    ent[id].pos = pos;
    ent[id].valid = 1;
    ent[id].type = type;

    return id;
}

int create_tile(int type, vec pos)
{
    int id = allocate_tile();
    memset(&ent[id], 0, sizeof(ent[id]));
    ent[id].pos = pos;
    ent[id].valid = 1;
    ent[id].type = type;

    return id;
}


int create_bullet(int type, vec pos)
{
    //assert(type >= ET__bullets_start && type <= ET__bullets_end);
    int id = allocate_bullet();
    memset(&ent[id], 0, sizeof(ent[id]));
    ent[id].pos = pos;
    ent[id].valid = 1;
    ent[id].type = type;

    return id;
}

int world_to_tile_pos(float world_pos)
{
    return world_pos / TILE_SIZE;
}
