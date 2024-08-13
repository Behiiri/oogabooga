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

#define SCREEN_X (1280.0f/3*2)
#define SCREEN_Y (720.0f/3*2)

static int program_mode = MODE_game;
static float  dt;
static vec player_pos;
static vec camera_pos;
static character player_char;
static weapon cur_weapon;

static int kill_count;

config cfg = {
    .zoom = 2.5f,
    .player_speed = 50.0f,
    .player_start_pos = (vec){0, 0},
};

float dot(vec a, vec b) {
    return a.x*b.x + a.y*b.y;
}

float distance(vec a, vec b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrt(dx*dx + dy*dy);
}

typedef struct
{
    vec pos;
    float radius;
} range;

typedef struct {
    vec   c;
    vec   d;
    vec   e;
    float r;

} obb;

range ent_to_range(entity_id id)
{
    range range;
    sprite s = sprites[ent[id].type];
    range.pos = (vec){ ent[id].pos.x + s.size.x/2, ent[id].pos.y + s.size.y/2 };
    int x = s.size.x/2 * s.scale;
    int y = s.size.y/2 * s.scale;
    range.radius = (x > y ? x : y);
    return range;
}

obb ent_to_obb(entity_id id)
{
    obb o;
    sprite s = sprites[ent[id].type];
    o.c = (vec){ ent[id].pos.x + s.size.x / 2, ent[id].pos.y + s.size.y / 2 };
    o.e = (vec){ s.size.x / 2 * s.scale, s.size.y / 2 * s.scale };
    o.r = ent[id].r;
    o.d = ent[id].dir;
    return o;
}

Bool check_range_collision(range a, range b)
{
    return distance(a.pos, b.pos) <= a.radius + b.radius;
}

Bool check_range_collision_by_id(entity_id id_a, entity_id id_b)
{
    range a = ent_to_range(id_a);
    range b = ent_to_range(id_b);
    return distance(a.pos, b.pos) <= a.radius + b.radius;
}

Bool check_obb_collision(obb* o1, obb* o2) {
    // vec a1 = {  cos(o1->r), sin(o1->r) };
    // vec a2 = { -sin(o1->r), cos(o1->r) };
    // vec a3 = {  cos(o2->r), sin(o2->r) };
    // vec a4 = { -sin(o2->r), cos(o2->r) };

    // monsters has no rotation so thier dir is (0,0)
    vec a1 = o1->d;
    vec a2 = { -o1->d.y, o1->d.x };
    vec a3 = { 1, 0 };
    vec a4 = { 0, 1 };
    
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
        return false;

    // project to a2
    r1 = l1.x * fabs(dot(a1, a2));
    r2 = l1.y * fabs(dot(a2, a2));
    r3 = l2.x * fabs(dot(a3, a2));
    r4 = l2.y * fabs(dot(a4, a2));
    if (r1 + r2 + r3 + r4 <= fabs(dot(l, a2)))
        return false;

    // project to a3
    r1 = l1.x * fabs(dot(a1, a3));
    r2 = l1.y * fabs(dot(a2, a3));
    r3 = l2.x * fabs(dot(a3, a3));
    r4 = l2.y * fabs(dot(a4, a3));
    if (r1 + r2 + r3 + r4 <= fabs(dot(l, a3)))
        return false;

    // project to a4
    r1 = l1.x * fabs(dot(a1, a4));
    r2 = l1.y * fabs(dot(a2, a4));
    r3 = l2.x * fabs(dot(a3, a4));
    r4 = l2.y * fabs(dot(a4, a4));
    if (r1 + r2 + r3 + r4 <= fabs(dot(l, a4))) 
        return false;
    
    return true;
}

Bool check_obb_collision_by_id(entity_id a, entity_id b)
{
    obb o1 = ent_to_obb(a);
    obb o2 = ent_to_obb(b);
    return check_obb_collision(&o1, &o2);
}

