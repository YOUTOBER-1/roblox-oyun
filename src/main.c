/*
 * Roblox Tarzı 3D Oyun - SDL2 ile C
 * Windows & Linux destekli
 * Derleme: gcc main.c -o game $(sdl2-config --cflags --libs) -lm
 */

#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* --- Sabitler --- */
#define SCREEN_W     900
#define SCREEN_H     600
#define TILE_SIZE    40
#define MAP_W        20
#define MAP_H        20
#define MAX_COINS    30
#define MAX_ENEMIES  5
#define FPS          60
#define FRAME_TIME   (1000 / FPS)

#define PI 3.14159265358979323846

/* --- Renkler --- */
#define COL_SKY      0x87CEEBFF
#define COL_GROUND   0x4CAF50FF
#define COL_WALL     0xE57373FF
#define COL_PLAYER   0x42A5F5FF
#define COL_COIN     0xFFD700FF
#define COL_ENEMY    0xEF5350FF
#define COL_UI_BG    0x1A1A2EDD
#define COL_WHITE    0xFFFFFFFF
#define COL_YELLOW   0xFFEB3BFF

/* --- Harita --- */
static int map[MAP_H][MAP_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,1,0,1,0,0,0,0,0,0,1,0,0,1,0,0,0,1},
    {1,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

/* --- Tipler --- */
typedef struct {
    double x, y;       /* dünya konumu */
    double angle;      /* bakış açısı (radyan) */
    int health;
    int score;
    double velX, velY;
    int onGround;      /* 2D için kullanılmıyor, zıplama efekti */
} Player;

typedef struct {
    double x, y;
    int alive;
    int collected;
} Coin;

typedef struct {
    double x, y;
    double angle;
    int alive;
    double speed;
    int health;
} Enemy;

typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
} GameState;

/* --- Yardımcı: renk çiz --- */
static void set_color(SDL_Renderer *r, Uint32 col) {
    SDL_SetRenderDrawColor(r,
        (col >> 24) & 0xFF,
        (col >> 16) & 0xFF,
        (col >>  8) & 0xFF,
        (col      ) & 0xFF);
}

/* --- Dikdörtgen çiz --- */
static void fill_rect(SDL_Renderer *r, int x, int y, int w, int h, Uint32 col) {
    set_color(r, col);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}

/* --- Harita çarpışma --- */
static int map_solid(int mx, int my) {
    if (mx < 0 || mx >= MAP_W || my < 0 || my >= MAP_H) return 1;
    return map[my][mx] == 1;
}

/* --- 2.5D Raycasting ile duvar çizimi --- */
#define NUM_RAYS  SCREEN_W
#define FOV       (PI / 3.0)   /* 60 derece */
#define MAX_DIST  20.0

static void draw_3d_view(SDL_Renderer *rend, Player *p) {
    double ray_step = FOV / NUM_RAYS;
    double ray_angle = p->angle - FOV / 2.0;

    for (int col = 0; col < NUM_RAYS; col++) {
        /* Ray direction */
        double rdx = cos(ray_angle);
        double rdy = sin(ray_angle);

        /* DDA raycasting */
        double px = p->x, py = p->y;
        int mapX = (int)px, mapY = (int)py;

        double deltaX = fabs(1.0 / rdx);
        double deltaY = fabs(1.0 / rdy);

        double sideDistX, sideDistY;
        int stepX, stepY;
        int hit = 0, side = 0;

        if (rdx < 0) { stepX = -1; sideDistX = (px - mapX) * deltaX; }
        else          { stepX =  1; sideDistX = (mapX + 1.0 - px) * deltaX; }
        if (rdy < 0) { stepY = -1; sideDistY = (py - mapY) * deltaY; }
        else          { stepY =  1; sideDistY = (mapY + 1.0 - py) * deltaY; }

        double dist = 0;
        for (int step = 0; step < 100 && !hit; step++) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaY;
                mapY += stepY;
                side = 1;
            }
            if (map_solid(mapX, mapY)) {
                hit = 1;
                dist = (side == 0)
                    ? (mapX - px + (1 - stepX) / 2.0) / rdx
                    : (mapY - py + (1 - stepY) / 2.0) / rdy;
            }
        }

        /* Fish-eye düzeltme */
        double corr = dist * cos(ray_angle - p->angle);
        if (corr < 0.01) corr = 0.01;

        int wall_h = (int)(SCREEN_H / corr);
        int wall_top = (SCREEN_H - wall_h) / 2;
        int wall_bot = wall_top + wall_h;

        /* Duvar rengi (yan gölge) */
        Uint32 wcol = side ? 0xB71C1CFF : 0xE53935FF;

        /* Mesafeye göre karart */
        double brightness = 1.0 - (dist / MAX_DIST) * 0.7;
        if (brightness < 0.2) brightness = 0.2;
        int wr = (int)(((wcol >> 24) & 0xFF) * brightness);
        int wg = (int)(((wcol >> 16) & 0xFF) * brightness);
        int wb = (int)(((wcol >>  8) & 0xFF) * brightness);

        /* Zemin (alt yarı) */
        SDL_SetRenderDrawColor(rend, 76, 175, 80, 255);
        SDL_RenderDrawLine(rend, col, wall_bot, col, SCREEN_H);

        /* Tavan (üst yarı) */
        SDL_SetRenderDrawColor(rend, 135, 206, 235, 255);
        SDL_RenderDrawLine(rend, col, 0, col, wall_top);

        /* Duvar */
        SDL_SetRenderDrawColor(rend, wr, wg, wb, 255);
        SDL_RenderDrawLine(rend, col, wall_top, col, wall_bot);

        ray_angle += ray_step;
    }
}

