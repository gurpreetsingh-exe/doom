#include "renderer.h"
#include "config.h"
#include "wad.h"
#include <stdlib.h>

extern bool eq(char* a, char* b, int sz);
extern AssetManager* am_;
extern Texture* find_texture(char* name);

Renderer* renderer_init(Image* image) {
  Renderer* renderer = (Renderer*)malloc(sizeof(Renderer));
  renderer->image = image;
  return renderer;
}

/// https://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
void renderer_draw_line(Renderer* renderer, Vec2 v0, Vec2 v1, uint32_t color) {
  int x0 = v0.x;
  int x1 = v1.x;
  int y0 = v0.y;
  int y1 = v1.y;
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2, e2;

  for (;;) {
    if (x0 > renderer->image->width - 1 || x0 < 0 ||
        y0 > renderer->image->height - 1 || y0 < 0) {
      break;
    }
    renderer->image->data[x0 + (renderer->image->height - y0 - 1) *
                                   renderer->image->width] = color;
    if (x0 == x1 && y0 == y1) {
      break;
    }
    e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y0 += sy;
    }
  }
}

void renderer_line(Renderer* renderer, int x, int y1, int y2, uint32_t color) {
  Image* image = renderer->image;
  if (y1 < y2) {
    for (int y = y1; y <= y2; ++y) {
      renderer->image->data[x + (image->height - y - 1) * image->width] = color;
    }
  }
}

void renderer_draw_image(Renderer* renderer, int x, int y, char* image) {
  for (int i = 0; i < 764; ++i) {
    Patch* p = am_->sprites[i];
    if (eq(p->name, image, 6)) {
      Image* img = renderer->image;
      int xstart = x;
      int xend = x + p->width;
      int ystart = y;
      int yend = y + p->height;
      for (int i = xstart, x0 = 0; i < xend; ++i, ++x0) {
        for (int j = ystart, y0 = 0; j < yend; ++j, ++y0) {
          uint32_t color = p->image[x0 + (p->height - y0 - 1) * p->width];
          if (color) {
            img->data[i - (p->width / 2) + j * img->width] = color;
          }
        }
      }
      break;
    }
  }
}

void renderer_draw_flat(Renderer* renderer, Player* player, char* tex_id,
                        int light_level, int x, int y1, int y2, int world_z) {
  if (y1 > y2) {
    return;
  }
  Texture* tex = find_texture(tex_id);
  if (!tex) {
    return;
  }

  float player_dir_x = cosf(RADIANS(player->angle));
  float player_dir_y = sinf(RADIANS(player->angle));

  for (int y = y1; y <= y2; ++y) {
    int z = 0;
    if (y != 200) {
      z = 320 * world_z / (200 - y);
    }

    float px = player_dir_x * z + player->pos.x;
    float py = player_dir_y * z + player->pos.y;

    float left_x = -player_dir_y * z + px;
    float left_y = player_dir_x * z + py;
    float right_x = player_dir_y * z + px;
    float right_y = -player_dir_x * z + py;

    float dx = (right_x - left_x) / WIDTH;
    float dy = (right_y - left_y) / WIDTH;

    int tx = (int)(left_x + dx * x) & 63;
    int ty = (int)(left_y + dy * x) & 63;

    uint32_t col = tex->image[tx + ty * tex->width];
    Image* im = renderer->image;
    im->data[x + (im->height - y - 1) * im->width] = col;
    // PIXEL(renderer->image, x, y) = col;
  }
}

void renderer_draw_wall_col(Renderer* renderer, Texture* tex, int tex_col,
                            int x, int y1, int y2, float tex_alt,
                            float inv_scale, int light_level) {
  if (y1 > y2) {
    return;
  }
  int w = tex->width;
  int h = tex->height;
  tex_col %= w;
  float tex_y = tex_alt + ((float)y1 - 200) * inv_scale;

  for (int y = y1; y <= y2; ++y) {
    int i = tex_col + ((int)tex_y % h) * tex->width;
    int col = tex->image[i];
    Image* im = renderer->image;
    im->data[x + (im->height - y - 1) * im->width] = col;
    // PIXEL(renderer->image, x, y) = col;
    tex_y += inv_scale;
  }
}

void renderer_draw_rect(Renderer* renderer, Vec2 p0, Vec2 p1, uint32_t color) {
  Vec2 v0 = vec2(p0.x, p0.y);
  Vec2 v1 = vec2(p0.x, p1.y);
  Vec2 v2 = vec2(p1.x, p1.y);
  Vec2 v3 = vec2(p1.x, p0.y);
  renderer_draw_line(renderer, v0, v1, color);
  renderer_draw_line(renderer, v1, v2, color);
  renderer_draw_line(renderer, v2, v3, color);
  renderer_draw_line(renderer, v3, v0, color);
}

void renderer_destroy(Renderer* renderer) {
  image_destroy(renderer->image);
  free(renderer);
}
