#ifndef _GAME_H
#define _GAME_H

#include <bc_common.h>

typedef struct {
    int8_t x, y;
} point_t;

typedef struct {
    uint8_t w, h;
} _size_t;

typedef struct {
    point_t origin;
    _size_t size;
} rect_t;

typedef struct {
    int8_t dx, dy;
} velocity_t;

typedef struct {
    rect_t frame;
    velocity_t v;
} ball_t;

typedef struct {
    rect_t frame;
    uint16_t score;
} player_t;

void game_init();

#endif /* _GAME_H */
