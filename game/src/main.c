#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "game_funcs.h"
#include "render.c" // render.c needs to access some engine specific types
#include "menu.c"

#if JUMBO_BUILD
#include "weapon.c"
#include "character.c"
#include "monster.c"
#include "world.c"
#include "entity.c"
#endif

static int program_mode = MODE_game;
static float dt;
static vec player_pos;
static vec camera_pos;
static character player_char;
static weapon cur_weapon;
static int kill_count;

static veci inner_spawn_box;
static veci outer_spawn_box;

config cfg = {
    .zoom = 2.5f,
    .player_speed = 75.0f,
    .player_start_pos = (vec){0, 0},
    .max_pickup_time = 10.0f,
    .pickup_flash_dur = 3.0f
};

vec vec2(float x, float y)
{
    vec v = {x, y};
    return v;
}


vec vec_rotate_u(vec v, vec p, vec u)
{
    v.x -= p.x;
    v.y -= p.y;

    float rx = v.x * u.x - v.y * u.y;
    float ry = v.x * u.y + v.y * u.x;

    v.x = rx + p.x;
    v.y = ry + p.y;

    return v;
}

vec vec_rotate(vec v, float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    v.x = v.x * c - v.y * s;
    v.y = v.x * s + v.y * c;
    return v;
}

float dot(vec a, vec b) {
    return a.x*b.x + a.y*b.y;
}

float distance(vec a, vec b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrt(dx*dx + dy*dy);
}

range ent_to_range(entity_id id)
{
    range range;
    entity *en = &ent[id];
    range.c = vec2(en->x + en->w/2, en->y + en->h/2);
    range.r = (en->w > en->h ? en->w : en->h);
    return range;
}

obb ent_to_obb(entity_id id)
{
    obb o;
    vec s = ent[id].size;
    vec p = ent[id].pos;
    o.u = ent[id].u;
    o.c = vec2(p.x + s.x/2, p.y + s.y/2);
    //o.c = vec_rotate_u(o.c, p, o.u);
    o.e = vec2(s.x/2, s.y/2);
    return o;
}

Bool check_range_collision(range a, range b)
{
    return distance(a.c, b.c) <= a.r + b.r;
}

Bool check_range_collision_by_id(entity_id id_a, entity_id id_b)
{
    range a = ent_to_range(id_a);
    range b = ent_to_range(id_b);
    return check_range_collision(a, b);
}

Bool check_obb_collision(obb* o1, obb* o2) {
    o1->c = vec_rotate_u(o1->c, vec2(o1->c.x-o1->e.x, o1->c.y-o1->e.y), o1->u);
    o2->c = vec_rotate_u(o2->c, vec2(o2->c.x-o2->e.x, o2->c.y-o2->e.y), o2->u);
    // monsters has no rotation so thier dir is (0,0)
    vec a1 = o1->u;
    vec a2 = { -o1->u.y, o1->u.x };
    // vec a3 = { 1, 0 };
    // vec a4 = { 0, 1 };
    vec a3 = o2->u;
    vec a4 = { -o2->u.y, o2->u.x };
    // edge lengths
    vec l1 = o1->e;
    vec l2 = o2->e;

    // vector between pivots
    vec l = { o1->c.x - o2->c.x, o1->c.y - o2->c.y };

    float r1, r2, r3, r4;

    // project to a1
    r1 = l1.x * fabs(dot(a1, a1));
    r2 = l1.y * fabs(dot(a2, a1));
    r3 = l2.x * fabs(dot(a3, a1));
    r4 = l2.y * fabs(dot(a4, a1));
    if (r1 + r2 + r3 + r4 <= fabs(dot(l, a1)))
        return False;

    // project to a2
    r1 = l1.x * fabs(dot(a1, a2));
    r2 = l1.y * fabs(dot(a2, a2));
    r3 = l2.x * fabs(dot(a3, a2));
    r4 = l2.y * fabs(dot(a4, a2));
    if (r1 + r2 + r3 + r4 <= fabs(dot(l, a2)))
        return False;

    // project to a3
    r1 = l1.x * fabs(dot(a1, a3));
    r2 = l1.y * fabs(dot(a2, a3));
    r3 = l2.x * fabs(dot(a3, a3));
    r4 = l2.y * fabs(dot(a4, a3));
    if (r1 + r2 + r3 + r4 <= fabs(dot(l, a3)))
        return False;

    // project to a4
    r1 = l1.x * fabs(dot(a1, a4));
    r2 = l1.y * fabs(dot(a2, a4));
    r3 = l2.x * fabs(dot(a3, a4));
    r4 = l2.y * fabs(dot(a4, a4));
    if (r1 + r2 + r3 + r4 <= fabs(dot(l, a4)))
        return False;

    return True;
}

