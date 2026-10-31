/*
 * Sonic Decompiled Origins - launcher
 *
 * A tiny SDL2 menu: pick one of the recompiled games and launch it with your
 * own ROM. Keyboard and gamepad both work.
 *
 * Build:  ./build.sh
 */
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#define WINDOW_W 640
#define WINDOW_H 480

struct game {
  const char *title;
  const char *binary;
  const char *rom;
  const char *hint;
};

static const struct game games[] = {
  { "SONIC 1",        "games/sonic1",   "roms/sonic1.bin",   "MEGA DRIVE" },
  { "SONIC 2",        "games/sonic2",   "roms/sonic2.bin",   "MEGA DRIVE" },
  { "SONIC 3 & K",    "games/s3k",      "roms/s3k.bin",      "MEGA DRIVE" },
  { "SONIC SMS",      "games/sonicsms", "roms/sonicsms.sms", "MASTER SYSTEM" },
};
static const int game_count = (int)(sizeof(games) / sizeof(games[0]));

/* --- 5x7 bitmap font, uppercase only ------------------------------------- */
static const unsigned char font[][7] = {
  ['A' - 32] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
  ['B' - 32] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
  ['C' - 32] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
  ['D' - 32] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E},
  ['E' - 32] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F},
  ['F' - 32] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},
  ['G' - 32] = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F},
  ['H' - 32] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
  ['I' - 32] = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},
  ['J' - 32] = {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C},
  ['K' - 32] = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
  ['L' - 32] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
  ['M' - 32] = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11},
  ['N' - 32] = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
  ['O' - 32] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
  ['P' - 32] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
  ['Q' - 32] = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D},
  ['R' - 32] = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
  ['S' - 32] = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E},
  ['T' - 32] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
  ['U' - 32] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
  ['V' - 32] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04},
  ['W' - 32] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11},
  ['X' - 32] = {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11},
  ['Y' - 32] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
  ['Z' - 32] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},
  ['0' - 32] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
  ['1' - 32] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
  ['2' - 32] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
  ['3' - 32] = {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},
  ['&' - 32] = {0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D},
  ['-' - 32] = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
  ['.' - 32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C},
  ['(' - 32] = {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02},
  [')' - 32] = {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
};

static void draw_char(SDL_Renderer *r, int x, int y, int scale, char c) {
  if (c < 32 || c > 126) return;
  const unsigned char *glyph = font[c - 32];
  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 5; col++) {
      if (glyph[row] & (1 << (4 - col))) {
        SDL_Rect px = {x + col * scale, y + row * scale, scale, scale};
        SDL_RenderFillRect(r, &px);
      }
    }
  }
}

static void draw_text(SDL_Renderer *r, int x, int y, int scale, const char *s) {
  for (; *s; s++) {
    draw_char(r, x, y, scale, *s);
    x += 6 * scale;
  }
}

static int file_exists(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) return 0;
  fclose(f);
  return 1;
}

static void launch(const struct game *g) {
  if (!file_exists(g->binary) || !file_exists(g->rom)) {
    return;
  }

#ifdef _WIN32
  _spawnl(_P_NOWAIT, g->binary, g->binary, g->rom, (char *)NULL);
#else
  pid_t pid = fork();
  if (pid == 0) {
    execl(g->binary, g->binary, g->rom, (char *)NULL);
    _exit(127);
  } else if (pid > 0) {
    waitpid(pid, NULL, 0);
  }
#endif
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *win = SDL_CreateWindow("SONIC DECOMPILED ORIGINS",
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);

  SDL_GameController *pad = NULL;
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_IsGameController(i)) {
      pad = SDL_GameControllerOpen(i);
      if (pad) break;
    }
  }

  int selected = 0;
  int running = 1;

  while (running) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) {
        running = 0;
      } else if (ev.type == SDL_KEYDOWN) {
        switch (ev.key.keysym.sym) {
        case SDLK_UP:    selected = (selected + game_count - 1) % game_count; break;
        case SDLK_DOWN:  selected = (selected + 1) % game_count; break;
        case SDLK_RETURN:
        case SDLK_SPACE: launch(&games[selected]); break;
        case SDLK_ESCAPE: running = 0; break;
        default: break;
        }
      } else if (ev.type == SDL_CONTROLLERBUTTONDOWN) {
        switch (ev.cbutton.button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:   selected = (selected + game_count - 1) % game_count; break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: selected = (selected + 1) % game_count; break;
        case SDL_CONTROLLER_BUTTON_A:         launch(&games[selected]); break;
        case SDL_CONTROLLER_BUTTON_B:         running = 0; break;
        default: break;
        }
      }
    }

    SDL_SetRenderDrawColor(ren, 10, 12, 22, 255);
    SDL_RenderClear(ren);

    SDL_SetRenderDrawColor(ren, 0x2a, 0x6f, 0xdb, 255);
    SDL_Rect bar = {0, 0, WINDOW_W, 6};
    SDL_RenderFillRect(ren, &bar);

    SDL_SetRenderDrawColor(ren, 0xf8, 0xc8, 0x20, 255);
    draw_text(ren, 40, 40, 3, "SONIC DECOMPILED ORIGINS");

    SDL_SetRenderDrawColor(ren, 0x60, 0x6a, 0x80, 255);
    draw_text(ren, 40, 80, 1, "ROM A C NATIVO. PON TUS ROMS EN ROMS/");

    for (int i = 0; i < game_count; i++) {
      const struct game *g = &games[i];
      const int ok = file_exists(g->binary) && file_exists(g->rom);
      const int y = 140 + i * 60;

      if (i == selected) {
        SDL_SetRenderDrawColor(ren, 0x1a, 0x24, 0x3a, 255);
        SDL_Rect hi = {24, y - 12, WINDOW_W - 48, 48};
        SDL_RenderFillRect(ren, &hi);
        SDL_SetRenderDrawColor(ren, 0xf8, 0xc8, 0x20, 255);
        SDL_Rect cursor = {32, y, 10, 10};
        SDL_RenderFillRect(ren, &cursor);
      }

      SDL_SetRenderDrawColor(ren, ok ? 0xf0, 0xf4, 0xff : 0x70, 0x76, 0x88, 255);
      draw_text(ren, 56, y, 2, g->title);

      SDL_SetRenderDrawColor(ren, 0x60, 0x6a, 0x80, 255);
      draw_text(ren, 300, y + 4, 1, g->hint);

      if (!ok) {
        SDL_SetRenderDrawColor(ren, 0xe0, 0x50, 0x50, 255);
        draw_text(ren, 300, y + 16, 1, "(FALTA ROM O JUEGO)");
      }
    }

    SDL_SetRenderDrawColor(ren, 0x60, 0x6a, 0x80, 255);
    draw_text(ren, 40, WINDOW_H - 40, 1, "FLECHAS: ELEGIR   ENTER: JUGAR   ESC: SALIR");

    SDL_RenderPresent(ren);
  }

  if (pad) SDL_GameControllerClose(pad);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
