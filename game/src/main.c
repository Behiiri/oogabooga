#include <stdlib.h>
#include <time.h>

#include "game_funcs.h"
#include "render.c" // render.c needs to access some engine specific types


#if JUMBO_BUILD
#include "world.c"
#include "entity.c"
#endif

static int program_mode = MODE_game;
static float  dt;
static Vector2 player_pos;
static Vector2 camera_pos;

 
int get_random_int() {

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
    if (almost_equals(*v, d, 0.1f))
    {
        *v = d;
        return True; // reached
    }
    return False;
}

void animate_v2_to_d(Vector2* v, Vector2 d, float dt, float t)
{
    animate_f32_to_d(&(v->x), d.x, dt, t);
    animate_f32_to_d(&(v->y), d.y, dt, t);
}

Bool lerp_towards_f32_to_d(float* v, float d, float dt, float t)
{
    *v += (d - *v) * dt;
    if (almost_equals(*v, d, 0.1f))
    {
        *v = d;
        return True; // reached
    }
    return False;
}

void lerp_towards_v2_to_d(Vector2* v, Vector2 d, float dt, float t)
{
    lerp_towards_f32_to_d(&(v->x), d.x, dt, t);
    lerp_towards_f32_to_d(&(v->y), d.y, dt, t);
}

Bool move_towards_f32_to_d(float* v, float d, float dt, float t)
{float dx = d - *v;
    
    if (fabs(dx) <= t * dt) {
        *v = d;
        return 1;
    } else {
        float direction = (dx > 0) ? 1 : -1;
        *v += direction * t * dt;
        return 0;
    }

// *v += d + t * dt;
    if (almost_equals(*v, d, 0.1f))
    {
        *v = d;
        return True; // reached
    }
    return False;
}

void move_towards_v2_to_d(Vector2* v, Vector2 d, float dt, float t)
{
    move_towards_f32_to_d(&(v->x), d.x, dt, t);
    move_towards_f32_to_d(&(v->y), d.y, dt, t);
}

Bool move_towards(Vector2* v, Vector2 d, float dt, float t) {
    Vector2 dir;
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

Vector2 world_to_screen(float x, float y)
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

    return (Vector2) {world_pos.x, world_pos.y};
}

void next_weapon()
{
    cfg.bullet++;
    if(cfg.bullet > ET__bullets_end - 1) // -1 because of special ammo
        cfg.bullet = ET__bullets_start;
}

int special_ammo = 0;
void fire_bullet()
{
    float mouse_x = input_frame.mouse_x;
    float mouse_y = input_frame.mouse_y;
    Vector2 pos = world_to_screen(mouse_x, mouse_y);
    
    float offset_x = sprites[ET_player].size.x /2;
    float offset_y = sprites[ET_player].size.y /2;
    
    Vector2 dir = {pos.x - (player_pos.x + offset_x), pos.y - (player_pos.y + offset_y)};
    float length = sqrt(dir.x * dir.x + dir.y * dir.y);
    Vector2 unit_dir = {dir.x / length, dir.y / length};
    vec velocity = {unit_dir.x * cfg.bullet_speed, unit_dir.y * cfg.bullet_speed};
       
    int id = create_bullet(cfg.bullet, (vec){player_pos.x + offset_x, player_pos.y + offset_y});
    ent[id].velocity.x = velocity.x;
    ent[id].velocity.y = velocity.y;
    
    ent[id].rotation = (vec){unit_dir.x, unit_dir.y};
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

void increase_fire_rate(float percent)
{
    bullet_fire_cd = bullet_fire_cd * (1.0f - percent/100.0f);
    if(bullet_fire_cd < 0.06f) bullet_fire_cd = 0.06f;
}

static Bool should_draw_info = 0;
void process_game_input(Vector2 *axis)
{
    if (is_key_just_pressed(KEY_ESCAPE)) {
        window.should_close = true;
    }
    if (is_key_down('A')) {
        axis->x -= 1.0f;
    }
    if (is_key_down('D')) {
        axis->x += 1.0f;
    }
    if (is_key_down('S')) {
        axis->y -= 1.0f;
    }
    if (is_key_down('W')) {
        axis->y += 1.0f;
    }

    if (is_key_just_pressed('Q')) {
        next_weapon();
    }
    
    if (is_key_just_pressed('E')) {
        increase_fire_rate(10.0f);
    }

    if (is_key_just_pressed('H')) {
        should_draw_info = !should_draw_info;
    }
    
    // if (is_key_just_pressed('Z')) {
        // cfg.zoom += 0.5;
        // if(cfg.zoom > 8)
        //     cfg.zoom = 8;
    // }
    
    // if (is_key_just_pressed('C')) {

        // cfg.zoom -= 0.5;
        // if(cfg.zoom < 0.5)
        //     cfg.zoom = 0.5;
    // }
    
    if (is_key_just_pressed('X')) {
        cfg.zoom = 4;
    }

    if (is_key_down('Z'))
    {
        cfg.zoom += 0.01f;
        if(cfg.zoom > 6)
            cfg.zoom = 6;
    }
    
    if (is_key_down('C'))
    {
        cfg.zoom -= 0.01f;
        if(cfg.zoom < 1.25)
            cfg.zoom = 1.25;
    }

    if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)) {

            if(special_ammo > 0)
            {
                special_ammo--;
        
                int cb = cfg.bullet;
                cfg.bullet = ET_bullet_tank;
                fire_bullet();
                cfg.bullet = cb;
            }

    }

    
    
    //if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) {
    if (is_key_down(MOUSE_BUTTON_LEFT)) {

        if(can_fire())
            fire_bullet();
    }
    
}

