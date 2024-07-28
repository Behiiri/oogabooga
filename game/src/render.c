#include "game_funcs.h"

#include "math.h"

#define TILE_X 32
#define TILE_Y 32

typedef struct
{
    int id;
    struct Gfx_Image *tex;
    Vector2 size;
    float scale;
} sprite;

sprite sprites[ET__count];

// Gfx_Image* load_sprite(char *filename)
// {
//     Gfx_Image *image = load_image_from_disk(STR(filename), get_heap_allocator());
//     assert(image, "load_sprite: Failed loading image");
//     return image;
// }

Gfx_Image* load_sprite_by_id(int id)
{
    assert(sprite_files[id].type == id);
    char * filaname = sprite_files[id].filename;
    Gfx_Image *image = load_image_from_disk(STR(filaname), get_heap_allocator());
    assert(image, "load_sprite: Failed loading image");
    return image; 
}

void render_init(void)
{
    // TODO just load the sprites that will be used
    int i;
    for (i=1; i < sizeof(sprite_files)/sizeof(sprite_files[0]); ++i) {
        int id = sprite_files[i].type;
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

extern Vector2 player_pos;
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
    for (i=BULLET_ENTITY_MAX;i<max_entity_id;++i)
        if (ent[i].valid) {
            int type = ent[i].type;
            Gfx_Image *g = sprites[type].tex;
            Vector2 sz = sprites[type].size;
            vec pos = ent[i].pos;
            draw_image(g, v2(pos.x, pos.y), sz, COLOR_WHITE);
        }
}


void render_bullets(void)
{
    int i;
    for (i=TILE_ENTITY_MAX;i<max_bullet_id;++i)
        if (ent[i].valid) {
            int type = ent[i].type;
            Gfx_Image *g = sprites[type].tex;
            Vector2 sz = sprites[type].size;
            vec pos = ent[i].pos;
            // draw_image(g, v2(pos.x, pos.y), sz, COLOR_WHITE);

            
            float radians = atan2(-ent[i].rotation.y, ent[i].rotation.x);
            
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

extern Gfx_Font* font;
extern const u32 font_height;
extern int special_ammo;
void render_ui()
{
    Gfx_Image *g = sprites[UI_special_ammo].tex;
    Vector2 sz = sprites[UI_special_ammo].size;
    sz         = v2_mul(sz, v2(4.f/cfg.zoom,4.f/cfg.zoom));
    int y = window.height/3*2 - sz.y/2;
    int x = window.width/20;
    Vector2 pos = world_to_screen(x, y);
    //Vector2 pos = v2(x, y);
    draw_image(g, pos, sz, COLOR_WHITE);
    
    pos.y -= sz.y/2;
    pos.x += sz.x/3;
    float scale = 0.2f;
    draw_text(font, sprint(get_heap_allocator(), STR("%d"), special_ammo),
              font_height, pos, v2(scale * 4.f/cfg.zoom, scale * 4.f/cfg.zoom), COLOR_WHITE);
}

void render_game(void)
{
    render_tiles();
    render_entities();
    render_bullets();
    render_player();
    render_ui();
}
