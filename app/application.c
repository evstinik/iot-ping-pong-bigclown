#include <application.h>
#include <game.h>

#define BLACK true

void application_init(void)
{
    // Initialize LCD
    // The parameter is internal buffer in SDK, no need to define it
    bc_module_lcd_init();

    // Init default font, this is necessary
    // See other fonts in sdk/bcl/inc/bc_font_common.h
    bc_module_lcd_set_font(&bc_font_ubuntu_15);

    game_init();
}