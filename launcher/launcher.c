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
  { "SONIC 3 & K", "RECOMP", "games/s3k",      "roms/s3k.bin",      "",         "GENESIS", "games" },
  { "SONIC CD",    "RSDK",   "games/soniccd/SonicCD", "games/soniccd/Data.rsdk", "", "SEGA CD", "games/soniccd" },
  { "SONIC CHAOS", "RECOMP", "games/sonicchaos/SonicChaosRecomp", "roms/sonicchaos.gg", "--gg --window 3", "GAME GEAR", "games/sonicchaos" },
  { "SONIC TT",    "RECOMP", "games/sonicttsms/SonicTripleTroubleSMSRecomp", "roms/sonicttsms.sms", "--sms --interp --window 3", "MASTER SYSTEM", "games/sonicttsms" },
};
static const int game_count = (int)(sizeof(games) / sizeof(games[0]));

/* Category: 0 = Genesis / 16-bit (+Sega CD), 1 = Master System / Game Gear. */
static const int game_cat[] = { 0, 0, 0, 0, 1, 1 };
static const char *cat_names[] = { "GENESIS / 16-BIT", "SMS / GAME GEAR" };
#define CAT_COUNT 2

static int cat_first(int c) {
  for (int i = 0; i < game_count; i++) if (game_cat[i] == c) return i;
  return 0;
}
static int cat_step(int cur, int dir) {
  for (int k = 1; k <= game_count; k++) {
    int i = (cur + dir * k + game_count * 4) % game_count;
    if (game_cat[i] == game_cat[cur]) return i;
  }
  return cur;
}

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
  ['4' - 32] = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
  ['5' - 32] = {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
  ['6' - 32] = {0x0E, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x0E},
  ['7' - 32] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
  ['8' - 32] = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
  ['9' - 32] = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E},
  ['/' - 32] = {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10},
  [':' - 32] = {0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00},
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

static int text_width(const char *s, int scale) { return (int)strlen(s) * 6 * scale; }

