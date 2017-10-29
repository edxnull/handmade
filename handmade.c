#include <stdint.h>
#include "handmade.h"

static void render_weird_gradient(struct game_offscreen_buffer *buffer, int boffset, int roffset)
{
    int x, y;
    uint8_t *row;
    uint32_t *pixel;
    uint8_t r, g, b;

    row = (uint8_t *)buffer->memory;
    for (y = 0; y < buffer->height; ++y) {
        pixel = (uint32_t *)row;
        for (x = 0; x < buffer->width; ++x) {
            b = (x + boffset);
            g = (y + roffset);
            *pixel++ = ((g << 8) | b);
        }
        row += buffer->pitch;
    }
}

void game_update_and_render(struct game_offscreen_buffer *buffer, int xoffset, int yoffset)
{
    render_weird_gradient(buffer, xoffset, yoffset);
}
