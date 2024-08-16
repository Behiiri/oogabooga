#define TILE_X 32
#define TILE_Y 32

#include "game_funcs.h"

typedef struct
{
    int id;
    struct Gfx_Image *tex;
    int layer;
} sprite;

sprite sprites[ET__count];

struct
{
    int type;
    float scale;
    int layer;
    char *filename;
    int x;
    int y;
} sprite_info[ET__count] =
{
#define X(name, scale, layer, filename, x, y) { name, scale, layer, filename, x, y },
    ENTITY_TYPES_X
#undef X
};

vec get_scaled_sprite_size(int type)
{
    vec v;
    v.x = sprite_info[type].x * sprite_info[type].scale;
    v.y = sprite_info[type].y * sprite_info[type].scale;
    return v;
}


Gfx_Image* load_sprite_by_id(int id)
{
    assert(sprite_info[id].type == id);
    char * filename = sprite_info[id].filename;
    Gfx_Image *image = load_image_from_disk(STR(filename), get_heap_allocator());
    assert(image, "load_sprite: Failed loading image %cs", filename);
    return image;
}

void render_init(void)
{
    // TODO just load the sprites that will be used
    int i;
    for (i=1; i < sizeof(sprite_info)/sizeof(sprite_info[0]); ++i) {
        int id = i;
        Gfx_Image *g = load_sprite_by_id(i);
        sprite *s = &sprites[id];
        s->tex = g;
        s->id = id;
    }
}

void draw_outline_rect(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Vector4 color)
{
    float s = 0.5;
    draw_line(a, b, s, color);
    draw_line(b, c, s, color);
    draw_line(c, d, s, color);
    draw_line(d, a, s, color);
}
 
void draw_aabb(box b)
{
    Vector2 p0 = v2(b.min.x, b.min.y);
    Vector2 p1 = v2(b.min.x, b.max.y);
    Vector2 p2 = v2(b.max.x, b.max.y);
    Vector2 p3 = v2(b.max.x, b.min.y);
    draw_outline_rect(p0, p1, p2, p3, v4(0.2, 0.2, 0.2, 1.0));
}

void draw_all_entity_aabb()
{
    int i;
    for (i=MONSTER_ENTITY_MIN; i<max_entity_id; ++i)
        if (ent[i].valid)
            draw_aabb(ent_to_box(i));
}

Vector2 vector2_rotate_u(Vector2 v, Vector2 p, vec u)
{
    v.x -= p.x;
    v.y -= p.y;

    float rx = v.x * u.x - v.y * u.y;
    float ry = v.x * u.y + v.y * u.x;

    v.x = rx + p.x;
    v.y = ry + p.y;

    return v;
}

void draw_obb(obb o)
{
    vec c = o.c;
    vec u = o.u;
    vec e = o.e;

    o.c = vec_rotate_u(o.c, o.e, o.u);
    
    float s = 0.5f;
    Vector2 p0 = v2(c.x-e.x, c.y-e.y);
    Vector2 p1 = v2(c.x+e.x, c.y-e.y);
    Vector2 p2 = v2(c.x+e.x, c.y+e.y);
    Vector2 p3 = v2(c.x-e.x, c.y+e.y);

    p0 = vector2_rotate_u(p0, p0, o.u);
    p1 = vector2_rotate_u(p1, p0, o.u);
    p2 = vector2_rotate_u(p2, p0, o.u);
    p3 = vector2_rotate_u(p3, p0, o.u);
    draw_outline_rect(p0, p1, p2, p3, v4(1,1,0,1));
    draw_circle(v2(p0.x-0.5f, p0.y-0.5f), v2(1,1), COLOR_RED);
}

void draw_all_entity_obb()
{
    int i;
    for (i=TILE_ENTITY_MAX; i<max_bullet_id; ++i)
        if (ent[i].valid)
            draw_obb(ent_to_obb(i));
}


extern vec player_pos;
void render_player(void)
{
    Matrix4 m = m4_scalar(1.0);
    m         = m4_translate(m, v3(player_pos.x, player_pos.y, 0));;
    Vector2 sz = v2(ent[player_id].size.x, ent[player_id].size.y);
    draw_image_xform(sprites[ET_player].tex, m, sz, COLOR_WHITE);
}