/* --- 2D minimap --- */
static void draw_minimap(SDL_Renderer *rend, Player *p, Coin *coins, Enemy *enemies) {
    int ox = SCREEN_W - MAP_W * 5 - 10;
    int oy = 10;
    int ts = 5;

    /* Harita arkaplanı */
    fill_rect(rend, ox - 2, oy - 2, MAP_W * ts + 4, MAP_H * ts + 4, 0x000000AA);

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            Uint32 c = map[y][x] ? 0x880000FF : 0x333333FF;
            fill_rect(rend, ox + x * ts, oy + y * ts, ts - 1, ts - 1, c);
        }
    }

    /* Coinler */
    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].alive && !coins[i].collected) {
            int cx = ox + (int)(coins[i].x * ts);
            int cy = oy + (int)(coins[i].y * ts);
            fill_rect(rend, cx, cy, ts, ts, COL_COIN);
        }
    }

    /* Düşmanlar */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            int ex = ox + (int)(enemies[i].x * ts);
            int ey = oy + (int)(enemies[i].y * ts);
            fill_rect(rend, ex, ey, ts, ts, COL_ENEMY);
        }
    }

    /* Oyuncu */
    int px = ox + (int)(p->x * ts);
    int py = oy + (int)(p->y * ts);
    fill_rect(rend, px - 2, py - 2, ts + 1, ts + 1, COL_PLAYER);

    /* Yön oku */
    SDL_SetRenderDrawColor(rend, 255, 255, 0, 255);
    SDL_RenderDrawLine(rend, px, py,
        px + (int)(cos(p->angle) * 8),
        py + (int)(sin(p->angle) * 8));
}

/* --- Sprite (coin/düşman) ekranda çiz --- */
static void draw_sprite(SDL_Renderer *rend, Player *p,
                         double sx, double sy, Uint32 col, int size_base) {
    double dx = sx - p->x;
    double dy = sy - p->y;
    double dist = sqrt(dx * dx + dy * dy);
    if (dist < 0.1) return;

    /* Açı farkı */
    double angle_to = atan2(dy, dx);
    double rel = angle_to - p->angle;

    /* Normalize [-PI, PI] */
    while (rel >  PI) rel -= 2 * PI;
    while (rel < -PI) rel += 2 * PI;

    /* FOV dışındaysa çizme */
    if (fabs(rel) > FOV * 0.7) return;

    int screen_x = (int)((rel / FOV + 0.5) * SCREEN_W);
    int sprite_h = (int)(size_base / dist * SCREEN_H * 0.5);
    if (sprite_h < 4) return;
    if (sprite_h > SCREEN_H) sprite_h = SCREEN_H;

    int sprite_top = (SCREEN_H - sprite_h) / 2;

    /* Parlaklık */
    double br = 1.0 - dist / MAX_DIST * 0.8;
    if (br < 0.2) br = 0.2;
    int sr = (int)(((col >> 24) & 0xFF) * br);
    int sg = (int)(((col >> 16) & 0xFF) * br);
    int sb = (int)(((col >>  8) & 0xFF) * br);

    SDL_SetRenderDrawColor(rend, sr, sg, sb, 255);
    SDL_Rect rect = {screen_x - sprite_h / 2, sprite_top, sprite_h, sprite_h};
    SDL_RenderFillRect(rend, &rect);
}

