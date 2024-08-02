#include "game_funcs.h"
// TODO add menu pages so we can have load page in menu

typedef struct
{
    Gfx_Font * font;
    int size;
} game_font;

static float scale = 0.2f;

game_font title_font;
game_font body_font;
game_font version_font;

Bool asking_for_restart_comformation = False;
Bool asking_for_quit_comformation = False;

int current_menu_choice = 0;

int index_resume  = -1;
int index_music   = -1;
int index_restart = -1;
int index_load    = -1;
int index_quit    = -1;

int menu_items_total = 0;

int num_menu_items_drawn = 0;

typedef struct
{
    float music_volume;
} Menu_State;

Menu_State menu_state = {};

static void toggle_menu(void)
{
    if(program_mode == MODE_game) {
        program_mode = MODE_menu;
    } else {
        program_mode = MODE_game;
    }        
}

static void advance_menu_choice(int delta)
{
    current_menu_choice += delta;
    
    if(current_menu_choice < 0)                 current_menu_choice += menu_items_total;
    if(current_menu_choice >= menu_items_total) current_menu_choice -= menu_items_total;
}

static void handle_enter(void)
{
    int choice = current_menu_choice;

    if(choice == index_resume) {
        toggle_menu();
        return;
    }
    
    if(choice == index_music) {
        if(menu_state.music_volume == 0.0f)
            menu_state.music_volume = 1.0f;
        else
            menu_state.music_volume = 0.0f;
        return;
    }

    if(choice == index_restart) {
        if(asking_for_restart_comformation) {
            // @Incomplete: Restart game state
            // should_restart_game = True;
            asking_for_restart_comformation = False;
        } else {
            asking_for_restart_comformation = True;
        }
        return;
    }

    if(choice == index_quit) {
        if(asking_for_quit_comformation) {
            //should_quit_game = True;
            window.should_close = True;
        } else {
            asking_for_quit_comformation = True;
        }
        return;
    }
}

#define BIG_FONT_SIZE 72

static void menu_init(void)
{
    Gfx_Font* fnt = load_font_from_disk(STR("../dat/fnt/karmina.otf"), get_heap_allocator());
    title_font.font   = fnt;
    title_font.size   = (int)(BIG_FONT_SIZE * 1.2);
    
    body_font.font    = fnt;
    body_font.size    = (int)(BIG_FONT_SIZE * 1.0);
    
    version_font.font = fnt;
    version_font.size = (int)(BIG_FONT_SIZE * 0.4);
}

int get_text_width_in_pixels(game_font gfont, char *str)
{
    Gfx_Text_Metrics metric = measure_text(gfont.font, STR(str), gfont.size, v2(0.2, 0.2));
    return metric.functional_size.x;
}

int menu_screen_x = 320;
int menu_screen_y = 180;

static void draw_version(void)
{
    game_font gfont = version_font;
    int k = 99;
    Vector4 color = {k,k,k,1};
    char* version_text = "Version: 0.0.1";
    Gfx_Text_Metrics metric = measure_text(gfont.font, STR(version_text), gfont.size, v2(0.2, 0.2));
    int w = metric.functional_size.x;
    int h = metric.functional_size.y;
    draw_text(gfont.font, STR(version_text), gfont.size, v2(menu_screen_x-w-1, h+1), v2(scale, scale), color);
}

static int draw_item(char *text, game_font gfont, int center_x, int y, Vector4 color)
{
    int index = num_menu_items_drawn;
    // SDL_Color item_color = {255, 173, 0};
    if(index == current_menu_choice)
        color =  (Vector4){255, 173, 0, 1};
    int tw = get_text_width_in_pixels(gfont, text);
    
    draw_text(gfont.font, STR(text), gfont.size, v2(center_x - tw/2, y), v2(scale, scale), color);
    num_menu_items_drawn += 1;
    return index;
}

static void draw_menu_choices(void)
{   
    draw_version();
    
    game_font font = body_font;
    int k = 111;
    int stride = (int)(1.2f * font.size) / -4;
    Vector4 color = {k, k, k, 1};

    int center_x = (int)(menu_screen_x/2);
    int corsur_y = (int)(menu_screen_y - menu_screen_y * 0.3);

    
    menu_items_total = num_menu_items_drawn;
    num_menu_items_drawn = 0;
    
    //
    // MENU_RESUME,
    //
    char* resume_string = "Resume";
    index_resume = draw_item(resume_string, font, center_x, corsur_y, color);
    corsur_y += stride;
        
    //
    // MENU_MUTE_MUSIC,
    //

    char* music_string = "Music: On";
    if(!menu_state.music_volume) music_string = "Music: Off";
    index_music = draw_item(music_string, font, center_x, corsur_y, color);
    corsur_y += stride;
    
    //
    // MENU_RESTART,
    //

    char* restart_string = "Restart";
    if(asking_for_restart_comformation) restart_string = "Restart? Are You Sure?";
    index_restart = draw_item(restart_string, font, center_x, corsur_y, color);   
    corsur_y += stride;
    
    //
    // MENU_LOAD_GAME,
    //

    char* load_game_string = "Load Game";
    index_load = draw_item(load_game_string, font, center_x, corsur_y, color);
    corsur_y += stride;
        
    //
    // MENU_QUIT
    //

    char* quit_string = "Quit Game";
    if(asking_for_quit_comformation) quit_string = "Quit? Are you Sure?";
    index_quit = draw_item(quit_string, font, center_x, corsur_y, color);
    
    //
    // Reset conformations
    //

    int choice = current_menu_choice;
    if(choice != index_quit) {
        asking_for_quit_comformation = false;
    }
    
    if(choice != index_restart) {
        asking_for_restart_comformation = false;
    }
}

static  void draw_menu_view(void)
{
    draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));    
    draw_frame.projection = m4_make_orthographic_projection(0, menu_screen_x,
                                                            0, menu_screen_y, -1, 10);
    
    char* title_text = "Hello, survivor!";
    int text_width = get_text_width_in_pixels(title_font, title_text);
    Vector4 color = {255, 0, 0, 1.0f};

    draw_text(title_font.font, STR(title_text), title_font.size,
              v2((menu_screen_x * 0.5f) - text_width/2,
                 (menu_screen_y * 0.9f)), v2(scale, scale), color);

    draw_menu_choices();
}

static void process_menu_input(void)
{
    if (is_key_just_pressed(KEY_ESCAPE)) {
        window.should_close = True;
    }
    
    if (is_key_just_pressed('M')) {
        toggle_menu();
        consume_key_just_pressed('M');
    }
    
    if (is_key_just_pressed('W')) advance_menu_choice(-1);
    if (is_key_just_pressed('S')) advance_menu_choice(1);
    if (is_key_just_pressed('E')) handle_enter();

    if (is_key_just_pressed(KEY_ARROW_UP))   advance_menu_choice(-1);
    if (is_key_just_pressed(KEY_ARROW_DOWN)) advance_menu_choice(1);
    if (is_key_just_pressed(KEY_ENTER))      handle_enter();
    // TODO consume keys or reset frame_input if necessary
}
