#define TILE_X 32
#define TILE_Y 32

typedef struct
{
    int id;
    struct Gfx_Image *tex;
    Vector2 size;
    float scale;
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
} sprite_files[ET__count] =
{
#define X(name, scale, layer, filename, x, y) { name, scale, layer, filename, x, y },
    ENTITY_TYPES_X
#undef X
};

Gfx_Image* load_sprite_by_id(int id)
{
    assert(sprite_files[id].type == id);
    char * filename = sprite_files[id].filename;
    Gfx_Image *image = load_image_from_disk(STR(filename), get_heap_allocator());
    assert(image, "load_sprite: Failed loading image %cs", filename);
    return image; 
}

void render_init(void)
{
    // TODO just load the sprites that will be used
    int i;
    for (i=1; i < sizeof(sprite_files)/sizeof(sprite_files[0]); ++i) {
        int id = i;//sprite_files[i].type;
        Gfx_Image *g = load_sprite_by_id(i);
        sprite *s = &sprites[id];
        s->tex = g;
        s->id = id;
        s->scale = sprite_files[i].scale;
        s->size = v2(sprite_files[i].x, sprite_files[i].y);
        if(s->scale != 1.0f)
            s->size = v2_mulf(s->size, s->scale);
    }
}

extern vec player_pos;
void render_player(void)
{
    Matrix4 m = m4_scalar(1.0);
    m         = m4_translate(m, v3(player_pos.x, player_pos.y, 0));
    Vector2 sz = sprites[ET_player].size;
    draw_image_xform(sprites[ET_player].tex, m, sz, COLOR_WHITE);
}

void render_entities(void)
{
    int i;
    for (i=BULLET_ENTITY_MAX; i<max_entity_id; ++i)
        if (ent[i].valid) {
            int type = ent[i].type;
            Gfx_Image *g = sprites[type].tex;
            int layer = sprites[type].layer;
            Vector2 sz = sprites[type].size;
            vec pos = ent[i].pos;
            push_z_layer(layer);
            draw_image(g, v2(pos.x, pos.y), sz, COLOR_WHITE);
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
            Vector2 sz = sprites[type].size;
            vec pos = ent[i].pos;
            // draw_image(g, v2(pos.x, pos.y), sz, COLOR_WHITE);

            
            float radians = atan2(-ent[i].dir.y, ent[i].dir.x);
            
            Matrix4 m = m4_scalar(1.0);
            m         = m4_translate(m, v3(pos.x, pos.y, 0));
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
            Vector2 sz = sprites[type].size;
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
        Vector2 sz = sprites[UI_special_ammo].size;
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
        Vector2 sz = sprites[UI_fire_rate].size;
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
        Vector2 sz = sprites[UI_skull].size;
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

static Bool should_draw_info = 0;
void render_game(void)
{
    render_tiles();
    render_entities();
    render_bullets();
    render_player();
    render_ui();
    
    if(should_draw_info)
        draw_info();
}