extern int show_debug_info;
void render_entities(void)
{
    int i;
    for (i=BULLET_ENTITY_MAX; i<max_entity_id; ++i)
        if (ent[i].valid) {
            int type = ent[i].type;
            Gfx_Image *g = sprites[type].tex;
            int layer = sprites[type].layer;
            Vector2 sz = v2(ent[i].size.x, ent[i].size.y);
            vec p = ent[i].pos;
            push_z_layer(layer);
            draw_image(g, v2(p.x, p.y), sz, COLOR_WHITE);
            pop_z_layer();
        }
}

void render_bullets(void)
{
    int i;
    for (i=TILE_ENTITY_MAX; i<max_bullet_id; ++i)
        if (ent[i].valid) {
            int type = ent[i].type;
            Gfx_Image *g = sprites[type].tex;
            Vector2 sz = v2(ent[i].size.x, ent[i].size.y);
            vec p = ent[i].pos;
            // draw_image(g, v2(pos.x, pos.y), sz, COLOR_WHITE);


            float radians = atan2(-ent[i].u.y, ent[i].u.x);

            Matrix4 m = m4_scalar(1.0);
            m         = m4_translate(m, v3(p.x, p.y, 0));
            //m         = m4_rotate_z(m, );
            m         = m4_rotate(m, v3(0.0f,0.0f,1.0f), radians);
            // push_z_layer(1000001);
            draw_image_xform(g, m, sz, COLOR_WHITE);
            // pop_z_layer();
        }
}


void render_tiles(void)
{
    { // tiled background
        float window_w = window.width;
        float window_h = window.height;

        Vector2 sz = v2(TILE_SIZE, TILE_SIZE);
        vec p_pos = ent[0].pos;
        int p_tile_x = world_to_tile_pos(p_pos.x);
        int p_tile_y = world_to_tile_pos(p_pos.y);
        int max_i = (window_w / cfg.zoom / TILE_SIZE) + 1;
        int max_j = (window_h / cfg.zoom / TILE_SIZE) + 1;

        // log("%d, %d", max_i, max_j);
        int i, j;
        for (i=-max_i;i<max_i;++i)
            for (j = -max_j; j < max_j; ++j){
                int tile_x = i + p_tile_x;
                int tile_y = j + p_tile_y;

                if ((tile_x + tile_y) % 2 == 0)
                {
                    Vector2 pos = v2(tile_x * TILE_SIZE,
                                     tile_y * TILE_SIZE);
                    draw_rect(pos, sz, (Vector4){0.2f, 0.2f, 0.3f, 0.2f});
                }
            }
    }


    int i;
    for (i=1;i<max_tile_id;++i) // ent[0] is player
        if (ent[i].valid) {
            int type = ent[i].type;
            Gfx_Image *g = sprites[type].tex;
            Vector2 sz = v2(ent[i].size.x, ent[i].size.y);
            Vector2 pos = v2(ent[i].pos.x, ent[i].pos.y);
            draw_image(g, pos, sz, COLOR_WHITE);
        }
}

extern Vector2 vec_to_v2(vec v);

Gfx_Font *font;
int font_height = 48;
void draw_mouse_coordinates(void)
{
    float mouse_x = input_frame.mouse_x;
    float mouse_y = input_frame.mouse_y;

    vec vpos = screen_to_world(mouse_x, mouse_y);
    Vector2 pos = vec_to_v2(vpos);
    draw_text(font, tprint(STR("%.2f %.2f"), pos.x, pos.y), font_height, pos, v2(0.1, 0.1), COLOR_WHITE);
}

void draw_text_on_screen(int x, int y, float scale,  string str)
{
    y = window.scaled_height - y - 18;
    vec vpos = screen_to_world(x, y);
    Vector2 pos = vec_to_v2(vpos);
    draw_text(font, str, font_height, pos, v2(scale, scale), COLOR_WHITE);
}

