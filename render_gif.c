
#include "render_gif.h"
#include "satisfaction75.h"
#include <stdio.h>

#define DEFAULT_ANIM_SIZE 512
#define ANIM_GIF_SPEED 100

#ifdef OLED_ENABLE

uint32_t gif_anim_timer = 0;

void draw_gif() {}

#endif
