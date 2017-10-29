#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include "network.h"
#include "update.h"
#include "draw.h"
#include "util.h"

int state = ST_TITLE;
int hud = 0;
int running = 1;

int main(int argc, char* argv[]) {
    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_DISPLAY* display;
    int redraw = 1;

    initUserFolder();

    al_init();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_mouse();
    al_install_keyboard();

    drawInit();
    networkInit();

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

        updateInput(&event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;
        if (event.type == ALLEGRO_EVENT_TIMER) {
            update();
            redraw = 1;
        }
        if (redraw && al_is_event_queue_empty(queue)) {
            redraw = 0;
            draw();
        }
    }

    networkFini();
    drawFini();

    return 0;
}