Gfx_Font *font;
const u32 font_height = 48;
void draw_mouse_coordinates()
{
    float mouse_x = input_frame.mouse_x;
    float mouse_y = input_frame.mouse_y;

    Vector2 pos = world_to_screen(mouse_x, mouse_y);
    draw_text(font, sprint(get_heap_allocator(), STR("%.2f %.2f"), pos.x, pos.y), font_height, pos, v2(0.1, 0.1), COLOR_WHITE);
}

void draw_text_on_screen(int x, int y, float scale,  string str)
{
    scale = scale * (4 / cfg.zoom);
    //scale = cfg.zoom;
    y = window.scaled_height - y - 18;
    Vector2 pos = world_to_screen(x, y);
    draw_text(font, str, font_height, pos, v2(scale, scale), COLOR_WHITE);
}


void update_view(void)
{
    draw_frame.projection = m4_make_orthographic_projection(window.width  * -0.5f, window.width  * 0.5f,
                                                            window.height * -0.5f, window.height * 0.5f, -1, 10);

    Vector2 target_pos = vec_to_v2(ent[player_id].pos);
    animate_v2_to_d(&camera_pos, target_pos, dt, 15.0f);

    draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));
    draw_frame.view = m4_mul(draw_frame.view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
    draw_frame.view = m4_mul(draw_frame.view, m4_make_scale(v3(1.0f/cfg.zoom, 1.0f/cfg.zoom, 1.0f)));
}

Bool check_collision(entity_id ent_id_a, entity_id ent_id_b)
{
    entity a = ent[ent_id_a];
    entity b = ent[ent_id_b];
    float ax = a.pos.x;
    float ay = a.pos.y;
    float bx = b.pos.x;
    float by = b.pos.y;
    float aw = sprites[a.type].size.x;
    float ah = sprites[a.type].size.y;
    float bw = sprites[b.type].size.x;
    float bh = sprites[b.type].size.y;
    
    if (ax < bx + bw &&
        ax + aw > bx &&
        ay < by + bh &&
        ay + ah > by) {
        return True;
    }
    
    return False;
}

float distance(vec a, vec b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrt(dx*dx + dy*dy);
}


void update_bullets(void)
{
    int i, j;
    for (i=1; i<max_entity_id; ++i) { // bullets
        if (ent[i].valid) {
            if(ent[i].type >= ET__bullets_start && ent[i].type <= ET__bullets_end)
            {
                float d = distance(ent[i].pos, ent[player_id].pos);
                if(d > 300.0f) {
                    ent[i].valid = 0;
                    if(ent[i].type != ET_bullet_tank)
                        if(d > 150.0f)
                            ent[i].valid = 0;
                }
                    
                ent[i].pos.x += ent[i].velocity.x * dt * 100; // TODO @Hardcoded value
                ent[i].pos.y += ent[i].velocity.y * dt * 100;

                for (j=1;j<=MAX_ENTITIES;++j) // mummy
                    if (ent[j].valid && ent[j].type == ET_mummy)
                        if (check_collision(i, j)) {
                            if(ent[i].type == ET_bullet_tank)
                            {
                                ent[j].hp -= 5;
                            }
                            else
                            {
                                ent[j].hp--;
                                ent[i].valid = 0;
                            }

                            break;
                        }
            }
        }
    }    
}