void draw_info(void)
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

    draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("fps:  %.2f  dt: %f"), temp_fps, temp_dt));
    //draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("max_tile_id:  %d"), max_tile_id));
    //draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("max_bullet_id:  %d"), max_bullet_id));
    //draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("max_entity_id:  %d"), max_entity_id));
    draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("time:  %f"), world_timer));
    draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("fire_rate:  %f"), 1.0f/bullet_fire_cd));

    draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("tile count:  %d"), max_tile_id - 1));
    draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("bullet count:  %d"), max_bullet_id - TILE_ENTITY_MAX));
    draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("entity count:  %d"), max_entity_id - BULLET_ENTITY_MAX));
    //draw_text_on_screen(0, (p+fh*s)*o++, s, tprint(STR("monster pos:  %f , %f"), ent[BULLET_ENTITY_MAX].pos.x, ent[BULLET_ENTITY_MAX].pos.y));
}

Vector2 get_sprite_size(int type)
{
    return v2(sprite_info[type].x * sprite_info[type].scale,
              sprite_info[type].y * sprite_info[type].scale);
}

extern Gfx_Font* font;
extern int font_height;
extern int special_ammo;
void render_ui(void)
{
    draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));
    draw_frame.projection = m4_make_orthographic_projection(0, 320,
                                                            0, 180, -1, 10);
    float scale = 0.2f;

    int fh = font_height;
    int p = 16 + fh*scale + 5;
    int o = 0;
    int x = 10;
    int y = 120;

    { // special ammo
        Gfx_Image *g = sprites[UI_special_ammo].tex;
        Vector2 sz = get_sprite_size(UI_special_ammo);
        Vector2 pos = v2(x, y);
        pos.y -= p*o++;
        draw_image(g, pos, sz, COLOR_WHITE);

        string str = tprint(STR("%d"), special_ammo);
        Gfx_Text_Metrics str_metrics = measure_text(font, str, fh, v2(scale, scale));

        pos.y = pos.y -sz.y/2;
        pos.x = pos.x +sz.x/2 - str_metrics.functional_size.x/2.0f;

        draw_text(font, str, fh, pos, v2(scale, scale), COLOR_WHITE);
    }

    { // fire rate
        Gfx_Image *g = sprites[UI_fire_rate].tex;
        Vector2 sz = get_sprite_size(UI_fire_rate); 
        Vector2 pos = v2(x, y);
        pos.y -= p*o++;
        draw_image(g, pos, sz, COLOR_WHITE);

        string str = tprint(STR("%d"), cur_weapon.fire_rate);
        Gfx_Text_Metrics str_metrics = measure_text(font, str, fh, v2(scale, scale));

        pos.y -= sz.y/2;
        pos.x += sz.x/2 - str_metrics.functional_size.x/2.0f;
        draw_text(font, str, fh, pos, v2(scale, scale), COLOR_WHITE);
    }

    { // World Time
        float scale = 0.25;
        string str = tprint(STR("time:  %4.2f"), world_timer);
        Gfx_Text_Metrics str_metrics = measure_text(font, STR("time:  8888.88"), fh, v2(scale, scale));
        int y = window.height/20 + str_metrics.functional_size.y;
        int x = window.width/2 - str_metrics.functional_size.x;

        draw_text_on_screen(x, y, scale, str);
    }

    { // skull annd kill count
        Gfx_Image *g = sprites[UI_skull].tex;
        Vector2 sz = get_sprite_size(UI_skull);
        int y = 10;
        Vector2 pos = v2(x, y);
        draw_image(g, pos, sz, COLOR_WHITE);

        string str = tprint(STR("%d"), kill_count);
        // Gfx_Text_Metrics str_metrics = measure_text(font, str, fh, v2(scale, scale));

        pos.y = pos.y + sz.y/3;
        pos.x = pos.x + sz.x + 3;
        draw_text(font, str, fh, pos, v2(scale, scale), COLOR_WHITE);
    }
}

