#include <application.h>
#include <game.h>
#include <bc_scheduler.h>
#include <bcl.h>
#include <bc_led.h>
#include <bc_button.h>

ball_t ball;
_size_t screen;
uint8_t fps;
player_t player;
uint32_t game_counter;
uint8_t blink_led_counter = 0;
uint8_t pl_speed = 10;
uint8_t increase_game_speed_each_sec = 1;
uint32_t player_width_counter = 0;
uint8_t decrease_player_width_each_sec = 10;
bool is_game_over = false;
bool is_game_paused = false;

bc_scheduler_task_id_t update_task_id;
bc_scheduler_task_id_t blink_led_task_id = 0;
bc_button_t button_left;
bc_button_t button_right;
bc_led_t lcdLed;
bc_led_driver_t *led_driver;

void game_init();
void game_update();
void game_draw();
void game_over();
void ball_update();
void ball_draw();
void player_move_left();
void player_move_right();
void player_draw();
void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
uint8_t rect_min_x(rect_t frame);
uint8_t rect_max_x(rect_t frame);
uint8_t rect_min_y(rect_t frame);
uint8_t rect_max_y(rect_t frame);

void game_init() {
    is_game_over = false;
    is_game_paused = true;
    player_width_counter = 0;
    fps = 60;

    // Update internal state
    screen.w = 128;
    screen.h = 128;
    player.score = 0;
    player.frame.size.w = 30;
    player.frame.size.h = 4;
    player.frame.origin.x = abs(rand()) % (screen.w - 1 - player.frame.size.w);
    player.frame.origin.y = (screen.h - player.frame.size.h);
    ball.frame.origin.x = player.frame.origin.x + player.frame.size.w * 0.5;
    ball.frame.origin.y = screen.h - 2 - player.frame.size.h;
    ball.frame.size.w = 4;
    ball.frame.size.h = 4;
    ball.v.dx = abs(rand()) % 2 == 1 ? -1 : 1;
    ball.v.dy = -1;

    // Draw pause menu
    game_draw_menu();

    // Init buttons and leds
    led_driver = bc_module_lcd_get_led_driver();
    bc_led_init_virtual(&lcdLed, 2, led_driver, 1);

    const bc_button_driver_t *lcdButtonDriver = bc_module_lcd_get_button_driver();
    bc_button_init_virtual(&button_left, 0, lcdButtonDriver, 0);
    bc_button_init_virtual(&button_right, 1, lcdButtonDriver, 0);

    bc_button_set_event_handler(&button_left, button_event_handler, (int *)0);
    bc_button_set_event_handler(&button_right, button_event_handler, (int *)1);
}

void game_draw_menu() {
    bc_module_lcd_clear();
    bc_module_lcd_draw_string(10, screen.h * 0.5 - 10, "Press any key", true);
    bc_module_lcd_update();
}

void game_start() {
    is_game_paused = false;
    // Register scheduler task (game update)
    update_task_id = bc_scheduler_register(game_update, NULL, 0);
}

void game_update() {
    ball_update();
    if (!is_game_over) {
        game_draw();
    }

    game_counter += 1;
    if ((uint8_t)ceilf((float)game_counter / fps) > increase_game_speed_each_sec) {
        game_counter = 0;
        fps += 1;
    }

    player_width_counter += 1;
    if ((uint8_t)ceilf((float)player_width_counter / fps) > decrease_player_width_each_sec && player.frame.size.w > ball.frame.size.w) {
        player_width_counter = 0;
        player.frame.size.w -= 2;
        player.frame.origin.x += 1;
        if (player.frame.size.w < ball.frame.size.w)
        {
            player.frame.size.w = ball.frame.size.w;
        }
    }

    if (update_task_id != 0) {
        bc_scheduler_plan_current_from_now(1000 / fps);
    }
}

void game_draw() {
    bc_module_lcd_clear();
    ball_draw();
    player_draw();
    bc_module_lcd_update();
}

void game_over() {
    is_game_over = true;
    bc_module_lcd_draw_string(30, screen.h * 0.5 - 10, "Game over", true);
    bc_module_lcd_update();
    bc_scheduler_unregister(update_task_id);
    update_task_id = 0;
    bc_scheduler_register(game_init, NULL, bc_tick_get() + 5000);
}

void ball_update() {
    ball.frame.origin.x += ball.v.dx;
    ball.frame.origin.y += ball.v.dy;

    if (rect_min_x(ball.frame) == 0 && ball.v.dx == -1)
        ball.v.dx = 1;
    if (rect_max_x(ball.frame) == screen.w - 1 && ball.v.dx == 1)
        ball.v.dx = -1;
    if (rect_min_y(ball.frame) == 0 && ball.v.dy == -1)
        ball.v.dy = 1;

    bool collisionX = rect_max_x(ball.frame) > rect_min_x(player.frame) && rect_min_x(ball.frame) < rect_max_x(player.frame);
    bool collisionY = rect_max_y(ball.frame) == (rect_min_y(player.frame) - 1);
    if (collisionX && collisionY && ball.v.dy == 1)
    {
        ball.v.dy = -1;
        player.score += 1;
        bc_led_pulse(&lcdLed, 100);
    }   

    if (rect_max_y(ball.frame) == screen.h-1)
        game_over();
}

void ball_draw() {
    bc_module_lcd_draw_rectangle(ball.frame.origin.x, ball.frame.origin.y, ball.frame.origin.x + ball.frame.size.w, ball.frame.origin.y + ball.frame.size.h, true);
}

void player_move_left() {
    player.frame.origin.x -= pl_speed;
    if (player.frame.origin.x < 0) {
        player.frame.origin.x = 0;
    }
}

void player_move_right() {
    player.frame.origin.x += pl_speed;
    if (rect_max_x(player.frame) >= screen.w) {
        player.frame.origin.x = screen.w - 1 - player.frame.size.w;
    }
}

void player_draw() {
    char score[10];
    snprintf(score, 10, "%u", player.score);
    bc_module_lcd_draw_rectangle(player.frame.origin.x, player.frame.origin.y, player.frame.origin.x + player.frame.size.w, player.frame.origin.y + player.frame.size.h, true);
    bc_module_lcd_draw_string(2, 2, score, true);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void)self;

    if (event == BC_BUTTON_EVENT_PRESS || event == BC_BUTTON_EVENT_HOLD) {
        if (is_game_paused) {
            game_start();
        } else {
            if ((int)event_param == 0)
            {
                player_move_left();
            }
            else
            {
                player_move_right();
            }
        }
    }
}

uint8_t rect_min_x(rect_t rect) {
    return rect.origin.x;
}

uint8_t rect_min_y(rect_t rect) {
    return rect.origin.y;
}

uint8_t rect_max_x(rect_t rect) {
    return rect.origin.x + rect.size.w;
}

uint8_t rect_max_y(rect_t rect) {
    return rect.origin.y + rect.size.h;
}