static int fit_scale(const char *s, int maxw, int cap) {
  int n = (int)strlen(s);
  if (n <= 0) return 1;
  int s2 = maxw / (n * 6);
  if (s2 < 1) s2 = 1;
  if (s2 > cap) s2 = cap;
  return s2;
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
                                     WINDOW_W, WINDOW_H,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);

  SDL_GameController *pad = NULL;
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_IsGameController(i)) {
      pad = SDL_GameControllerOpen(i);
      if (pad) break;
    }
  }

  /* Two-level menu: 0 = category, 1 = games of that category. */
  int selected = 0;
  int level = 0, sel_cat = 0;
  int running = 1;
  int fullscreen = 1;

  while (running) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) {
        running = 0;
      } else if (ev.type == SDL_KEYDOWN) {
        switch (ev.key.keysym.sym) {
        case SDLK_UP:
          if (level == 0) sel_cat = (sel_cat + CAT_COUNT - 1) % CAT_COUNT;
          else selected = cat_step(selected, -1);
          break;
        case SDLK_DOWN:
          if (level == 0) sel_cat = (sel_cat + 1) % CAT_COUNT;
          else selected = cat_step(selected, +1);
          break;
        case SDLK_RETURN:
          if (ev.key.keysym.mod & KMOD_ALT) {
            fullscreen = !fullscreen;
            SDL_SetWindowFullscreen(win, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
          } else if (level == 0) {
            level = 1; selected = cat_first(sel_cat);
          } else {
            launch(&games[selected]);
          }
          break;
        case SDLK_SPACE:
          if (level == 0) { level = 1; selected = cat_first(sel_cat); }
          else launch(&games[selected]);
          break;
        case SDLK_ESCAPE:
          if (level == 1) level = 0; else running = 0;
          break;
        default: break;
        }
      } else if (ev.type == SDL_CONTROLLERBUTTONDOWN) {
        switch (ev.cbutton.button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
          if (level == 0) sel_cat = (sel_cat + CAT_COUNT - 1) % CAT_COUNT;
          else selected = cat_step(selected, -1);
          break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
          if (level == 0) sel_cat = (sel_cat + 1) % CAT_COUNT;
          else selected = cat_step(selected, +1);
          break;
        case SDL_CONTROLLER_BUTTON_A:
        case SDL_CONTROLLER_BUTTON_START:
          if (level == 0) { level = 1; selected = cat_first(sel_cat); }
          else launch(&games[selected]);
          break;
        case SDL_CONTROLLER_BUTTON_B:
        case SDL_CONTROLLER_BUTTON_BACK:
          if (level == 1) level = 0; else running = 0;
          break;
        default: break;
        }
      }
    }

    int W = WINDOW_W, H = WINDOW_H;
    SDL_GetRendererOutputSize(ren, &W, &H);
    if (W <= 0 || H <= 0) { W = WINDOW_W; H = WINDOW_H; }
    int small = (W < 420 || H < 320);

    SDL_SetRenderDrawColor(ren, 10, 12, 22, 255);
    SDL_RenderClear(ren);

    SDL_SetRenderDrawColor(ren, 0x2a, 0x6f, 0xdb, 255);
    SDL_Rect bar = {0, 0, W, 6};
    SDL_RenderFillRect(ren, &bar);

    const char *title = small ? "SONIC" : "SONIC DECOMPILED ORIGINS";
    int ts = fit_scale(title, (int)(W * 0.95), small ? 4 : 8);
    int ty = H / 20;
    SDL_SetRenderDrawColor(ren, 0xf8, 0xc8, 0x20, 255);
    draw_text(ren, (W - text_width(title, ts)) / 2, ty, ts, title);

    const char *hint = small
        ? (level == 0 ? "A: ELEGIR   B: SALIR" : "A: JUGAR   B: VOLVER")
        : (level == 0 ? "FLECHAS: MOVER   ENTER: ELEGIR   ESC: SALIR"
                      : "FLECHAS: MOVER   ENTER: JUGAR   ESC: VOLVER   ALT+ENTER: PANTALLA COMPLETA");
    int hs = fit_scale(hint, (int)(W * 0.95), 2);
    int hy = ty + 7 * ts + H / 24;
    SDL_SetRenderDrawColor(ren, 0x60, 0x6a, 0x80, 255);
    draw_text(ren, (W - text_width(hint, hs)) / 2, hy, hs, hint);

    int startY = hy + 7 * hs + H / 14;
    int avail = H - startY - H / 12;

    if (level == 0) {
      int es = small ? 4 : 6;
      for (int c = 0; c < CAT_COUNT; c++)
        es = fit_scale(cat_names[c], (int)(W * 0.9), es);
      int step = avail / CAT_COUNT;
      if (step < 7 * es + 8) step = 7 * es + 8;
      for (int c = 0; c < CAT_COUNT; c++) {
        if (c == sel_cat) SDL_SetRenderDrawColor(ren, 0xff, 0xff, 0xff, 255);
        else SDL_SetRenderDrawColor(ren, 0x60, 0x6a, 0x80, 255);
        int y = startY + c * step + (step - 7 * es) / 2;
        draw_text(ren, (W - text_width(cat_names[c], es)) / 2, y, es, cat_names[c]);
      }
    } else {
      int idx[16], cnt = 0;
      for (int i = 0; i < game_count && cnt < 16; i++)
        if (game_cat[i] == sel_cat) idx[cnt++] = i;

      int cols = (cnt > 4 && !small) ? 2 : 1;
      int rows = (cnt + cols - 1) / cols;
      int colW = W / cols;

      int es = small ? 4 : 6;
      for (int k = 0; k < cnt; k++)
        es = fit_scale(games[idx[k]].title, (int)(colW * 0.9), es);

      int step = avail / rows;
      if (step < 7 * es + 4) step = 7 * es + 4;

      for (int k = 0; k < cnt; k++) {
        int i = idx[k];
        const struct game *g = &games[i];
        const int ok = game_binary_exists(g->binary) && file_exists(g->rom);
        int col = k / rows;
        int row = k % rows;
        if (col >= cols) col = cols - 1;

        if (i == selected) SDL_SetRenderDrawColor(ren, 0xff, 0xff, 0xff, 255);
        else if (!ok) SDL_SetRenderDrawColor(ren, 0xff, 0x70, 0x70, 255);
        else SDL_SetRenderDrawColor(ren, 0x60, 0x6a, 0x80, 255);

        int x = col * colW + (colW - text_width(g->title, es)) / 2;
        int y = startY + row * step + (step - 7 * es) / 2;
        draw_text(ren, x, y, es, g->title);
      }
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