Bool check_obb_collision_by_id(entity_id a, entity_id b)
{
    obb o1 = ent_to_obb(a);
    obb o2 = ent_to_obb(b);
    return check_obb_collision(&o1, &o2);
}


Bool resolve_overlap(int ent_a, int ent_b)
{
    entity *a = &ent[ent_a];
    entity *b = &ent[ent_b];

    float ox = ((a->x + a->w) - b->x) < ((b->x + b->w) - a->x)
        ? ((a->x + a->w) - b->x) : ((b->x + b->w) - a->x);
    float oy = ((a->y + a->h) - b->y) < ((b->y + b->h) - a->y)
        ? ((a->y + a->h) - b->y) : ((b->y + b->h) - a->y);

    if (ox < 0 || oy < 0 || ox > a->w+b->w || oy > a->h+b->h)
        return false;
    vec p = player_pos;
    if (ox < oy) {
        if (a->x < b->x) {
            if (a->x > p.x) b->x += ox;
            else            a->x -= ox;
        } else {
            if (a->x > p.x) a->x += ox;
            else            b->x -= ox;
        }
    } else {
        if (a->y < b->y) {
            if (a->y > p.y) b->y += oy;
            else            a->y -= oy;
        } else {
            if (a->y > p.y) a->y += oy;
            else            b->y -= oy;
        }
    }

    float mx = p.x / 2;
    float my = p.y / 2;

    float acx = a->x + a->w / 2;
    float acy = a->y + a->h / 2;
    float bcx = b->x + b->w / 2;
    float bcy = b->y + b->h / 2;

    float adx = mx - acx;
    float ady = my - acy;
    float bdx = mx - bcx;
    float bdy = my - bcy;

    float la = sqrt(adx * adx + ady * ady);
    float lb = sqrt(bdx * bdx + bdy * bdy);

    if (la > 0) {
        adx /= la;
        ady /= la;
    }

    if (lb > 0) {
        bdx /= lb;
        bdy /= lb;
    }

    float sa = a->speed;
    float sb = b->speed;

    float dx = (b->pos.x - a->pos.x);
    float dy = (b->pos.y - a->pos.y);
    float len = sqrt(dx * dx + dy * dy);

    if (len > 0) {
        dx /= len;
        dy /= len;
    }

    if (sa >= sb) {
        b->x += dx * (sa - sb) * dt;
        b->y += dy * (sa - sb) * dt;
    } else if (sb > sa) {
        a->x -= dx * (sb - sa) * dt;
        a->y -= dy * (sb - sa) * dt;
    }

    return true;
}

int get_random_int(void) {
    return rand();
}

int get_random_int_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