/* --- HUD (skor, sağlık) --- */
static void draw_hud(SDL_Renderer *rend, Player *p, int coins_left) {
    /* Üst bar arkaplanı */
    fill_rect(rend, 0, 0, SCREEN_W, 40, 0x00000099);

    /* Sağlık barı */
    fill_rect(rend, 10, 10, 200, 20, 0xFF000099);
    fill_rect(rend, 10, 10, p->health * 2, 20, 0x4CAF50FF);

    /* Skor göstergesi (büyük sarı bloklar) */
    /* Sayıları piksel bloklarla çiz */
    char buf[64];
    snprintf(buf, sizeof(buf), "SKOR: %d  COIN: %d  HP: %d",
             p->score, coins_left, p->health);

    /* SDL_ttf olmadan basit gösterim: renkli bar + boyut */
    int score_w = (int)(p->score * 1.5);
    if (score_w > 300) score_w = 300;
    fill_rect(rend, 220, 10, score_w, 20, COL_COIN);

    /* Crosshair */
    int cx = SCREEN_W / 2, cy = SCREEN_H / 2;
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 200);
    SDL_RenderDrawLine(rend, cx - 10, cy, cx + 10, cy);
    SDL_RenderDrawLine(rend, cx, cy - 10, cx, cy + 10);

    /* WASD ipucu */
    fill_rect(rend, 10, SCREEN_H - 90, 180, 80, 0x000000AA);
    /* W tuşu */
    fill_rect(rend, 65, SCREEN_H - 85, 30, 25, 0x555555FF);
    fill_rect(rend, 68, SCREEN_H - 82, 24, 19, 0x777777FF);
    /* A S D tuşları */
    fill_rect(rend, 15, SCREEN_H - 55, 30, 25, 0x555555FF);
    fill_rect(rend, 65, SCREEN_H - 55, 30, 25, 0x555555FF);
    fill_rect(rend, 115, SCREEN_H - 55, 30, 25, 0x555555FF);
    fill_rect(rend, 18, SCREEN_H - 52, 24, 19, 0x777777FF);
    fill_rect(rend, 68, SCREEN_H - 52, 24, 19, 0x777777FF);
    fill_rect(rend, 118, SCREEN_H - 52, 24, 19, 0x777777FF);

    /* Sol/sağ ok: fare ile bak */
    fill_rect(rend, 200, SCREEN_H - 60, 120, 30, 0x000000AA);
    fill_rect(rend, 205, SCREEN_H - 55, 110, 20, 0x333399FF);
}

/* --- Menü --- */
static void draw_menu(SDL_Renderer *rend, int frame) {
    /* Arkaplan gradyan efekti */
    for (int y = 0; y < SCREEN_H; y++) {
        int r = 20 + y / 5;
        int g = 20 + y / 10;
        int b = 60 + y / 4;
        SDL_SetRenderDrawColor(rend, r, g, b, 255);
        SDL_RenderDrawLine(rend, 0, y, SCREEN_W, y);
    }

    /* Başlık bloğu */
    int tx = SCREEN_W / 2 - 200;
    int ty = 80 + (int)(sin(frame * 0.05) * 10);
    fill_rect(rend, tx, ty, 400, 80, 0xFF4444FF);
    fill_rect(rend, tx + 5, ty + 5, 390, 70, 0xFF6666FF);

    /* "ROBLOX TARZLI OYUN" yazı bloğu gösterimi */
    fill_rect(rend, tx + 20, ty + 20, 360, 12, COL_WHITE);
    fill_rect(rend, tx + 40, ty + 40, 280, 10, COL_YELLOW);

    /* Blok dekorasyonları */
    Uint32 block_colors[] = {
        0xFF4444FF, 0x44FF44FF, 0x4444FFFF,
        0xFFFF44FF, 0xFF44FFFF, 0x44FFFFFF
    };
    for (int i = 0; i < 6; i++) {
        int bx = 80 + i * 130 + (int)(sin(frame * 0.03 + i) * 15);
        int by = 200 + (int)(cos(frame * 0.04 + i * 0.5) * 20);
        fill_rect(rend, bx, by, 50, 50, block_colors[i]);
        fill_rect(rend, bx + 5, by + 5, 40, 40,
            block_colors[(i + 1) % 6]);
    }

    /* BAŞLA düğmesi */
    int btn_pulse = (int)(sin(frame * 0.1) * 10);
    fill_rect(rend, SCREEN_W / 2 - 120 - btn_pulse,
              350, 240 + btn_pulse * 2, 60, 0x00AA00FF);
    fill_rect(rend, SCREEN_W / 2 - 110 - btn_pulse,
              360, 220 + btn_pulse * 2, 40, 0x00CC00FF);

    /* Kontroller bilgisi */
    fill_rect(rend, SCREEN_W / 2 - 180, 440, 360, 100, 0x000000AA);
    fill_rect(rend, SCREEN_W / 2 - 170, 450, 340, 80, 0x111133AA);

    /* Kontrol ikonları */
    fill_rect(rend, SCREEN_W / 2 - 140, 460, 60, 20, 0x4444AAFF);
    fill_rect(rend, SCREEN_W / 2 - 70, 460, 80, 20, 0x4444AAFF);
    fill_rect(rend, SCREEN_W / 2 + 20, 460, 100, 20, 0x4444AAFF);
    fill_rect(rend, SCREEN_W / 2 - 140, 490, 280, 15, 0x555555FF);
    fill_rect(rend, SCREEN_W / 2 - 140, 510, 200, 15, 0x555555FF);
}