vec vec2(float x, float y)
{
    vec v = {x, y};
    return v;
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
    Matrix4 view = draw_frame.view;
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
    float fac = 1.2f;
    int sw = window.scaled_width/cfg.zoom  *fac;
    int sh = window.scaled_height/cfg.zoom *fac;
    int x, y;
    int dx = rand() % sw;
    int dy = rand() % sh;
    
    switch(side) {
        case LEFT: // 0
        x = origin.x - sw/2 - dx/(fac);
        y = origin.y - sh/2 + dy;
        break;
        case UP: // 1
            x = origin.x + dx - sw/2;
            y = origin.y + sh/2 + dy/(fac);
            break;
        case RIGHT: // 2
            x = origin.x + sw/2 + dx/(fac);
            y = origin.y - sh/2 + dy;
            break;
        case DOWN: // 3
            x = origin.x + dx - sw/2;
            y = origin.y - sh/2 - dy/(fac);
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

int get_new_spawn_side(vec origin, vec pos)
{
    int VIEW_WIDTH  = window.width/cfg.zoom;
    int VIEW_HEIGHT = window.height/cfg.zoom;
    float lb = origin.x - VIEW_WIDTH  / 2;
    float rb = origin.x + VIEW_WIDTH  / 2;
    float tb = origin.y + VIEW_HEIGHT / 2;
    float bb = origin.y - VIEW_HEIGHT / 2;

    if(pos.x > rb) return LEFT;
    if(pos.y < bb) return UP;
    if(pos.x < lb) return RIGHT;
    if(pos.y > tb) return DOWN;

    return -1;
}

vec reposition_monster(vec player_pos, vec pos)
{
    int side = get_new_spawn_side(player_pos, pos);
    return get_random_pos_on_side(player_pos, side);
}

void next_weapon(void)
{
    player_char.weapon++;
    if(player_char.weapon == WT__count)
        player_char.weapon = 0;
    cur_weapon = weapon_info[player_char.weapon];
}

int special_ammo = 10;
void fire_bullet(void)
{
    float mouse_x = input_frame.mouse_x;
    float mouse_y = input_frame.mouse_y;
    vec pos = screen_to_world(mouse_x, mouse_y);

    float offset_x = sprites[ET_player].size.x/2;
    float offset_y = sprites[ET_player].size.y/2;

    vec dir = {pos.x - (player_pos.x + offset_x), pos.y - (player_pos.y + offset_y)};
    float length = sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length == 0) length = 1;
    vec unit_dir = {dir.x / length, dir.y / length};
    vec velocity = {unit_dir.x * cur_weapon.bullet_speed, unit_dir.y * cur_weapon.bullet_speed};

    int id = create_bullet(cur_weapon.bullet_type, (vec){player_pos.x + offset_x, player_pos.y + offset_y});
    ent[id].velocity.x = velocity.x;
    ent[id].velocity.y = velocity.y;

    ent[id].dir = (vec){unit_dir.x, unit_dir.y};
    ent[id].r = atan2(unit_dir.x, unit_dir.y);
}

double bullet_fire_cd = 0.5;
double last_fire_time = 0;
Bool can_fire()
{
    double now = world_timer;
    if(now - last_fire_time >= bullet_fire_cd)
    {
        last_fire_time = now;
        return True;
    }
    return False;
}

void decrease_fire_cd(float percent)
{
    bullet_fire_cd = bullet_fire_cd * (1.0f - percent/100.0f);
    if(bullet_fire_cd < 0.02f) bullet_fire_cd = 0.02f;
}

void increase_fire_rate(int amount)
{
    cur_weapon.fire_rate = cur_weapon.fire_rate + amount;
    if(cur_weapon.fire_rate > 20) cur_weapon.fire_rate = 20;
    bullet_fire_cd = 1.0f/cur_weapon.fire_rate;
}

void update_view(void)
{
    draw_frame.projection = m4_make_orthographic_projection(window.scaled_width  * -0.5f, window.scaled_width  * 0.5f,
                                                            window.scaled_height * -0.5f, window.scaled_height * 0.5f, -1, 10);
    vec target_pos = ent[player_id].pos;
    animate_v2_to_d(&camera_pos, target_pos, dt, 15.0f);

    float scale_x = SCREEN_X/window.width;
    float scale_y = SCREEN_Y/window.height;
    float scale = scale_x > scale_y ? scale_x : scale_y;
    
    draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));
    draw_frame.view = m4_mul(draw_frame.view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
    draw_frame.view = m4_mul(draw_frame.view, m4_make_scale(v3((scale)/cfg.zoom, (scale)/cfg.zoom, 1.0f)));
}

