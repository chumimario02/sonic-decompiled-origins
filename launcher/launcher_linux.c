/*
 * Sonic Decompiled Origins - Linux launcher
 *
 * SDL2 menu for the Linux builds:
 *   SONIC 1 (RSDKv4) / SONIC 2 (RSDKv4) / SONIC 3 & K (recomp) / SONIC CD (RSDKv3)
 *
 * Keyboard + gamepad. Alt+Enter toggles fullscreen.
 *
 * Build:  gcc launcher_linux.c -o launcher `pkg-config --cflags --libs sdl2` -lm
 */
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

#define WINDOW_W 640
#define WINDOW_H 480

struct game {
  const char *title;   /* "SONIC 1" */
  const char *hint;    /* "RSDK" / "RECOMP" */
  const char *binary;  /* path relative to workdir */
  const char *rom;     /* data/rom path relative to workdir */
  const char *args;    /* extra single arg or "" */
  const char *cwd;     /* working dir relative to workdir or NULL */
  const char *env_poke;/* GENESIS_POKE value or NULL */
};

static const struct game games[] = {
  { "SONIC 1",      "RSDK",   "Sonic12v4/linux-out/RSDKv4",       "Sonic12v4/linux-out/Data.rsdk",    "usingCWD=true", "Sonic12v4/linux-out",     NULL },
  { "SONIC 2",      "RSDK",   "Sonic12v4/linux-out-s2/RSDKv4",    "Sonic12v4/linux-out-s2/Data.rsdk", "usingCWD=true", "Sonic12v4/linux-out-s2",  NULL },
  { "SONIC 3 & K",  "RECOMP", "Sonic3AndKnucklesRecomp/linux-out/Sonic3KRecomp", "Sonic3AndKnucklesRecomp/linux-out/sonic3k.bin", "", "Sonic3AndKnucklesRecomp/linux-out", NULL },
  { "SONIC CD",     "RSDK",   "SonicCDv3/linux-out/RSDKv3",       "SonicCDv3/linux-out/Data.rsdk",    "",              "SonicCDv3/linux-out",     NULL },
  { "SONIC CHAOS",  "RECOMP", "SonicChaosRecomp/linux-out/SonicChaosRecomp", "SonicChaosRecomp/linux-out/sonicchaos.gg", "--gg --window 3", "SonicChaosRecomp/linux-out", NULL },
  { "SONIC TT",     "RECOMP", "SonicTripleTroubleSMSRecomp/linux-out/SonicTripleTroubleSMSRecomp", "SonicTripleTroubleSMSRecomp/linux-out/sonicttsms.sms", "--sms --interp --window 3", "SonicTripleTroubleSMSRecomp/linux-out", NULL },
  { "SONIC 2 SMS",  "RECOMP", "Sonic2SMSRecomp/linux-out/Sonic2SMSRecomp", "Sonic2SMSRecomp/linux-out/sonic2sms.sms", "--sms --window 3", "Sonic2SMSRecomp/linux-out", NULL },
  { "TAILS ADV",    "RECOMP", "TailsAdventureRecomp/linux-out/TailsAdventureRecomp", "TailsAdventureRecomp/linux-out/tailsadv.gg", "--gg --window 3", "TailsAdventureRecomp/linux-out", NULL },
  { "SRB2",         "FAN",    "SonicRoboBlast2/srb2", "SonicRoboBlast2/srb2.pk3", "NOROM", "SonicRoboBlast2", NULL },
};
static const int game_count = (int)(sizeof(games) / sizeof(games[0]));

/* Category: 0 = Genesis / 16-bit (+Sega CD), 1 = Master System / Game Gear. */
static const int game_cat[] = { 0, 0, 0, 0, 1, 1, 1, 1, 2 };
static const char *cat_names[] = { "GENESIS / 16-BIT", "SMS / GAME GEAR", "EXTRA / FAN" };
#define CAT_COUNT 3

static int cat_first(int c) {
  for (int i = 0; i < game_count; i++) if (game_cat[i] == c) return i;
  return 0;
}
static int cat_count_of(int c) {
  int n = 0;
  for (int i = 0; i < game_count; i++) if (game_cat[i] == c) n++;
  return n;
}
static int cat_step(int cur, int dir) {
  for (int k = 1; k <= game_count; k++) {
    int i = (cur + dir * k + game_count * 4) % game_count;
    if (game_cat[i] == game_cat[cur]) return i;
  }
  return cur;
}