float get_random_float_range(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

Vector2 vec_to_v2(vec v)
{
    return (Vector2){v.x, v.y};
}

Bool almost_equals(float a, float b, float epsilon) {
    return fabs(a - b) <= epsilon;
}

Bool animate_f32_to_d(float* v, float d, float dt, float t)
{
    *v += (d - *v) * (1.0 - pow(2.0f, -t * dt));
    if (almost_equals(*v, d, 0.1f)) {
        *v = d;
        return True; // reached
    }
    return False;
}

void animate_v2_to_d(vec* v, vec d, float dt, float t)
{
    animate_f32_to_d(&(v->x), d.x, dt, t);
    animate_f32_to_d(&(v->y), d.y, dt, t);
}

Bool move_towards(vec* v, vec d, float dt, float t) {
    vec dir;
    dir.x = d.x - v->x;
    dir.y = d.y - v->y;

    float dist = sqrt(dir.x * dir.x + dir.y * dir.y);

    if (dist < dt * t) {
        v->x = d.x;
        v->y = d.y;
        return True;
    } else {
        v->x += (dir.x / dist) * dt * t;
        v->y += (dir.y / dist) * dt * t;
        return False;
    }
}

vec screen_to_world(float x, float y)
{
    Matrix4 proj = draw_frame.projection;
    Matrix4 view = draw_frame.camera_xform;
    float w = window.width;
    float h = window.height;

    float norm_x = (x / (w * 0.5f)) - 1.0f;
    float norm_y = (y / (h * 0.5f)) - 1.0f;

    Vector4 world_pos = v4(norm_x, norm_y, 0, 1);
    world_pos = m4_transform(m4_inverse(proj), world_pos);
    world_pos = m4_transform(view, world_pos);

    return (vec) {world_pos.x, world_pos.y};
}

vec get_random_pos_on_side(vec origin, int side)
{
    int x, y;
    int dx = inner_spawn_box.x + rand() % (outer_spawn_box.x - inner_spawn_box.x);
    int dy = inner_spawn_box.y + rand() % (outer_spawn_box.y - inner_spawn_box.y);
    int ddx = rand() % (outer_spawn_box.x*2);
    int ddy = rand() % (outer_spawn_box.y*2);
    
    switch(side) {
        case LEFT: // 0
            x = origin.x - dx;
            y = origin.y - outer_spawn_box.y + ddy;
            break;
        case UP: // 1
            x = origin.x - outer_spawn_box.x + ddx;
            y = origin.y + dy;
            break;
        case RIGHT: // 2
            x = origin.x + dx;
            y = origin.y - outer_spawn_box.y + ddy;
            break;
        case DOWN: // 3
            x = origin.x - outer_spawn_box.x + ddx;
            y = origin.y - dy;
            break;
    }

    vec v = {x, y};
    return v;
}

vec get_random_spawn_pos(vec origin)
{
    int side = rand() % 4;
    return get_random_pos_on_side(origin, side);
}

vec reposition_monster(vec player_pos, vec pos)
{
    int side = -1;
    float lb = player_pos.x - inner_spawn_box.x;
    float rb = player_pos.x + inner_spawn_box.x;
    float tb = player_pos.y + inner_spawn_box.y;
    float bb = player_pos.y - inner_spawn_box.y;

    if (pos.x > rb) side = LEFT;
    if (pos.y < bb) side = UP;
    if (pos.x < lb) side = RIGHT;
    if (pos.y > tb) side = DOWN;
    
    return get_random_pos_on_side(player_pos, side);
}

void next_weapon(void)
{
    player_char.weapon++;
    if (player_char.weapon == WT__count)
        player_char.weapon = WT_pistol;
    cur_weapon = weapon_info[player_char.weapon];
    bullet_fire_cd = 1.0f/cur_weapon.fire_rate;
}

int special_ammo = 10;
void fire_bullet(void)
{
    float mouse_x = input_frame.mouse_x;
    float mouse_y = input_frame.mouse_y;
    vec mouse_pos = screen_to_world(mouse_x, mouse_y);

    float offset_x = ent[player_id].size.x/2;
    float offset_y = ent[player_id].size.y/2;

    if (cur_weapon.fire_mode == FM_normal) {
        vec dir = {mouse_pos.x - (player_pos.x + offset_x), mouse_pos.y - (player_pos.y + offset_y)};
        float length = sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length == 0) length = 1;
        vec unit_dir = {dir.x / length, dir.y / length};
        vec velocity = {unit_dir.x * cur_weapon.bullet_speed, unit_dir.y * cur_weapon.bullet_speed};

        int id = create_bullet(cur_weapon.bullet_type, (vec){player_pos.x + offset_x, player_pos.y + offset_y});
        ent[id].v = velocity;
        ent[id].u = (vec){unit_dir.x, unit_dir.y};
        return;
    }

    if (cur_weapon.fire_mode == FM_spread) {
        vec dir = {mouse_pos.x - (player_pos.x + offset_x), mouse_pos.y - (player_pos.y + offset_y)};
        float length = sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length == 0) length = 1;
        vec unit_dir = {dir.x / length, dir.y / length};

        int id;
        vec bullet_pos;

        if (cur_weapon.bullets_per_shot % 2) // odds
        {
            bullet_pos = vec2(player_pos.x + offset_x, player_pos.y + offset_y);
            id = create_bullet(cur_weapon.bullet_type, bullet_pos);
            ent[id].v = vec2(unit_dir.x * cur_weapon.bullet_speed, unit_dir.y * cur_weapon.bullet_speed);
            ent[id].u = unit_dir;
        } else if (cur_weapon.bullets_per_shot >= 2)
        {
            bullet_pos = vec2(player_pos.x + offset_x, player_pos.y + offset_y);
            vec u = vec_rotate(unit_dir, M_PI/24);
            id = create_bullet(cur_weapon.bullet_type, bullet_pos);
            ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
            ent[id].u = u;

            u = vec_rotate(unit_dir, -M_PI/24);
            id = create_bullet(cur_weapon.bullet_type, bullet_pos);
            ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
            ent[id].u = u;

            if (cur_weapon.bullets_per_shot >= 4) {
                vec u = vec_rotate(unit_dir, M_PI/12);
                id = create_bullet(cur_weapon.bullet_type, bullet_pos);
                ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
                ent[id].u = u;

                u = vec_rotate(unit_dir, -M_PI/12);
                id = create_bullet(cur_weapon.bullet_type, bullet_pos);
                ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
                ent[id].u = u;
            }

            return;
        }

        if (cur_weapon.bullets_per_shot >= 3) {
            vec u = vec_rotate(unit_dir, M_PI/12);
            id = create_bullet(cur_weapon.bullet_type, bullet_pos);
            ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
            ent[id].u = u;

            u = vec_rotate(unit_dir, -M_PI/12);
            id = create_bullet(cur_weapon.bullet_type, bullet_pos);
            ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
            ent[id].u = u;

            if (cur_weapon.bullets_per_shot >= 5) {
                u = vec_rotate(unit_dir, M_PI/6);
                id = create_bullet(cur_weapon.bullet_type, bullet_pos);
                ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
                ent[id].u = u;

                u = vec_rotate(unit_dir, -M_PI/6);
                id = create_bullet(cur_weapon.bullet_type, bullet_pos);
                ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
                ent[id].u = u;
            }
        }

        return;
    }

    if (cur_weapon.fire_mode == FM_cycle) {
        vec dir = {mouse_pos.x - (player_pos.x + offset_x), mouse_pos.y - (player_pos.y + offset_y)};
        float length = sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length == 0) length = 1;
        vec unit_dir = {dir.x / length, dir.y / length};

        int id;
        vec bullet_pos = vec2(player_pos.x + offset_x, player_pos.y + offset_y);
        vec u = unit_dir;

        id = create_bullet(cur_weapon.bullet_type, bullet_pos);
        ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
        ent[id].u = u;

        if (cur_weapon.bullets_per_shot >= 4) {

            u = vec2(-unit_dir.x, -unit_dir.y); // back
            id = create_bullet(cur_weapon.bullet_type, bullet_pos);
            ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
            ent[id].u = u;

            u = vec2(unit_dir.y, -unit_dir.x); // right
            id = create_bullet(cur_weapon.bullet_type, bullet_pos);
            ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
            ent[id].u = u;

            u = vec2(-unit_dir.y, unit_dir.x); // left
            id = create_bullet(cur_weapon.bullet_type, bullet_pos);
            ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
            ent[id].u = u;

            if (cur_weapon.bullets_per_shot >= 8) {
                unit_dir = vec_rotate_u(unit_dir, vec2(0,0), vec2(M_PI/4, M_PI/4));

                u = vec2(unit_dir.x, unit_dir.y); // org
                id = create_bullet(cur_weapon.bullet_type, bullet_pos);
                ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
                ent[id].u = u;

                u = vec2(-unit_dir.x, -unit_dir.y); // back
                id = create_bullet(cur_weapon.bullet_type, bullet_pos);
                ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
                ent[id].u = u;

                u = vec2(unit_dir.y, -unit_dir.x); // right
                id = create_bullet(cur_weapon.bullet_type, bullet_pos);
                ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
                ent[id].u = u;

                u = vec2(-unit_dir.y, unit_dir.x); // right
                id = create_bullet(cur_weapon.bullet_type, bullet_pos);
                ent[id].v = vec2(u.x * cur_weapon.bullet_speed, u.y * cur_weapon.bullet_speed);
                ent[id].u = u;
            }

        }

    }

}

