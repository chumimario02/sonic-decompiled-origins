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
#include <errno.h>
#ifdef _WIN32
#  include <windows.h>
#  include <shellapi.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#ifdef _WIN32
#  define EXE_SUFFIX ".exe"
#else
#  define EXE_SUFFIX ""
#endif

#define WINDOW_W 640
#define WINDOW_H 480

struct game {
  const char *title;      /* "SONIC 1" */
  const char *version;    /* "RECOMP" / "RSDK" / NULL (single) */
  const char *binary;
  const char *rom;
  const char *args;
  const char *hint;
  const char *cwd;
};

static const struct game games[] = {
  { "SONIC 1", "RSDK",   "games/sonic1rsdk/RSDKv4", "games/sonic1rsdk/Data.rsdk", "usingCWD=true", "GENESIS", "games/sonic1rsdk" },
  { "SONIC 2", "RSDK",   "games/sonic2rsdk/RSDKv4", "games/sonic2rsdk/Data.rsdk", "usingCWD=true", "GENESIS", "games/sonic2rsdk" },
  { "SONIC 3 & K", "RECOMP", "games/s3k",      "roms/s3k.bin",      "",         "GENESIS", NULL },
  { "SONIC CD",    "RSDK",   "games/soniccd/SonicCD", "games/soniccd/Data.rsdk", "", "SEGA CD", "games/soniccd" },
};
static const int game_count = (int)(sizeof(games) / sizeof(games[0]));

/* Working directory the games expect (the folder with games/ and roms/). */
static char g_workdir[512];

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

static int game_binary_exists(const char *binary) {
  char path[512];
  snprintf(path, sizeof(path), "%s\\%s%s", g_workdir[0] ? g_workdir : ".", binary, EXE_SUFFIX);
  FILE *f = fopen(path, "rb");
  if (!f) return 0;
  fclose(f);
  return 1;
}

static int file_exists(const char *path) {
  char full[512];
  snprintf(full, sizeof(full), "%s\\%s", g_workdir[0] ? g_workdir : ".", path);
  FILE *f = fopen(full, "rb");
  if (!f) return 0;
  fclose(f);
  return 1;
}