static char g_workdir[PATH_MAX];

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
  for (int row = 0; row < 7; row++)
    for (int col = 0; col < 5; col++)
      if (glyph[row] & (1 << (4 - col))) {
        SDL_Rect px = {x + col * scale, y + row * scale, scale, scale};
        SDL_RenderFillRect(r, &px);
      }
}

static void draw_text(SDL_Renderer *r, int x, int y, int scale, const char *s) {
  for (; *s; s++) { draw_char(r, x, y, scale, *s); x += 6 * scale; }
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

static int join(char *out, size_t n, const char *a, const char *b) {
  return snprintf(out, n, "%s/%s", a, b) < (int)n;
}

static int path_ok(const char *rel) {
  char p[PATH_MAX];
  join(p, sizeof(p), g_workdir, rel);
  FILE *f = fopen(p, "rb");
  if (!f) return 0;
  fclose(f);
  return 1;
}

static void launch(const struct game *g) {
  if (!path_ok(g->binary) || !path_ok(g->rom)) return;

  char exe[PATH_MAX], rom[PATH_MAX], cwd[PATH_MAX];
  join(exe, sizeof(exe), g_workdir, g->binary);
  join(rom, sizeof(rom), g_workdir, g->rom);
  if (g->cwd && g->cwd[0]) join(cwd, sizeof(cwd), g_workdir, g->cwd);
  else snprintf(cwd, sizeof(cwd), "%s", g_workdir);

  FILE *lg = fopen("launcher.log", "a");
  pid_t pid = fork();
  if (pid == 0) {
    chdir(cwd);
    if (g->env_poke) setenv("GENESIS_POKE", g->env_poke, 1);
    /* Prefer per-game shared libs (e.g. libtheoradec.so.2 -> .so.1 on Pi OS). */
    {
      char libdir[PATH_MAX];
      snprintf(libdir, sizeof(libdir), "%s/lib", cwd);
      if (access(libdir, F_OK) == 0) {
        const char *old = getenv("LD_LIBRARY_PATH");
        char newlp[2 * PATH_MAX];
        if (old && old[0]) snprintf(newlp, sizeof(newlp), "%s:%s", libdir, old);
        else snprintf(newlp, sizeof(newlp), "%s", libdir);
        setenv("LD_LIBRARY_PATH", newlp, 1);
      }
    }
    /* Split extra args on spaces, then append the ROM/data path.
       "NOROM" = the game loads its own data (e.g. SRB2). */
    char argsbuf[256];
    char *argv[16];
    int argc = 0;
    const char *extra = g->args;
    int norom = 0;
    if (extra && strncmp(extra, "NOROM", 5) == 0) {
      norom = 1;
      extra += 5;
      if (*extra == ' ') extra++;
    }
    argv[argc++] = exe;
    if (extra && extra[0]) {
      snprintf(argsbuf, sizeof(argsbuf), "%s", extra);
      for (char *tok = strtok(argsbuf, " "); tok && argc < 14; tok = strtok(NULL, " "))
        argv[argc++] = tok;
    }
    if (!norom) argv[argc++] = rom;
    argv[argc] = NULL;
    execv(exe, argv);
    _exit(127);
  } else if (pid > 0) {
    if (lg) { fprintf(lg, "spawn pid=%d cwd=%s cmd=%s %s %s\n", (int)pid, cwd, exe, g->args ? g->args : "", rom); fclose(lg); }
    waitpid(pid, NULL, 0);
  } else {
    if (lg) { fprintf(lg, "fork FAIL errno=%d\n", errno); fclose(lg); }
  }
}

static void resolve_workdir(void) {
  char start[PATH_MAX];
  if (!getcwd(start, sizeof(start))) { snprintf(g_workdir, sizeof(g_workdir), "."); return; }
  char cur[PATH_MAX];
  snprintf(cur, sizeof(cur), "%s", start);
  for (int depth = 0; depth < 8; depth++) {
    char probe[PATH_MAX];
    snprintf(probe, sizeof(probe), "%s/Sonic12v4", cur);
    if (access(probe, F_OK) == 0) { snprintf(g_workdir, sizeof(g_workdir), "%s", cur); return; }
    char *slash = strrchr(cur, '/');
    if (!slash || slash == cur) break;
    *slash = 0;
  }
  snprintf(g_workdir, sizeof(g_workdir), "%s", start);
}

int main(int argc, char **argv) {
  resolve_workdir();

  if (argc >= 3 && strcmp(argv[1], "--launch") == 0) {
    int idx = atoi(argv[2]);
    if (idx >= 0 && idx < game_count) launch(&games[idx]);
    return 0;
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return 1;
  }

  int winW = WINDOW_W, winH = WINDOW_H, want_full = 1;
  Uint32 winFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP;
  const char *ws = getenv("SONIC_WINSIZE");
  if (ws) {
    int a = 0, b = 0;
    if (sscanf(ws, "%dx%d", &a, &b) == 2 && a > 0 && b > 0) {
      winW = a; winH = b; want_full = 0; winFlags = SDL_WINDOW_SHOWN;
    }
  }
  SDL_Window *win = SDL_CreateWindow("SONIC DECOMPILED ORIGINS",
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     winW, winH, winFlags);
  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);

  SDL_GameController *pad = NULL;
  for (int i = 0; i < SDL_NumJoysticks(); i++)
    if (SDL_IsGameController(i)) { pad = SDL_GameControllerOpen(i); if (pad) break; }

  int selected = 0, running = 1, fullscreen = want_full;
  int level = 0, sel_cat = 0;   /* 0 = category menu, 1 = game menu */
  int snap_done = 0;
  Uint32 lastNav = 0;

  { /* dev/test: start directly on a given menu level */
    const char *ml = getenv("SONIC_MENU_LEVEL");
    if (ml) {
      int l = atoi(ml);
      if (l >= 0 && l <= 1) level = l;
      const char *mc = getenv("SONIC_MENU_CAT");
      if (mc) { int c = atoi(mc); if (c >= 0 && c < CAT_COUNT) sel_cat = c; }
      if (level == 1) selected = cat_first(sel_cat);
    }
  }

  while (running) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) running = 0;
      else if (ev.type == SDL_KEYDOWN) {
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
      } else if (ev.type == SDL_CONTROLLERDEVICEADDED) {
        if (!pad) pad = SDL_GameControllerOpen(ev.cdevice.which);
      } else if (ev.type == SDL_CONTROLLERDEVICEREMOVED) {
        if (pad && ev.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad))) {
          SDL_GameControllerClose(pad); pad = NULL;
        }
      } else if (ev.type == SDL_CONTROLLERBUTTONDOWN) {
        switch (ev.cbutton.button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
          if (level == 0) sel_cat = (sel_cat + CAT_COUNT - 1) % CAT_COUNT;
          else selected = cat_step(selected, -1);
          lastNav = SDL_GetTicks(); break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
          if (level == 0) sel_cat = (sel_cat + 1) % CAT_COUNT;
          else selected = cat_step(selected, +1);
          lastNav = SDL_GetTicks(); break;
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
      } else if (ev.type == SDL_CONTROLLERAXISMOTION) {
        Uint32 now = SDL_GetTicks();
        if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY && now - lastNav > 180) {
          if (ev.caxis.value < -16000) {
            if (level == 0) sel_cat = (sel_cat + CAT_COUNT - 1) % CAT_COUNT;
            else selected = cat_step(selected, -1);
            lastNav = now;
          } else if (ev.caxis.value > 16000) {
            if (level == 0) sel_cat = (sel_cat + 1) % CAT_COUNT;
            else selected = cat_step(selected, +1);
            lastNav = now;
          }
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

    /* Title: short on tiny screens (240x240 handhelds). */
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

    int startY = hy + 7 * hs + H / 16;
    int avail = H - startY - H / 16;

    if (level == 0) {
      /* Category menu */
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
      /* Game menu for the selected category */
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
        const int ok = path_ok(g->binary) && path_ok(g->rom);
        int col = k / rows;
        int row = k % rows;
        if (col >= cols) col = cols - 1;

        if (i == selected) SDL_SetRenderDrawColor(ren, 0xff, 0xff, 0xff, 255);
        else if (!ok) SDL_SetRenderDrawColor(ren, 0xff, 0x70, 0x70, 255);
        else SDL_SetRenderDrawColor(ren, 0x60, 0x6a, 0x80, 255);

        int x = col * colW + (colW - text_width(g->title, es)) / 2;
        int ly = startY + row * step + (step - 7 * es) / 2;
        draw_text(ren, x, ly, es, g->title);
      }
    }

    SDL_RenderPresent(ren);

    /* Optional one-shot snapshot (preview / QA). */
    if (!snap_done) {
      const char *shot = getenv("SONIC_SNAPSHOT");
      if (shot && shot[0]) {
        SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, SDL_PIXELFORMAT_ARGB8888);
        if (s) {
          SDL_RenderReadPixels(ren, NULL, SDL_PIXELFORMAT_ARGB8888, s->pixels, s->pitch);
          SDL_SaveBMP(s, shot);
          SDL_FreeSurface(s);
        }
        snap_done = 1;
        running = 0;
      }
    }
  }

  if (pad) SDL_GameControllerClose(pad);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