double bullet_fire_cd = 0.5;
double last_fire_time = 0;
Bool can_fire(void)
{
    double now = world_timer;
    if (now - last_fire_time >= bullet_fire_cd)
    {
        last_fire_time = now;
        return True;
    }
    return False;
}

void decrease_fire_cd(float percent)
{
    bullet_fire_cd = bullet_fire_cd * (1.0f - percent/100.0f);
    if (bullet_fire_cd < 0.02f) bullet_fire_cd = 0.02f;
}

void increase_fire_rate(int percent)
{
    float  amount = cur_weapon.fire_rate / 100 * percent;
    cur_weapon.fire_rate = cur_weapon.fire_rate + amount;
    if (cur_weapon.fire_rate > 20) cur_weapon.fire_rate = 20;
    bullet_fire_cd = 1.0f/cur_weapon.fire_rate;
}

void update_view(void)
{
    draw_frame.projection = m4_make_orthographic_projection(window.width  * -0.5f, window.width  * 0.5f,
                                                            window.height * -0.5f, window.height * 0.5f, -1, 10);
    vec target_pos = ent[player_id].pos;
    animate_v2_to_d(&camera_pos, target_pos, dt, 15.0f);

    float scale_x = SCREEN_X/window.width;
    float scale_y = SCREEN_Y/window.height;
    float scale = scale_x > scale_y ? scale_x : scale_y;

    draw_frame.camera_xform = m4_make_scale(v3(1.0, 1.0, 1.0));
    draw_frame.camera_xform = m4_mul(draw_frame.camera_xform, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
    draw_frame.camera_xform = m4_mul(draw_frame.camera_xform, m4_make_scale(v3((scale)/cfg.zoom, (scale)/cfg.zoom, 1.0f)));
}

box ent_to_box(entity_id id)
{
    entity e = ent[id];
    float  w = e.size.x;
    float  h = e.size.y;
    return (box) {e.pos, (vec) { e.pos.x+w, e.pos.y+h }};
}

Bool check_box_collision_by_id(entity_id id_a, entity_id id_b)
{
    box a = ent_to_box(id_a);
    box b = ent_to_box(id_b);

    if (a.max.x < b.min.x || a.min.x > b.max.x) return False;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return False;

    return True;
}

void update_bullets(void)
{
    int i, j; // bullet, monster
    for (i=TILE_ENTITY_MAX; i<max_bullet_id; ++i) { // bullets
        if (ent[i].valid) {
            entity *en = &ent[i];
            float d = distance(en->pos, ent[player_id].pos);
            if (d > cur_weapon.fire_range) {
                en->valid = 0;
                if (en->type != ET_bullet_tank)
                    if (d > 150.0f)
                        en->valid = 0;
            }

            en->pos.x += en->v.x * dt * cur_weapon.bullet_speed; // TODO @Hardcoded value
            en->pos.y += en->v.y * dt * cur_weapon.bullet_speed;

            for (j=BULLET_ENTITY_MAX; j<=max_monster_id; ++j)
                if (ent[j].valid) {
                    if (check_range_collision_by_id(i, j))
                    {
                        if (check_obb_collision_by_id(i, j))
                        {
                            if (en->type == ET_bullet_tank) {
                                int dmg = 99;
                                ent[j].hp = ent[j].hp - dmg;
                                add_game_text(en->pos, dmg, 0.33, 2);
                            } else {
                                int dmg = get_random_int_range(cur_weapon.min_damage, cur_weapon.max_damage);
                                int color_id = 0;
                                if (dmg > 25) color_id = 1;
                                add_game_text(en->pos, dmg, 0.25, color_id);
                                ent[j].hp = ent[j].hp - dmg;
                                en->valid = 0;
                            }

                            ent[j].flash_dur = 0.065f;
                            break;
                        }
                    }
                }
        }
    }
}

Bool is_out_of_screen(vec origin, vec pos, float factor)
{
    float lb = origin.x - outer_spawn_box.x * factor;
    float rb = origin.x + outer_spawn_box.x * factor;
    float tb = origin.y - outer_spawn_box.y * factor;
    float bb = origin.y + outer_spawn_box.y * factor;

    return (pos.x < lb || pos.x > rb || pos.y < tb || pos.y > bb);
}

Bool is_out_of_chase_range(vec origin, vec pos, float range)
{
    return (distance(origin, pos) > range);
}


void update_entities(void)
{
    update_bullets();

    int i, j;
    for (i=BULLET_ENTITY_MAX; i<max_entity_id; ++i)
        if (ent[i].valid) {
            entity *en = &ent[i];
            // monsters
            if (i>=BULLET_ENTITY_MAX &&  i<=max_monster_id) {
                if (en->hp <= 0) {
                    en->valid = 0;
                    kill_count++;
                    int rand = get_random_int_range(0, 100);
                    if(rand < 3) // @hardcoded 3 percent chance of dropping a pickup
                    {
                        rand = get_random_int_range(0, 6);
                        if (rand < 2) {
                            create_entity(ET_pickup_a, en->pos);
                        } else if (rand >= 2 && rand < 4) {
                            create_entity(ET_pickup_s, en->pos);
                        } else if (rand >= 4 && rand < 6) {
                            create_entity(ET_pickup_health, en->pos);
                        }
                    }

                    int type = get_random_int_range(ET__monsters_start, ET__monsters_end);
                    create_monster_in_random_side(type, player_pos);
                }

                if (is_out_of_screen(ent[player_id].pos, en->pos, 1.5f)) // @hardcoded
                {
                    vec pos = reposition_monster(ent[player_id].pos, en->pos);
                    en->pos = pos;
                }

                vec p = en->pos;
                move_towards(&p, player_pos, dt, en->speed);
                en->pos.x = p.x;
                en->pos.y = p.y;

                int j;
                //for (j = i + 1; j<max_monster_id; ++j) // when two monster are on the right side they does not collide and one of them comes to left side of the player
                for (j = BULLET_ENTITY_MAX; j<max_monster_id; ++j)
                    if (ent[j].valid) {
                        if (i == j) continue;
                        if (en->type >= ET__monsters_start && en->type <= ET__monsters_end) {
                            resolve_overlap(i, j);
                        }
                    }

                if (check_box_collision_by_id(0, i))
                {
                    ent[player_id].hp -= 50.0f * dt;
                    if(ent[player_id].hp <= 0) {
                        ent[player_id].hp = 0;
                        //program_mode = MODE_menu;   
                    }
                    resolve_overlap(i, 0);
                }
            }

            // pickups
            if (is_pickup(en->type))
            {
                if(en->picked) {
                    animate_v2_to_d(&en->pos, ent[player_id].pos, dt, 30.0f);
                    if(distance(en->pos, ent[player_id].pos) < 5)
                        en->valid = 0;    
                    continue;
                } else if (world_timer - en->created >= cfg.max_pickup_time)
                    en->valid = 0;

                if (check_range_collision_by_id(i, player_id))
                {
                    en->picked = True;
                    switch (en->type) {
                        case ET_pickup_a:
                            special_ammo += 3;
                            break;
                        case ET_pickup_s:
                            increase_fire_rate(20);
                            break;
                        case ET_pickup_health:
                            ent[player_id].hp += 20;
                            break;
                    }

                }
            }

        }
}

Bool is_mouse_over_an_entity(int *ent_id)
{
    float mouse_x = input_frame.mouse_x;
    float mouse_y = input_frame.mouse_y;
    vec pos = screen_to_world(mouse_x, mouse_y);

    mouse_x = pos.x;
    mouse_y = pos.y;

    box a = ent_to_box(0);
    if (!(mouse_x < a.min.x || mouse_x > a.max.x) &&
        !(mouse_y < a.min.y || mouse_y > a.max.y)) {

        *ent_id = 0;
        return True;
    }

    int i;
    for( i=MONSTER_ENTITY_MIN; i<max_monster_id; ++i ) {
        box a = ent_to_box(i);

        if (mouse_x < a.min.x || mouse_x > a.max.x) continue;
        if (mouse_y < a.min.y || mouse_y > a.max.y) continue;

        *ent_id = i;
        return True;
    }

    for( i=BULLET_ENTITY_MIN; i<max_bullet_id; ++i ) {
        obb a = ent_to_obb(i);
        obb m = { (vec){mouse_x, mouse_y},
            (vec){1, 0},
            (vec){2, 2}
        };
        if (check_obb_collision(&a, &m))
        {
            *ent_id = i;
            return True;
        }
    }

    for( i=TILE_ENTITY_MIN; i<max_tile_id; ++i ) {
        box a = ent_to_box(i);

        if (mouse_x < a.min.x || mouse_x > a.max.x) continue;
        if (mouse_y < a.min.y || mouse_y > a.max.y) continue;

        *ent_id = i;
        return True;
    }

    return False;
}

entity_id selected_debug_entity_id;
Bool show_debug_info;
void process_debug_input(void)
{
    if (is_key_just_pressed(KEY_ESCAPE) ||
        is_key_just_pressed(KEY_SPACEBAR)) {
        program_mode = MODE_game;
        //show_debug_info = False;
    }

    if (is_key_just_pressed(KEY_F4))
        show_debug_info = !show_debug_info;


    if (is_key_just_pressed(KEY_F7)) {
        dt = 0.033;
        world_timer += dt;
        update_entities();
    }

    if (is_key_just_pressed(KEY_F6)) {
        dt = -0.033;
        world_timer += dt;
        update_entities();
    }

    if (is_key_down(KEY_F8)) {
        dt = 0.001;
        world_timer += dt;
        update_entities();
    }

    if (is_key_down(KEY_F5)) {
        dt = -0.001;
        world_timer += dt;
        update_entities();
    }

    static Bool is_rmb_down = False;
    static vec camera_pos_on_drag;

    if (is_key_just_pressed(MOUSE_BUTTON_MIDDLE) ||
        is_key_just_pressed(MOUSE_BUTTON_RIGHT)) {
        is_rmb_down = True;

        float mouse_x = input_frame.mouse_x;
        float mouse_y = input_frame.mouse_y;
        vec pos = screen_to_world(mouse_x, mouse_y);
        camera_pos_on_drag = (vec) {pos.x, pos.y};
    }

    if (is_key_just_released(MOUSE_BUTTON_MIDDLE) ||
        is_key_just_released(MOUSE_BUTTON_RIGHT)) {
        is_rmb_down = False;
    }

    if (is_rmb_down)
    {
        float mouse_x = input_frame.mouse_x;
        float mouse_y = input_frame.mouse_y;
        vec pos = screen_to_world(mouse_x, mouse_y);
        camera_pos = (vec) {camera_pos.x + camera_pos_on_drag.x - pos.x,
            camera_pos.y + camera_pos_on_drag.y - pos.y};
    }

    if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) {
        int ent_id;
        if (is_mouse_over_an_entity(&ent_id))
            selected_debug_entity_id = ent_id;
        else
            selected_debug_entity_id = -1;
    }

    if (is_key_just_pressed('X'))
        cfg.zoom = 4;
    if (is_key_down('Z') || is_key_down('E')) {
        cfg.zoom += 0.01f;
        if (cfg.zoom > 6)
            cfg.zoom = 6;
    }
    if (is_key_down('C') || is_key_down('Q')) {
        cfg.zoom -= 0.01f;
        if (cfg.zoom < 1.00)
            cfg.zoom = 1.00;
    }

    if (is_key_down('A')) camera_pos.x -= 0.5f;
    if (is_key_down('D')) camera_pos.x += 0.5f;
    if (is_key_down('S')) camera_pos.y -= 0.5f;
    if (is_key_down('W')) camera_pos.y += 0.5f;

    for (u64 i = 0; i < input_frame.number_of_events; i++) {
        Input_Event e = input_frame.events[i];

        if (e.kind == INPUT_EVENT_SCROLL) {
            cfg.zoom += 0.25f * e.yscroll;
            break;
        }
    }
}