Bool check_box_collision_by_id(entity_id ent_id_a, entity_id ent_id_b)
{
    entity a = ent[ent_id_a];
    entity b = ent[ent_id_b];
    float ax = a.pos.x;
    float ay = a.pos.y;
    float bx = b.pos.x;
    float by = b.pos.y;
    float aw = sprites[a.type].size.x * sprites[a.type].scale;
    float ah = sprites[a.type].size.y * sprites[a.type].scale;
    float bw = sprites[b.type].size.x * sprites[b.type].scale;
    float bh = sprites[b.type].size.y * sprites[b.type].scale;

    if (ax < bx + bw &&
        ax + aw > bx &&
        ay < by + bh &&
        ay + ah > by) {
        return True;
    }

    return False;
}

void update_bullets(void)
{
    int i, j; // bullet, monster
    for (i=TILE_ENTITY_MAX; i<max_bullet_id; ++i) { // bullets
        if (ent[i].valid) {
            float d = distance(ent[i].pos, ent[player_id].pos);
            if(d > 300.0f) {
                ent[i].valid = 0;
                if(ent[i].type != ET_bullet_tank)
                    if(d > 150.0f)
                        ent[i].valid = 0;
            }

            ent[i].pos.x += ent[i].velocity.x * dt * cur_weapon.bullet_speed; // TODO @Hardcoded value
            ent[i].pos.y += ent[i].velocity.y * dt * cur_weapon.bullet_speed;

            for (j=BULLET_ENTITY_MAX; j<=max_monster_id; ++j)
                if(ent[j].valid && (ent[j].type >= ET__monsters_start || ent[j].type <= ET__monsters_end)) {
                    //if (check_box_collision_by_id(i, j)) {
                    if (check_range_collision_by_id(i, j))
                    {
                        // obb o1 = ent_to_obb(i);
                        // obb o2 = ent_to_obb(j);
                        // if (check_obb_collision(&o1, &o2))
                        if (check_obb_collision_by_id(i, j))
                        {
                            if(ent[i].type == ET_bullet_tank) {
                                ent[j].hp -= 5;
                            } else {
                                ent[j].hp--;
                                ent[i].valid = 0;
                            }

                            break;
                        }
                    }
                }
        }
    }
}