/* --- Oyun bitti --- */
static void draw_gameover(SDL_Renderer *rend, Player *p) {
    fill_rect(rend, 0, 0, SCREEN_W, SCREEN_H, 0x8B0000CC);
    fill_rect(rend, SCREEN_W / 2 - 200, SCREEN_H / 2 - 80,
              400, 160, 0x330000FF);
    fill_rect(rend, SCREEN_W / 2 - 190, SCREEN_H / 2 - 70,
              380, 60, 0x660000FF);
    /* Skor çubuğu */
    fill_rect(rend, SCREEN_W / 2 - 100, SCREEN_H / 2 + 10,
              p->score * 2 > 200 ? 200 : p->score * 2, 20, COL_YELLOW);
    fill_rect(rend, SCREEN_W / 2 - 190, SCREEN_H / 2 + 50,
              380, 30, 0x994444FF);
}

/* --- Kazandın --- */
static void draw_win(SDL_Renderer *rend, Player *p, int frame) {
    for (int y = 0; y < SCREEN_H; y++) {
        int g = 100 + y / 4 + (int)(sin(frame * 0.05 + y * 0.01) * 30);
        SDL_SetRenderDrawColor(rend, 20, g > 255 ? 255 : g, 60, 255);
        SDL_RenderDrawLine(rend, 0, y, SCREEN_W, y);
    }
    fill_rect(rend, SCREEN_W / 2 - 200, SCREEN_H / 2 - 80,
              400, 160, 0x00440088);
    fill_rect(rend, SCREEN_W / 2 - 190, SCREEN_H / 2 - 70,
              380, 60, 0x00880088);
    /* Kutlama blokları */
    for (int i = 0; i < 10; i++) {
        int bx = (int)(rand() % SCREEN_W);
        int by = (int)(sin(frame * 0.1 + i) * SCREEN_H / 2 + SCREEN_H / 2);
        fill_rect(rend, bx, by, 20, 20,
            (Uint32)(rand() | 0xFF));
    }
    fill_rect(rend, SCREEN_W / 2 - 100, SCREEN_H / 2 + 10,
              (p->score * 2 > 200 ? 200 : p->score * 2), 20, COL_COIN);
}

/* --- Coin toplama kontrolü --- */
static void check_coins(Player *p, Coin *coins) {
    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].alive || coins[i].collected) continue;
        double dx = coins[i].x - p->x;
        double dy = coins[i].y - p->y;
        if (dx * dx + dy * dy < 0.3 * 0.3) {
            coins[i].collected = 1;
            p->score += 10;
        }
    }
}

/* --- Düşman hareketi --- */
static void update_enemies(Enemy *enemies, Player *p) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        double dx = p->x - enemies[i].x;
        double dy = p->y - enemies[i].y;
        double dist = sqrt(dx * dx + dy * dy);
        if (dist < 0.1) continue;

        /* Oyuncuya doğru yürü */
        double nx = dx / dist * enemies[i].speed;
        double ny = dy / dist * enemies[i].speed;

        double newX = enemies[i].x + nx;
        double newY = enemies[i].y + ny;

        /* Duvar çarpışması */
        int mx = (int)newX, my = (int)newY;
        if (!map_solid(mx, (int)enemies[i].y)) enemies[i].x = newX;
        if (!map_solid((int)enemies[i].x, my)) enemies[i].y = newY;

        enemies[i].angle = atan2(dy, dx);

        /* Oyuncuya çarptı mı? */
        if (dist < 0.5) {
            p->health -= 1;
        }
    }
}