// void update_monsters(void)
// {
    
// }

void update_entities(void)
{
    update_bullets();
    
    int i, j;        
    for (i=BULLET_ENTITY_MAX; i<=max_entity_id; ++i)
        if(ent[i].valid) {
            if(ent[i].type == ET_mummy) {
                if(ent[i].hp <= 0) {
                    ent[i].valid = 0;
                    int rand = get_random_int_range(0, 100);
                    if(rand < 15)  {
                        create_entity(ET_pickup_a, ent[i].pos);
                    }
                    else if(rand >= 15 && rand < 30) {
                        create_entity(ET_pickup_s, ent[i].pos);
                    }
                        
                    cteate_monster(ET_mummy, (vec) {player_pos.x, player_pos.y} , 400, cfg.mummy_hp);             
                }

                if(ent[i].valid)
                {
                    Vector2 v = vec_to_v2(ent[i].pos);
                    move_towards(&v, player_pos, dt, cfg.mummy_speed);
                    
                    ent[i].pos.x = v.x;
                    ent[i].pos.y = v.y;
                }
            }

            if(ent[i].type == ET_pickup_a) {
                if(check_collision(i, player_id))
                {
                    ent[i].valid = 0;
                    special_ammo += 3;
                }
            }
            
            if(ent[i].type == ET_pickup_s) {
                if(check_collision(i, player_id))
                {
                    ent[i].valid = 0;
                    increase_fire_rate(20);
                }
            }
        }
    
}

void draw_info()
{
    float s = 0.1f;
    int fh = font_height;
    int p = 15;
    int o = 0;

    static float temp_fps, temp_dt;
    static double last_info_update_time = 0;
    if(world_timer - last_info_update_time > 0.1f)
    {
        last_info_update_time = world_timer;
        temp_fps = 1.0f/dt;
        temp_dt = dt;
    }
    
    draw_text_on_screen(0, (p+fh*s)*o++, s, sprint(get_heap_allocator(), STR("fps:  %.2f  dt: %f"), temp_fps, temp_dt));
    draw_text_on_screen(0, (p+fh*s)*o++, s, sprint(get_heap_allocator(), STR("max_tile_id:  %d"), max_tile_id));
    draw_text_on_screen(0, (p+fh*s)*o++, s, sprint(get_heap_allocator(), STR("max_bullet_id:  %d"), max_bullet_id));
    draw_text_on_screen(0, (p+fh*s)*o++, s, sprint(get_heap_allocator(), STR("max_entity_id:  %d"), max_entity_id));
    draw_text_on_screen(0, (p+fh*s)*o++, s, sprint(get_heap_allocator(), STR("time:  %f"), world_timer));
    draw_text_on_screen(0, (p+fh*s)*o++, s, sprint(get_heap_allocator(), STR("fire_rate:  %f"), 1.0f/bullet_fire_cd));
}

double game_start_time;
void gameloop(void)
{
    world_timer += dt;

    update_view();

    if(program_mode == MODE_game)
    {
        Vector2 input_axis = v2(0, 0);
        process_game_input(&input_axis);

        input_axis = v2_normalize(input_axis);
        player_pos = v2_add(player_pos, v2_mulf(input_axis, cfg.player_speed * dt));
        ent[0].pos.x = player_pos.x;
        ent[0].pos.y = player_pos.y;
    }

    update_entities();

    render_game();
    //render_tiles();
    // draw_mouse_coordinates();

    if(should_draw_info)
        draw_info();
}

void game_init(void)
{
    srand(time(0));

    bullet_fire_cd = 1.0f / cfg.fire_rate;
    
    font = load_font_from_disk(STR("../dat/fnt/karmina.otf"), get_heap_allocator());
    assert(font, "Failed loading arial.ttf, %d", GetLastError());

    player_pos = vec_to_v2(cfg.player_start_pos);
        
    world_init();
    render_init();
}

int entry(int argc, char **argv)
{
    window.title = STR("fatal strike");
    window.scaled_width = 1280/3*2;
    window.scaled_height = 720/3*2;
    window.x = 200;
    window.y = 90;
    window.clear_color = hex_to_rgba(0x181818ff);

    game_init();

    while (!window.should_close)
    {
        reset_temporary_storage();
        os_update();        
        double now = os_get_current_time_in_seconds();

        gameloop();

        gfx_update();
        
        dt = os_get_current_time_in_seconds() - now;        
    }

    return 0;
}
