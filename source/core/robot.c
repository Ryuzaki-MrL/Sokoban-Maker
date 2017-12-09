#include "robot.h"
#include "update.h"
#include "draw.h"

static entity_t* robot = NULL;

void initRobot(int x, int y) {
    robot = entityAdd(x, y, ENT_ROBOT);
    robot->sprite = SPR_ROBOT_HOR;
    robot->speed = 4;
}

void updateRobot() {
    if (robot->x >= (DISPLAY_WIDTH>>1)) {
        level.cam.scx = robot->x - (DISPLAY_WIDTH>>1);
    } else {
        level.cam.scx = 0;
    }
    if (robot->y >= (DISPLAY_HEIGHT>>1)) {
        level.cam.scy = robot->y - (DISPLAY_HEIGHT>>1);
    } else {
        level.cam.scy = 0;
    }

    if (robot->moving) return;

    if (isKeyHeld(KEY_RIGHT)) {
        robot->xdir = 1;
        robot->ydir = 0;
        robot->sprite = SPR_ROBOT_HOR;
    } else if (isKeyHeld(KEY_LEFT)) {
        robot->xdir = -1;
        robot->ydir = 0;
        robot->sprite = SPR_ROBOT_HOR;
    } else if (isKeyHeld(KEY_UP)) {
        robot->xdir = 0;
        robot->ydir = -1;
        robot->sprite = SPR_ROBOT_UP;
    } else if (isKeyHeld(KEY_DOWN)) {
        robot->xdir = 0;
        robot->ydir = 1;
        robot->sprite = SPR_ROBOT_DOWN;
    } else {
        robot->xdir = robot->ydir = 0;
    }

    robot->moving = (robot->xdir || robot->ydir) && (TILE_GET(robot->x + (robot->xdir<<5), robot->y + (robot->ydir<<5))!=TL_WALL);

    entity_t* col = entityCollision(robot->x + (robot->xdir<<5), robot->y + (robot->ydir<<5), 32, 32, ENT_BOX);
    if (col) {
        entity_t* boxcol = entityCollision(col->x + (robot->xdir<<5), col->y + (robot->ydir<<5), 32, 32, ENT_BOX);
        robot->moving &= !boxcol && TILE_GET(col->x + (robot->xdir<<5), col->y + (robot->ydir<<5))!=TL_WALL;
        if (robot->moving) {
            //col->x += robot->xdir<<5;
            //col->y += robot->ydir<<5;
            col->moving = 1;
            col->speed = robot->speed;
            col->xdir = robot->xdir;
            col->ydir = robot->ydir;
        }
    }
    // robot->anispd = robot->moving ? 0.2 : 0;
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