/* --- Hareket --- */
static void move_player(Player *p, const Uint8 *keys, double dt) {
    double speed = 3.0 * dt;
    double turn  = 2.0 * dt;

    /* Dönüş */
    if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_Q]) p->angle -= turn;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_E]) p->angle += turn;

    /* İleri/geri */
    double moveX = 0, moveY = 0;
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
        moveX += cos(p->angle) * speed;
        moveY += sin(p->angle) * speed;
    }
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
        moveX -= cos(p->angle) * speed;
        moveY -= sin(p->angle) * speed;
    }
    /* Yan kayma */
    if (keys[SDL_SCANCODE_A]) {
        moveX += cos(p->angle - PI / 2) * speed;
        moveY += sin(p->angle - PI / 2) * speed;
    }
    if (keys[SDL_SCANCODE_D]) {
        moveX += cos(p->angle + PI / 2) * speed;
        moveY += sin(p->angle + PI / 2) * speed;
    }

    /* Çarpışma (ayrı eksen) */
    double nx = p->x + moveX;
    double ny = p->y + moveY;
    double r = 0.25;

    if (!map_solid((int)(nx + r), (int)p->y) &&
        !map_solid((int)(nx - r), (int)p->y))
        p->x = nx;
    if (!map_solid((int)p->x, (int)(ny + r)) &&
        !map_solid((int)p->x, (int)(ny - r)))
        p->y = ny;
}

/* --- FARE ile kamera --- */
static void handle_mouse(Player *p, int rel_x) {
    p->angle += rel_x * 0.003;
}

