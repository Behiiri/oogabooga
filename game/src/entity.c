#include "game_funcs.h"
//#include <stdlib.h>
#include <string.h>

// Layout of entities on the end array
// ent[0] -> player
// ent[1] - ent[TILE_ENTITY_MAX] -> tiles
// ent[TILE_ENTITY_MAX] - ent[BULLET_ENTITY_MAX] -> bullets
// ent[BULLET_ENTITY_MAX] - ent[MAX_ENTITIES] -> everything else

entity ent[MAX_ENTITIES];
static entity_id max_tile_id = 0;
entity_id max_bullet_id = TILE_ENTITY_MAX - 1;
entity_id max_entity_id = BULLET_ENTITY_MAX - 1;

entity_id allocate_tile(void)
{
    int i;
    for (i=1; i < TILE_ENTITY_MAX; ++i)
        if(!ent[i].valid) {
            if(i >= max_tile_id)
                max_tile_id = (entity_id) i+1;
            ent[i].valid = True;
            return (entity_id) i;
        }
    return 0;
}

entity_id allocate_bullet(void)
{
    int i;
    for (i=TILE_ENTITY_MAX; i < BULLET_ENTITY_MAX; ++i)
        if(!ent[i].valid) {
            if(i >= max_bullet_id)
                max_bullet_id = (entity_id) i+1;
            ent[i].valid = True;
            return (entity_id) i;
        }
    return 0;
}

entity_id allocate_entity(void)
{
    int i;
    for (i=BULLET_ENTITY_MAX; i < MAX_ENTITIES; ++i)
        if(!ent[i].valid) {
            if(i >= max_entity_id)
                max_entity_id = (entity_id) i+1;
            ent[i].valid = True;
            return (entity_id) i;
        }
    return 0;
}
