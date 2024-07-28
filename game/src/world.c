#include "game_funcs.h"
#include <string.h> // memset

static double world_timer = 0;

entity_id player_id;

void cteate_monster(int type, vec origin, int radius, int hp)
{
    int x = get_random_int_range(-radius, radius) + origin.x;
    int y = get_random_int_range(-radius, radius) + origin.y;

    vec pos = {x, y};
    
    entity_id id = create_entity(type, pos);
    ent[id].hp = hp;
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
    int sz_x = sprite_files[ET_ground3].x;
    int sz_y = sprite_files[ET_ground3].y;
    int i, j;
    for (i = 0; i<50;++i)
        for (j = 0; j<50;++j) {
            vec pos = {i * sz_x, j * sz_y};
            int id = create_tile(ET_ground3, pos);
        }

    // mummy's ET_mummy
    for (i = 0; i<20;++i)
    {
        cteate_monster(ET_mummy, cfg.player_start_pos, 250, cfg.mummy_hp);
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
    assert(type >= ET__bullets_start && type <= ET__bullets_end);
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

void process_tick_raw(float dt)
{
}