void process_game_input(vec *axis)
{
    if (is_key_just_pressed(KEY_ESCAPE))
        window.should_close = True;//toggle_menu();
    if (is_key_down('A')) axis->x -= 1.0f;
    if (is_key_down('D')) axis->x += 1.0f;
    if (is_key_down('S')) axis->y -= 1.0f;
    if (is_key_down('W')) axis->y += 1.0f;

    if (is_key_just_pressed('M')) {
        program_mode = MODE_menu;
        consume_key_just_pressed('M');
    }

    if (is_key_just_pressed(KEY_F5) ||
        is_key_just_pressed(KEY_F6) ||
        is_key_just_pressed(KEY_F7) ||
        is_key_just_pressed(KEY_F8) ||
        is_key_just_pressed(KEY_SPACEBAR)) {
        program_mode = MODE_debug;
        show_debug_info = True;
    }

    if (is_key_just_pressed(KEY_F4))
        show_debug_info = !show_debug_info;

    if (is_key_just_pressed('Q'))
        next_weapon();
    if (is_key_just_pressed('E'))
        increase_fire_rate(20);
    if (is_key_just_pressed('H')) {
        should_draw_info = !should_draw_info;
    }

    if (is_key_just_pressed('X'))
        cfg.zoom = 4;
    if (is_key_down('Z')) {
        cfg.zoom += 0.01f;
        if (cfg.zoom > 6)
            cfg.zoom = 6;
    }
    if (is_key_down('C')) {
        cfg.zoom -= 0.01f;
        if (cfg.zoom < 1.00)
            cfg.zoom = 1.00;
    }
    if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)) {

        if (special_ammo > 0)
        {
            // TODO instead of changing bullet, fire another weapon
            special_ammo--;

            int cb = cur_weapon.bullet_type;
            cur_weapon.bullet_type = ET_bullet_tank;
            fire_bullet();
            cur_weapon.bullet_type = cb;
        }
    }
    //if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) {
    if (is_key_down(MOUSE_BUTTON_LEFT)) {

        if (can_fire())
            fire_bullet();
    }
}