static void launch(const struct game *g) {
  if (!game_binary_exists(g->binary) || !file_exists(g->rom)) {
    return;
  }

  char exe[512];
  snprintf(exe, sizeof(exe), "%s\\%s%s", g_workdir[0] ? g_workdir : ".", g->binary, EXE_SUFFIX);

  char abs_exe[512];
  char abs_rom[512];
  FILE *lg = fopen("launcher.log", "a");

#ifdef _WIN32
  if (!_fullpath(abs_exe, exe, sizeof(abs_exe))) { if (lg) { fprintf(lg, "fullpath FAIL exe=%s\n", exe); fclose(lg); } return; }
  {
    char rompath[512];
    snprintf(rompath, sizeof(rompath), "%s\\%s", g_workdir[0] ? g_workdir : ".", g->rom);
    if (!_fullpath(abs_rom, rompath, sizeof(abs_rom))) { if (lg) { fprintf(lg, "fullpath FAIL rom=%s\n", g->rom); fclose(lg); } return; }
  }
  char cmdline[1024];
  if (g->args && g->args[0]) {
    snprintf(cmdline, sizeof(cmdline), "%s \"%s\"", g->args, abs_rom);
  } else {
    snprintf(cmdline, sizeof(cmdline), "\"%s\"", abs_rom);
  }
  char fullcmd[1200];
  snprintf(fullcmd, sizeof(fullcmd), "\"%s\" %s", abs_exe, cmdline);

  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  memset(&pi, 0, sizeof(pi));

  /* Per-game engine env. Sonic 2: enable the level-select flag in work RAM
   * ($FFFFD0, the .w operand $FFD0 sign-extends to that) so the title screen
   * accepts A+Start without the Options > Sound Test cheat. The engine reads
   * GENESIS_POKE in its runner (see README). Also disable the per-line HBlank
   * handler, which busy-waits on the VDP H-blank flag and deadlocks under our
   * per-line-whole renderer on the CPZ water effect - the water loses its
   * purple shimmer but the level becomes playable. */
  if (strstr(g->binary, "sonic2") != NULL) {
    SetEnvironmentVariableA("GENESIS_POKE", "FFFFD0=0100");
    SetEnvironmentVariableA("GENESIS_POKE_FRAMES", "3000");
  }

  const char *cwd = (g->cwd && g->cwd[0]) ? g->cwd : (g_workdir[0] ? g_workdir : NULL);
  char abs_cwd[512];
  if (cwd && g->cwd && g->cwd[0]) {
    snprintf(abs_cwd, sizeof(abs_cwd), "%s\\%s", g_workdir[0] ? g_workdir : ".", cwd);
    char tmp[512];
    if (_fullpath(tmp, abs_cwd, sizeof(tmp))) snprintf(abs_cwd, sizeof(abs_cwd), "%s", tmp);
    cwd = abs_cwd;
  }
  BOOL ok = CreateProcessA(abs_exe, fullcmd, NULL, NULL, FALSE, CREATE_NO_WINDOW,
                           NULL, cwd, &si, &pi);
  if (ok) {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  if (lg) { fprintf(lg, "spawn ok=%d err=%lu cwd=%s cmd=%s\n", (int)ok, (unsigned long)GetLastError(), g_workdir, fullcmd); fclose(lg); }
#else
  if (!realpath(g->rom, abs_rom)) return;
  pid_t pid = fork();
  if (pid == 0) {
    if (g->args && g->args[0]) {
      execl(exe, exe, g->args, abs_rom, (char *)NULL);
    } else {
      execl(exe, exe, abs_rom, (char *)NULL);
    }
    _exit(127);
  } else if (pid > 0) {
    waitpid(pid, NULL, 0);
  }
#endif
}

int main(int argc, char **argv) {
#ifdef _WIN32
  {
    /* Find the project root (the folder that holds games/ and roms/) even if
       the launcher is invoked from inside launcher/. */
    char cand[512];
    const char *tries[8] = { ".", "..", "..\\..", NULL };
    /* also try the exe's own dir and the known project root */
    char exe[512]; GetModuleFileNameA(NULL, exe, sizeof(exe));
    char *sep = strrchr(exe, '\\'); if (sep) *sep = 0;
    for (int t = 0; tries[t]; t++) {
      DWORD attr = GetFileAttributesA(cand);
      /* build cand */
      snprintf(cand, sizeof(cand), "%s\\games", tries[t]);
      attr = GetFileAttributesA(cand);
      if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        snprintf(g_workdir, sizeof(g_workdir), "%s", tries[t]);
        break;
      }
    }
    if (!g_workdir[0]) {
      /* try exe's own dir */
      snprintf(cand, sizeof(cand), "%s\\games", exe);
      DWORD attr = GetFileAttributesA(cand);
      if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        snprintf(g_workdir, sizeof(g_workdir), "%s", exe);
      }
    }
    if (!g_workdir[0]) {
      /* last resort: known project root */
      snprintf(g_workdir, sizeof(g_workdir), "C:\\Users\\ThinkPad\\Documents\\Nueva carpeta\\sonic-decompiled-origins");
    }
    /* normalize to absolute */
    char abs[512];
    if (_fullpath(abs, g_workdir, sizeof(abs))) snprintf(g_workdir, sizeof(g_workdir), "%s", abs);
  }
#else
  if (!realpath(".", g_workdir)) g_workdir[0] = 0;
#endif

  FILE *dbg = fopen("launcher.log", "a");

  if (argc >= 3 && strcmp(argv[1], "--launch") == 0) {
    int idx = atoi(argv[2]);
    if (idx >= 0 && idx < game_count) {
      launch(&games[idx]);
    }
    return 0;
  }

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

  /* Flat menu: one entry per game. */
  int selected = 0;
  int running = 1;
  int fullscreen = 0;

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
        case SDLK_F11:
          fullscreen = !fullscreen;
          SDL_SetWindowFullscreen(win, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
          break;
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
    draw_text(ren, 40, 30, 3, "SONIC DECOMPILED ORIGINS");

    SDL_SetRenderDrawColor(ren, 0x60, 0x6a, 0x80, 255);
    draw_text(ren, 40, 70, 1, "ELIGE JUEGO. FLECHAS: MOVER   ENTER: JUGAR   ESC: SALIR");

    int y = 110;
    for (int i = 0; i < game_count; i++) {
      const struct game *g = &games[i];
      const int ok = game_binary_exists(g->binary) && file_exists(g->rom);
      if (i == selected) SDL_SetRenderDrawColor(ren, 0xff, 0xff, 0xff, 255);
      else SDL_SetRenderDrawColor(ren, 0x60, 0x6a, 0x80, 255);
      char line[64];
      snprintf(line, sizeof(line), "%s  (%s)", g->title, g->hint);
      draw_text(ren, 60, y, 3, line);
      if (!ok) {
        SDL_SetRenderDrawColor(ren, 0xff, 0x40, 0x40, 255);
        draw_text(ren, 60, y + 26, 2, "(FALTA ROM O JUEGO)");
      }
      y += 52;
    }

    SDL_RenderPresent(ren);
  }

  if (pad) SDL_GameControllerClose(pad);

    if (pad) SDL_GameControllerClose(pad);

    if (pad) SDL_GameControllerClose(pad);

    if (pad) SDL_GameControllerClose(pad);

    if (pad) SDL_GameControllerClose(pad);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
