#include "game_funcs.h"

monster monster_info[MONSTER_COUNT + 1] =
{
//     type       hp     power speed
//  { ET__none,   999,   99,   99 },
    { ET_mummy,     50,   10,   10 },
    { ET_spider,    10,   10,   20 },
    { ET_alien,     35,   10,   15 },
    { ET_robot,     100,   10,    5 }
};

inline entity_id monster_id(int i)
{
    return MONSTER_ENTITY_MIN + i;
}

Bool is_monster(int entity_type)
{
    return (entity_type >= ET__monsters_start || entity_type <= ET__monsters_end);
}

static int get_monster_index(int type)
{
    if(!is_monster(type))  return -1;
    return type - ET__monsters_start;
}

monster get_monster_info(int type)
{
    int index = get_monster_index(type);
    return monster_info[index];
}
