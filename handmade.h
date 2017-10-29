#ifndef HANDMADE_H
#define HANDMADE_H

struct game_offscreen_buffer {
    int width;
    int height;
    int pitch;
    void *memory;
};
/Users/edgar/Documents/projects/Asus projects/handmade/win32.c
void game_update_and_render(struct game_offscreen_buffer *buffer, int xoffset, int yoffset);

#endif /* HANDMADE_H */
