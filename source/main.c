#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include "update.h"
#include "draw.h"
#include "save.h"
#include "state.h"
#include "message.h"
#include "util.h"

int state = ST_TITLE;
int hud = 0;
int running = 1;
int cursor = 0;
int screen = 0;

int kdown[ALLEGRO_KEY_MAX] = { 0 };
int kheld[ALLEGRO_KEY_MAX] = { 0 };
int keyany = 0;
int unichar = 0;
int mouse_b = 0;
int mouse_x = 0;
int mouse_y = 0;

char* userinput = NULL;

int main(int argc, char* argv[]) {
#ifdef DEBUG_MODE
    freopen("log.txt", "w+", stderr);
#endif
    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_DISPLAY* display;
    int redraw = 1;

    initUserFolder();
    if (!loadGame()) deleGame();

    al_init();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_mouse();
    al_install_keyboard();

    drawInit();
    setMessageLanguage(getLanguage());

    display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    al_set_window_title(display, TITLE);
    ALLEGRO_BITMAP* icon = al_load_bitmap("icon.png");
    if (icon) al_set_display_icon(display, icon);
    timer = al_create_timer(1.0 / 60);
    queue = al_create_event_queue();

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_start_timer(timer);

    while(running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;
        if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
            mouse_x = event.mouse.x;
            mouse_y = event.mouse.y;
        }
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            mouse_b = event.mouse.button;
        }
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            mouse_b = MB_NONE;
        }
        if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
            unichar = (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) ? -1 : event.keyboard.unichar;
        }
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            keyany = 1;
            kdown[event.keyboard.keycode] = 1;
            kheld[event.keyboard.keycode] = 1;
        } else if (event.type == ALLEGRO_EVENT_KEY_UP) {
            keyany = 0;
            kdown[event.keyboard.keycode] = 0;
            kheld[event.keyboard.keycode] = 0;
        }
        // Atualiza o jogo 60 vezes por segundo
        if (event.type == ALLEGRO_EVENT_TIMER) {
            update();
            memset(kdown, 0, sizeof(kdown));
            keyany = 0;
            mouse_b = MB_NONE;
            redraw = 1;
        }
        // Renderiza o jogo quando todos os outros eventos
        // do frame tiverem terminado
        if (redraw && al_is_event_queue_empty(queue)) {
            redraw = 0;
            draw();
        }
    }

    saveGame();
    drawFini();
#ifdef DEBUG_MODE
    fclose(stderr);
#endif
    return 0;
}