extern entity_id selected_debug_entity_id;
void render_debug_ui(void)
{
    int w = 320;
    int h = 180;

    draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));
    draw_frame.projection = m4_make_orthographic_projection(0, w,
                                                            0, h, -1, 10);

    float s = 0.2f;
    int fh = font_height;
    int p = 1;
    int o = 0;

    int px = w/10*7.5;

    Matrix4 rect_xform = m4_scalar(1.0);
    rect_xform         = m4_translate(rect_xform, v3(px, 0, 0));
    draw_rect_xform(rect_xform, v2(w - px, h), v4(0.1,0.2,0.2,1));

    // h = h - 10;
    {
        string str = tprint(STR("entity info:"));
        Gfx_Text_Metrics metric = measure_text(font, str, fh, v2(s, s));
        int x = px + (w - px - metric.visual_size.x)/2;
        draw_text(font, str, fh, v2(x, h - (p+fh*s)*o++-metric.visual_size.y), v2(s, s), COLOR_WHITE);
    }

    {   // entity Id
        string str = tprint(STR("id:%d"), selected_debug_entity_id);
        Gfx_Text_Metrics metric = measure_text(font, str, fh, v2(s, s));
        int x = px + (w - px - metric.visual_size.x)/2;
        draw_text(font, str, fh, v2(x, h - (p+fh*s)*o++-metric.visual_size.y), v2(s, s), COLOR_WHITE);
    }

    {   // pos
        vec epos = ent[selected_debug_entity_id].pos;
        string str = tprint(STR("pos: %.1f, %.1f"), epos.x, epos.y);
        Gfx_Text_Metrics metric = measure_text(font, str, fh, v2(s, s));
        int x = px + (w - px - metric.functional_size.x)/2;
        draw_text(font, str, fh, v2(x, h - (p+fh*s)*o++-metric.visual_size.y), v2(s, s), COLOR_WHITE);
    }

    {   // velocity
        vec v = ent[selected_debug_entity_id].velocity;
        string str = tprint(STR("v: %.1f, %.1f"), v.x, v.y);
        Gfx_Text_Metrics metric = measure_text(font, str, fh, v2(s, s));
        int x = px + (w - px - metric.visual_size.x)/2;
        draw_text(font, str, fh, v2(x, h - (p+fh*s)*o++-metric.visual_size.y), v2(s, s), COLOR_WHITE);
    }

    {   // dir
        vec v = ent[selected_debug_entity_id].u;
        string str = tprint(STR("dir: %.1f, %.1f"), v.x, v.y);
        Gfx_Text_Metrics metric = measure_text(font, str, fh, v2(s, s));
        int x = px + (w - px - metric.visual_size.x)/2;
        draw_text(font, str, fh, v2(x, h - (p+fh*s)*o++-metric.visual_size.y), v2(s, s), COLOR_WHITE);
    }
    {   // hp
        int v = ent[selected_debug_entity_id].hp;
        string str = tprint(STR("hp:%d"), v);
        Gfx_Text_Metrics metric = measure_text(font, str, fh, v2(s, s));
        int x = px + (w - px - metric.visual_size.x)/2;
        draw_text(font, str, fh, v2(x, h - (p+fh*s)*o++-metric.visual_size.y), v2(s, s), COLOR_WHITE);
    }

    {   // type
        int v = ent[selected_debug_entity_id].type;
        string str = tprint(STR("type:%d"), v);
        Gfx_Text_Metrics metric = measure_text(font, str, fh, v2(s, s));
        int x = px + (w - px - metric.visual_size.x)/2;
        draw_text(font, str, fh, v2(x, h - (p+fh*s)*o++-metric.visual_size.y), v2(s, s), COLOR_WHITE);
    }

    {   // speed
        int v = ent[selected_debug_entity_id].speed;
        string str = tprint(STR("speed:%d"), v);
        Gfx_Text_Metrics metric = measure_text(font, str, fh, v2(s, s));
        int x = px + (w - px - metric.visual_size.x)/2;
        draw_text(font, str, fh, v2(x, h - (p+fh*s)*o++-metric.visual_size.y), v2(s, s), COLOR_WHITE);
    }

    {   // radius
        int v = ent[selected_debug_entity_id].radius;
        string str = tprint(STR("radius:%d"), v);
        Gfx_Text_Metrics metric = measure_text(font, str, fh, v2(s, s));
        int x = px + (w - px - metric.visual_size.x)/2;
        draw_text(font, str, fh, v2(x, h - (p+fh*s)*o++-metric.visual_size.y), v2(s, s), COLOR_WHITE);
    }
}

static Bool should_draw_info = 0;
void render_game(void)
{
    //
    // Game
    //
    render_tiles();

    if(show_debug_info) {
        draw_all_entity_aabb();
        draw_all_entity_obb();
    }
    
    render_entities();
    render_bullets();
    render_player();


    //
    // UI
    //
    if(program_mode == MODE_debug)
        if(selected_debug_entity_id != -1)
            render_debug_ui();

    render_ui();
    
    if(should_draw_info)
        draw_info();

}