/* --- Ana fonksiyon --- */
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL init hata: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Roblox Tarzli Oyun - C ile Yapildi!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, 0);
    if (!win) {
        fprintf(stderr, "Pencere hatasi: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *rend = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!rend) {
        rend = SDL_CreateRenderer(win, -1, 0); /* software fallback */
    }

    /* --- Oyuncu --- */
    Player player = {
        .x = 2.5, .y = 2.5,
        .angle = 0,
        .health = 100,
        .score = 0
    };

    /* --- Coinler --- */
    Coin coins[MAX_COINS];
    int coin_positions[][2] = {
        {3,3},{5,5},{7,2},{9,9},{11,5},{13,3},{15,7},{17,2},{3,10},
        {6,12},{8,8},{10,14},{12,10},{14,6},{16,12},{4,16},{7,15},
        {10,17},{13,15},{16,16},{2,7},{18,7},{5,13},{15,13},{9,3},
        {11,18},{3,17},{17,17},{6,6},{14,14}
    };
    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].x = coin_positions[i][0] + 0.5;
        coins[i].y = coin_positions[i][1] + 0.5;
        coins[i].alive = 1;
        coins[i].collected = 0;
    }

    /* --- Düşmanlar --- */
    Enemy enemies[MAX_ENEMIES];
    int epos[][2] = {{10,10},{5,15},{15,5},{12,3},{3,12}};
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x = epos[i][0] + 0.5;
        enemies[i].y = epos[i][1] + 0.5;
        enemies[i].alive = 1;
        enemies[i].speed = 0.015 + i * 0.005;
        enemies[i].health = 3;
        enemies[i].angle = 0;
    }

    GameState state = STATE_MENU;
    int running = 1;
    int frame = 0;
    Uint32 last_time = SDL_GetTicks();
    int mouse_captured = 0;

    while (running) {
        Uint32 now = SDL_GetTicks();
        double dt = (now - last_time) / 1000.0;
        last_time = now;
        if (dt > 0.1) dt = 0.1;
        frame++;

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = 0; break; }

            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE) {
                    if (state == STATE_PLAYING) {
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                        mouse_captured = 0;
                        state = STATE_MENU;
                    } else {
                        running = 0;
                    }
                }
                if (ev.key.keysym.sym == SDLK_RETURN ||
                    ev.key.keysym.sym == SDLK_SPACE) {
                    if (state == STATE_MENU) {
                        /* Oyunu sıfırla */
                        player.x = 2.5; player.y = 2.5;
                        player.angle = 0;
                        player.health = 100;
                        player.score = 0;
                        for (int i = 0; i < MAX_COINS; i++)
                            coins[i].collected = 0;
                        for (int i = 0; i < MAX_ENEMIES; i++) {
                            enemies[i].x = epos[i][0] + 0.5;
                            enemies[i].y = epos[i][1] + 0.5;
                            enemies[i].alive = 1;
                            enemies[i].health = 3;
                        }
                        state = STATE_PLAYING;
                        SDL_SetRelativeMouseMode(SDL_TRUE);
                        mouse_captured = 1;
                    } else if (state == STATE_GAMEOVER ||
                               state == STATE_WIN) {
                        state = STATE_MENU;
                    }
                }
                /* Ateş et (boşluk) */
                if (ev.key.keysym.sym == SDLK_f && state == STATE_PLAYING) {
                    for (int i = 0; i < MAX_ENEMIES; i++) {
                        if (!enemies[i].alive) continue;
                        double dx = enemies[i].x - player.x;
                        double dy = enemies[i].y - player.y;
                        double dist = sqrt(dx * dx + dy * dy);
                        double angle_to = atan2(dy, dx);
                        double rel = angle_to - player.angle;
                        while (rel >  PI) rel -= 2 * PI;
                        while (rel < -PI) rel += 2 * PI;
                        if (dist < 3.0 && fabs(rel) < 0.3) {
                            enemies[i].health--;
                            if (enemies[i].health <= 0) {
                                enemies[i].alive = 0;
                                player.score += 50;
                            }
                        }
                    }
                }
            }

            if (ev.type == SDL_MOUSEBUTTONDOWN && state == STATE_PLAYING) {
                if (!mouse_captured) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    mouse_captured = 1;
                } else {
                    /* Sol tık: ateş */
                    for (int i = 0; i < MAX_ENEMIES; i++) {
                        if (!enemies[i].alive) continue;
                        double dx = enemies[i].x - player.x;
                        double dy = enemies[i].y - player.y;
                        double dist = sqrt(dx * dx + dy * dy);
                        double angle_to = atan2(dy, dx);
                        double rel = angle_to - player.angle;
                        while (rel >  PI) rel -= 2 * PI;
                        while (rel < -PI) rel += 2 * PI;
                        if (dist < 4.0 && fabs(rel) < 0.4) {
                            enemies[i].health--;
                            if (enemies[i].health <= 0) {
                                enemies[i].alive = 0;
                                player.score += 50;
                            }
                        }
                    }
                }
            }

            if (ev.type == SDL_MOUSEMOTION && state == STATE_PLAYING &&
                mouse_captured) {
                handle_mouse(&player, ev.motion.xrel);
            }
        }

        /* --- Güncelle --- */
        if (state == STATE_PLAYING) {
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            move_player(&player, keys, dt);
            update_enemies(enemies, &player);
            check_coins(&player, coins);

            /* Kazanma/kaybetme */
            if (player.health <= 0) state = STATE_GAMEOVER;
            int uncollected = 0;
            for (int i = 0; i < MAX_COINS; i++)
                if (!coins[i].collected) uncollected++;
            if (uncollected == 0) state = STATE_WIN;
        }

        /* --- Çiz --- */
        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);

        if (state == STATE_MENU) {
            draw_menu(rend, frame);
        } else if (state == STATE_PLAYING) {
            draw_3d_view(rend, &player);
            /* Spritelar */
            for (int i = 0; i < MAX_COINS; i++) {
                if (!coins[i].collected)
                    draw_sprite(rend, &player, coins[i].x, coins[i].y,
                                COL_COIN, 1);
            }
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].alive)
                    draw_sprite(rend, &player, enemies[i].x, enemies[i].y,
                                COL_ENEMY, 1);
            }
            int uncollected = 0;
            for (int i = 0; i < MAX_COINS; i++)
                if (!coins[i].collected) uncollected++;
            draw_minimap(rend, &player, coins, enemies);
            draw_hud(rend, &player, uncollected);
        } else if (state == STATE_GAMEOVER) {
            draw_gameover(rend, &player);
        } else if (state == STATE_WIN) {
            draw_win(rend, &player, frame);
        }

        SDL_RenderPresent(rend);

        /* FPS limiti */
        Uint32 elapsed = SDL_GetTicks() - now;
        if (elapsed < FRAME_TIME)
            SDL_Delay(FRAME_TIME - elapsed);
    }

    if (mouse_captured) SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
