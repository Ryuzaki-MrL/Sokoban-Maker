#include "robot.h"
#include "sprite.h"
#include "state.h"

static entity_t* robot = NULL;

void initRobot(int x, int y) {
    robot = entityAdd(x, y, ENT_ROBOT);
    robot->sprite = SPR_ROBOT_HOR;
    robot->speed = 2;
}

void updateRobot() {
    // Camera
    if (robot->x >= (DISPLAY_WIDTH>>1)) {
        level.cam.scx = robot->x - (DISPLAY_WIDTH>>1);
    }
    if (robot->y >= (DISPLAY_HEIGHT>>1)) {
        level.cam.scy = robot->y - (DISPLAY_HEIGHT>>1);
    }

    // Movimento
    if (!robot->moving) {
        if (kheld[KB_RIGHT]) {
            robot->xdir = 1;
            robot->ydir = 0;
            robot->sprite = SPR_ROBOT_HOR;
        } else if (kheld[KB_LEFT]) {
            robot->xdir = -1;
            robot->ydir = 0;
            robot->sprite = SPR_ROBOT_HOR;
        } else if (kheld[KB_UP]) {
            robot->xdir = 0;
            robot->ydir = -1;
            robot->sprite = SPR_ROBOT_UP;
        } else if (kheld[KB_DOWN]) {
            robot->xdir = 0;
            robot->ydir = 1;
            robot->sprite = SPR_ROBOT_DOWN;
        }

        // O robô se move apenas se não houver uma parede
        // na direção em que ele está indo
        robot->moving = (
            ((kheld[KB_RIGHT] - kheld[KB_LEFT]) || (kheld[KB_UP] - kheld[KB_DOWN])) &&
            (TILE_GET(robot->x + (robot->xdir<<5), robot->y + (robot->ydir<<5))!=TL_WALL)
        );

        // Verifica se há uma caixa em sua direção. Se sim, verifica
        // se ela tem espaço para se mover. Só então o robô irá se
        // mover e empurrar a caixa
        entity_t* col = entityCollision(robot->x + (robot->xdir<<5), robot->y + (robot->ydir<<5), 32, 32, ENT_BOX);
        if (col) {
            entity_t* boxcol = entityCollision(col->x + (robot->xdir<<5), col->y + (robot->ydir<<5), 32, 32, ENT_BOX);
            robot->moving &= !boxcol && TILE_GET(col->x + (robot->xdir<<5), col->y + (robot->ydir<<5))!=TL_WALL;
            if (robot->moving) {
                col->moving = robot->moving;
                col->speed = robot->speed;
                col->xdir = robot->xdir;
                col->ydir = robot->ydir;
            }
        }

        robot->anispd = robot->moving ? 0.2 : 0;
    }
}

int getRobotX() {
    return robot->x;
}

int getRobotY() {
    return robot->y;
}

void resetRobot(int x, int y) {
    robot->x = robot->xstart = x;
    robot->y = robot->ystart = y;
    robot->visible = 1;
}