double game_start_time;
void gameloop(void)
{
    if (program_mode == MODE_game) {
        world_timer += dt;
        update_view();

        vec input_axis = (vec){0, 0};
        process_game_input(&input_axis);

        Vector2 v2_input_axis = vec_to_v2(input_axis);

        v2_input_axis = v2_normalize(v2_input_axis);

        input_axis.x = v2_input_axis.x;
        input_axis.y = v2_input_axis.y;

        input_axis.x *= cfg.player_speed * dt;
        input_axis.y *= cfg.player_speed * dt;
        player_pos.x += input_axis.x;
        player_pos.y += input_axis.y;

        ent[0].pos.x = player_pos.x;
        ent[0].pos.y = player_pos.y;

        update_entities();
        process_tick_raw(dt); // @TODO move other update related things to this
        render_game();
    } else if (program_mode == MODE_menu)
    {
        process_menu_input();
        draw_menu_view();
    }
    else if (program_mode == MODE_debug)
    {
        dt = 0;
        update_view();
        process_debug_input();
        process_tick_raw(dt);
        render_game();
    }
}


void game_init(void)
{
    inner_spawn_box = (veci){210, 180};
    outer_spawn_box = (veci){280, 240};
    
    srand(time(0));
    player_char = (character){
        .weapon = WT_pistol,
        .hp              = 100,
        .speed           = 50,
    };
    
    cur_weapon = weapon_info[player_char.weapon];
    bullet_fire_cd = 1.0f / cur_weapon.fire_rate;
    
    font = load_font_from_disk(STR("../dat/fnt/karmina.otf"), get_heap_allocator());
    assert(font, "Failed loading karmina.otf, %d", GetLastError());
    player_pos = cfg.player_start_pos;
    render_init();
    world_init();
    menu_init();
}

int entry(int argc, char **argv)
{
    window.title = STR("fatal strike");
    window.width = SCREEN_X;
    window.height = SCREEN_Y;
    window.x = 200;
    window.y = 90;
    window.clear_color = hex_to_rgba(0x181818ff);

    game_init();

    while (!window.should_close) {
        double now = os_get_elapsed_seconds();
        reset_temporary_storage();
        os_update();
        gameloop();
        gfx_update();
        dt = os_get_elapsed_seconds() - now;
    }

    return 0;
}