Bool is_out_of_screen(vec origin, vec pos, float factor)
{
    // factor 1 == size of the screen, and 0.5f half of it, etc
    float mul = 1/cfg.zoom;
    int VIEW_WIDTH  = window.width *mul;
    int VIEW_HEIGHT = window.height*mul;
    float lb = origin.x - VIEW_WIDTH/2  * factor;
    float rb = origin.x + VIEW_WIDTH/2  * factor;
    float tb = origin.y - VIEW_HEIGHT/2 * factor;
    float bb = origin.y + VIEW_HEIGHT/2 * factor;
    
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
    for (i=BULLET_ENTITY_MAX; i<=max_entity_id; ++i)
        if(ent[i].valid) {
            // monsters
            if(ent[i].type >= ET__monsters_start && ent[i].type <= ET__monsters_end) {
                if(ent[i].hp <= 0) {
                    ent[i].valid = 0;
                    kill_count++;
                    int rand = get_random_int_range(0, 100);
                    if(rand < 5) {
                        create_entity(ET_pickup_a, ent[i].pos);
                    } else if(rand >= 5 && rand < 10) {
                        create_entity(ET_pickup_s, ent[i].pos);
                    }

                    int type = get_random_int_range(ET__monsters_start, ET__monsters_end);
                    create_monster_in_random_side(type, player_pos);
                }

                if(ent[i].valid)
                {
                    vec v = ent[i].pos;
                    move_towards(&v, player_pos, dt, ent[i].speed);

                    ent[i].pos.x = v.x;
                    ent[i].pos.y = v.y;
                    if(is_out_of_screen(ent[player_id].pos, ent[i].pos, 1.5f))
                    //if(is_out_of_chase_range(ent[player_id].pos, ent[i].pos, 275)) // @hardcoded
                    {
                        vec pos = reposition_monster(ent[player_id].pos, ent[i].pos);
                        ent[i].pos = pos;
                    }
                }
            }

            // pickups
            if(ent[i].type == ET_pickup_a) {
                if(check_box_collision_by_id(i, player_id))
                {
                    ent[i].valid = 0;
                    special_ammo += 3;
                }
            }

            if(ent[i].type == ET_pickup_s) {
                if(check_box_collision_by_id(i, player_id))
                {
                    ent[i].valid = 0;
                    increase_fire_rate(2);
                }
            }
        }
}

void process_debug_input()
{
    if (is_key_just_pressed(KEY_ESCAPE) ||
        is_key_just_pressed(KEY_SPACEBAR)) {
        program_mode = MODE_game;
    }

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
}

void process_game_input(vec *axis)
{
    if (is_key_just_pressed(KEY_ESCAPE))
        window.should_close = true;//toggle_menu();
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
    }
    
    if (is_key_just_pressed('Q'))
        next_weapon();
    if (is_key_just_pressed('E'))
        increase_fire_rate(2);
    if (is_key_just_pressed('H'))
        should_draw_info = !should_draw_info;
    if (is_key_just_pressed('X'))
        cfg.zoom = 4;
    if (is_key_down('Z')) {
        cfg.zoom += 0.01f;
        if(cfg.zoom > 6)
            cfg.zoom = 6;
    }
    if (is_key_down('C')) {
        cfg.zoom -= 0.01f;
        if(cfg.zoom < 1.00)
            cfg.zoom = 1.00;
    }
    if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)) {

        if(special_ammo > 0)
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

        if(can_fire())
            fire_bullet();
    }
}

double game_start_time;
void gameloop(void)
{
    if(program_mode == MODE_game) {
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
        render_game();
    } else if(program_mode == MODE_menu)
    {
        process_menu_input();
        draw_menu_view();
    }
    else if(program_mode == MODE_debug)
    {
        dt = 0;
        update_view();
        process_debug_input();
        render_game();
    }
}


void game_init(void)
{
    srand(time(0));

    player_char = (character){
        .weapon = WT_SpreadGun,
        .hp              = 100,
        .speed           = 50,
    };

    cur_weapon = weapon_info[player_char.weapon];

    bullet_fire_cd = 1.0f / cur_weapon.fire_rate;

    font = load_font_from_disk(STR("../dat/fnt/karmina.otf"), get_heap_allocator());
    assert(font, "Failed loading karmina.otf, %d", GetLastError());

    player_pos = cfg.player_start_pos;

    world_init();
    render_init();
    
    menu_init();
}

int entry(int argc, char **argv)
{
    window.title = STR("fatal strike");
    window.scaled_width = SCREEN_X;
    window.scaled_height = SCREEN_Y;
    window.x = 200;
    window.y = 90;
    window.clear_color = hex_to_rgba(0x181818ff);

    game_init();

    while (!window.should_close) {
        reset_temporary_storage();
        os_update();
        double now = os_get_current_time_in_seconds();

        gameloop();

        gfx_update();

        dt = os_get_current_time_in_seconds() - now;
    }

    return 0;
}
