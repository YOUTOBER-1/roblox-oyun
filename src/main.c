/*
 * ROBLOX TARZLI 3D OYUN
 * SDL2 + OpenGL + GLU - Gercek 3D Perspektif
 * Derleme: gcc main.c -o game $(sdl2-config --cflags --libs) -lGL -lGLU -lm
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ===== SABİTLER ===== */
#define SCREEN_W     1024
#define SCREEN_H     640
#define FPS          60
#define FRAME_TIME   (1000 / FPS)
#define PI           3.14159265358979323846

#define MAP_W        20
#define MAP_H        20
#define MAX_COINS    25
#define MAX_ENEMIES  6

/* ===== HARİTA ===== */
static int map[MAP_H][MAP_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

/* ===== TİPLER ===== */
typedef struct { float r, g, b; } Color3;

typedef struct {
    double x, y, z;   /* pozisyon */
    double yaw;        /* sol-sag bakis (radyan) */
    double pitch;      /* yukari-asagi bakis */
    int health;
    int score;
} Player;

typedef struct {
    double x, y, z;
    int collected;
    double spin;
} Coin;

typedef struct {
    double x, y, z;
    int alive;
    int health;
    double speed;
    double spin;
} Enemy;

typedef enum { STATE_MENU, STATE_PLAYING, STATE_GAMEOVER, STATE_WIN } GameState;

/* ===== YARDIMCI FONKSİYONLAR ===== */
static int map_solid(int mx, int my) {
    if (mx < 0 || mx >= MAP_W || my < 0 || my >= MAP_H) return 1;
    return map[my][mx] == 1;
}

/* ===== 3D KÜP ÇİZ ===== */
static void draw_cube(double x, double y, double z,
                      double sx, double sy, double sz,
                      Color3 top, Color3 side, Color3 front) {
    double x0 = x,       x1 = x + sx;
    double y0 = y,       y1 = y + sy;
    double z0 = z,       z1 = z + sz;

    glBegin(GL_QUADS);

    /* ÜST yüz */
    glColor3f(top.r, top.g, top.b);
    glNormal3f(0, 1, 0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1);

    /* ALT yüz */
    glColor3f(side.r * 0.5f, side.g * 0.5f, side.b * 0.5f);
    glNormal3f(0, -1, 0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x0, y0, z1);

    /* ÖN yüz (z+) */
    glColor3f(front.r, front.g, front.b);
    glNormal3f(0, 0, 1);
    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1);

    /* ARKA yüz (z-) */
    glColor3f(side.r * 0.8f, side.g * 0.8f, side.b * 0.8f);
    glNormal3f(0, 0, -1);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x0, y1, z0);

    /* SAĞ yüz (x+) */
    glColor3f(side.r * 0.7f, side.g * 0.7f, side.b * 0.7f);
    glNormal3f(1, 0, 0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z0);

    /* SOL yüz (x-) */
    glColor3f(side.r * 0.6f, side.g * 0.6f, side.b * 0.6f);
    glNormal3f(-1, 0, 0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y0, z1);
    glVertex3f(x0, y1, z1);
    glVertex3f(x0, y1, z0);

    glEnd();
}

/* ===== ZEMİN ÇİZ ===== */
static void draw_floor(void) {
    int tile = 0;
    glBegin(GL_QUADS);
    for (int z = 0; z < MAP_H; z++) {
        for (int x = 0; x < MAP_W; x++) {
            tile = (x + z) % 2;
            if (tile == 0)
                glColor3f(0.35f, 0.65f, 0.30f);
            else
                glColor3f(0.30f, 0.58f, 0.25f);
            glNormal3f(0, 1, 0);
            glVertex3f((float)x,     0.0f, (float)z);
            glVertex3f((float)x+1.f, 0.0f, (float)z);
            glVertex3f((float)x+1.f, 0.0f, (float)z+1.f);
            glVertex3f((float)x,     0.0f, (float)z+1.f);
        }
    }
    glEnd();
}

/* ===== HARİTA DUVARLARI ===== */
static void draw_map(void) {
    /* Duvar renkleri - Roblox tarzı parlak renkler */
    Color3 wall_colors[] = {
        {0.85f, 0.25f, 0.25f},  /* kırmızı */
        {0.25f, 0.55f, 0.85f},  /* mavi */
        {0.85f, 0.65f, 0.20f},  /* sarı */
        {0.35f, 0.75f, 0.35f},  /* yeşil */
        {0.70f, 0.30f, 0.80f},  /* mor */
    };

    for (int z = 0; z < MAP_H; z++) {
        for (int x = 0; x < MAP_W; x++) {
            if (!map[z][x]) continue;
            int ci = (x * 3 + z * 7) % 5;
            Color3 c = wall_colors[ci];
            Color3 top = {c.r * 1.2f > 1 ? 1 : c.r * 1.2f,
                          c.g * 1.2f > 1 ? 1 : c.g * 1.2f,
                          c.b * 1.2f > 1 ? 1 : c.b * 1.2f};
            draw_cube((double)x, 0.0, (double)z,
                      1.0, 2.0, 1.0,
                      top, c, c);
        }
    }
}

/* ===== COİN ÇİZ ===== */
static void draw_coins(Coin *coins) {
    Color3 gold_top  = {1.0f, 0.95f, 0.2f};
    Color3 gold_side = {1.0f, 0.75f, 0.0f};

    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].collected) continue;
        coins[i].spin += 0.03;

        glPushMatrix();
        glTranslatef((float)coins[i].x + 0.5f,
                     (float)coins[i].y,
                     (float)coins[i].z + 0.5f);
        glRotatef((float)(coins[i].spin * 180.0 / PI), 0, 1, 0);
        /* Hafif yukarı aşağı sallanma */
        float bob = (float)(sin(coins[i].spin * 2.0) * 0.15);
        glTranslatef(0, bob, 0);
        glScalef(0.4f, 0.4f, 0.4f);
        glTranslatef(-0.5f, 0, -0.5f);
        draw_cube(0, 0, 0, 1, 1, 1, gold_top, gold_side, gold_side);
        glPopMatrix();
    }
}

/* ===== DÜŞMAN ÇİZ ===== */
static void draw_enemies(Enemy *enemies) {
    Color3 red_top  = {1.0f, 0.2f, 0.2f};
    Color3 red_side = {0.8f, 0.1f, 0.1f};
    Color3 eye      = {1.0f, 1.0f, 1.0f};
    Color3 pupil    = {0.0f, 0.0f, 0.0f};

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        enemies[i].spin += 0.02;

        glPushMatrix();
        glTranslatef((float)enemies[i].x,
                     0.0f,
                     (float)enemies[i].z);
        glRotatef((float)(enemies[i].spin * 180.0 / PI), 0, 1, 0);

        /* Gövde */
        draw_cube(-0.4f, 0, -0.4f, 0.8f, 1.4f, 0.8f, red_top, red_side, red_side);

        /* Baş */
        draw_cube(-0.35f, 1.4f, -0.35f, 0.7f, 0.7f, 0.7f, red_top, red_side, red_side);

        /* Gözler */
        draw_cube(-0.2f, 1.75f, 0.35f, 0.15f, 0.15f, 0.05f, eye, eye, eye);
        draw_cube( 0.05f, 1.75f, 0.35f, 0.15f, 0.15f, 0.05f, eye, eye, eye);

        /* Göz bebekleri */
        draw_cube(-0.16f, 1.77f, 0.40f, 0.07f, 0.07f, 0.05f, pupil, pupil, pupil);
        draw_cube( 0.09f, 1.77f, 0.40f, 0.07f, 0.07f, 0.05f, pupil, pupil, pupil);

        glPopMatrix();
    }
}

/* ===== GÖKYÜZÜ ÇİZ ===== */
static void draw_sky(void) {
    /* Gökyüzü gradient */
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
    glColor3f(0.40f, 0.70f, 1.0f);
    glVertex2f(-1,  1);
    glVertex2f( 1,  1);
    glColor3f(0.65f, 0.85f, 1.0f);
    glVertex2f( 1, -1);
    glVertex2f(-1, -1);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

/* ===== 2D HUD ===== */
static void draw_hud_2d(Player *p, int coins_left, int total_coins) {
    /* 2D moduna geç */
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, SCREEN_W, SCREEN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /* Üst bar */
    glColor4f(0, 0, 0, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(SCREEN_W, 0);
    glVertex2f(SCREEN_W, 45); glVertex2f(0, 45);
    glEnd();
    glDisable(GL_BLEND);

    /* HP barı */
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(10, 10); glVertex2f(210, 10);
    glVertex2f(210, 30); glVertex2f(10, 30);
    glEnd();

    float hp_frac = p->health / 100.0f;
    glColor3f(0.1f + (1-hp_frac)*0.8f, hp_frac * 0.8f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(10, 10); glVertex2f(10 + 200*hp_frac, 10);
    glVertex2f(10 + 200*hp_frac, 30); glVertex2f(10, 30);
    glEnd();

    /* Coin göstergesi */
    glColor3f(1.0f, 0.85f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(220, 10); glVertex2f(220 + coins_left * 8, 10);
    glVertex2f(220 + coins_left * 8, 30); glVertex2f(220, 30);
    glEnd();

    /* Skor barı */
    glColor3f(0.3f, 0.6f, 1.0f);
    int sw = p->score > 400 ? 400 : p->score;
    glBegin(GL_QUADS);
    glVertex2f(430, 10); glVertex2f(430 + sw, 10);
    glVertex2f(430 + sw, 30); glVertex2f(430, 30);
    glEnd();

    /* Crosshair */
    int cx = SCREEN_W / 2, cy = SCREEN_H / 2;
    glColor3f(1, 1, 1);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(cx - 12, cy); glVertex2f(cx + 12, cy);
    glVertex2f(cx, cy - 12); glVertex2f(cx, cy + 12);
    glEnd();
    /* Crosshair iç nokta */
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx, cy);
    glEnd();

    /* Alt sol ipucu */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0, 0, 0, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(5, SCREEN_H - 90);
    glVertex2f(215, SCREEN_H - 90);
    glVertex2f(215, SCREEN_H - 5);
    glVertex2f(5, SCREEN_H - 5);
    glEnd();
    glDisable(GL_BLEND);

    /* WASD tuşları görsel */
    float keys[][4] = {
        {70, SCREEN_H-85, 30, 22},   /* W */
        {35, SCREEN_H-58, 30, 22},   /* A */
        {70, SCREEN_H-58, 30, 22},   /* S */
        {105, SCREEN_H-58, 30, 22},  /* D */
    };
    for (int i = 0; i < 4; i++) {
        glColor3f(0.4f, 0.4f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2f(keys[i][0], keys[i][1]);
        glVertex2f(keys[i][0]+keys[i][2], keys[i][1]);
        glVertex2f(keys[i][0]+keys[i][2], keys[i][1]+keys[i][3]);
        glVertex2f(keys[i][0], keys[i][1]+keys[i][3]);
        glEnd();
        glColor3f(0.6f, 0.6f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(keys[i][0]+2, keys[i][1]+2);
        glVertex2f(keys[i][0]+keys[i][2]-2, keys[i][1]+2);
        glVertex2f(keys[i][0]+keys[i][2]-2, keys[i][1]+keys[i][3]-4);
        glVertex2f(keys[i][0]+2, keys[i][1]+keys[i][3]-4);
        glEnd();
    }

    /* Fare ipucu */
    glColor3f(0.3f, 0.5f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(145, SCREEN_H - 80);
    glVertex2f(205, SCREEN_H - 80);
    glVertex2f(205, SCREEN_H - 15);
    glVertex2f(145, SCREEN_H - 15);
    glEnd();

    /* F tuşu - ateş */
    glColor3f(0.8f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W-60, SCREEN_H-45);
    glVertex2f(SCREEN_W-10, SCREEN_H-45);
    glVertex2f(SCREEN_W-10, SCREEN_H-10);
    glVertex2f(SCREEN_W-60, SCREEN_H-10);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

/* ===== MENÜ ÇİZ ===== */
static void draw_menu_3d(int frame) {
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SCREEN_W, SCREEN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Arkaplan gradyanı */
    glBegin(GL_QUADS);
    glColor3f(0.05f, 0.05f, 0.20f); glVertex2f(0, 0);
    glColor3f(0.05f, 0.05f, 0.20f); glVertex2f(SCREEN_W, 0);
    glColor3f(0.10f, 0.05f, 0.30f); glVertex2f(SCREEN_W, SCREEN_H);
    glColor3f(0.10f, 0.05f, 0.30f); glVertex2f(0, SCREEN_H);
    glEnd();

    /* Animasyonlu Roblox-tarzı bloklar */
    float block_cols[][3] = {
        {0.9f,0.2f,0.2f},{0.2f,0.6f,0.9f},{0.9f,0.75f,0.1f},
        {0.2f,0.8f,0.3f},{0.75f,0.2f,0.9f},{0.9f,0.5f,0.1f}
    };
    for (int i = 0; i < 6; i++) {
        float bx = 80.f + i * 145.f;
        float by = 380.f + (float)sin(frame * 0.04 + i) * 25.f;
        float *bc = block_cols[i];
        float rot = (float)(frame * 0.8 + i * 60);
        /* Basit dönen kare simüle et */
        float bs = 55.f + (float)sin(frame * 0.05 + i) * 8.f;
        glColor3f(bc[0]*0.7f, bc[1]*0.7f, bc[2]*0.7f);
        glBegin(GL_QUADS);
        glVertex2f(bx, by); glVertex2f(bx+bs, by);
        glVertex2f(bx+bs, by+bs); glVertex2f(bx, by+bs);
        glEnd();
        glColor3f(bc[0], bc[1], bc[2]);
        float pad = bs * 0.1f;
        glBegin(GL_QUADS);
        glVertex2f(bx+pad, by+pad);
        glVertex2f(bx+bs-pad, by+pad);
        glVertex2f(bx+bs-pad, by+bs-pad*2);
        glVertex2f(bx+pad, by+bs-pad*2);
        glEnd();
        /* Üst yüz vurgu */
        glColor3f(bc[0]*1.3f>1?1:bc[0]*1.3f,
                  bc[1]*1.3f>1?1:bc[1]*1.3f,
                  bc[2]*1.3f>1?1:bc[2]*1.3f);
        glBegin(GL_QUADS);
        glVertex2f(bx+pad, by+pad);
        glVertex2f(bx+bs-pad, by+pad);
        glVertex2f(bx+bs-pad, by+pad+8);
        glVertex2f(bx+pad, by+pad+8);
        glEnd();
        (void)rot;
    }

    /* Başlık kutusu */
    float ty = 80.f + (float)sin(frame * 0.04) * 8.f;
    glColor3f(0.85f, 0.15f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-240.f, ty);
    glVertex2f(SCREEN_W/2+240.f, ty);
    glVertex2f(SCREEN_W/2+240.f, ty+90.f);
    glVertex2f(SCREEN_W/2-240.f, ty+90.f);
    glEnd();
    glColor3f(1.0f, 0.35f, 0.35f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-230.f, ty+8.f);
    glVertex2f(SCREEN_W/2+230.f, ty+8.f);
    glVertex2f(SCREEN_W/2+230.f, ty+72.f);
    glVertex2f(SCREEN_W/2-230.f, ty+72.f);
    glEnd();
    /* "ROBLOX TARZLI OYUN" metin çubuğu */
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-200.f, ty+20.f);
    glVertex2f(SCREEN_W/2+200.f, ty+20.f);
    glVertex2f(SCREEN_W/2+200.f, ty+35.f);
    glVertex2f(SCREEN_W/2-200.f, ty+35.f);
    glEnd();
    glColor3f(1.0f, 0.9f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-150.f, ty+45.f);
    glVertex2f(SCREEN_W/2+150.f, ty+45.f);
    glVertex2f(SCREEN_W/2+150.f, ty+58.f);
    glVertex2f(SCREEN_W/2-150.f, ty+58.f);
    glEnd();

    /* Alt bilgi kutusu */
    glColor3f(0.15f, 0.15f, 0.35f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-220.f, 490.f);
    glVertex2f(SCREEN_W/2+220.f, 490.f);
    glVertex2f(SCREEN_W/2+220.f, 590.f);
    glVertex2f(SCREEN_W/2-220.f, 590.f);
    glEnd();

    /* BAŞLA butonu */
    float pulse = (float)sin(frame * 0.12) * 12.f;
    glColor3f(0.1f, 0.75f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-130.f-pulse, 290.f);
    glVertex2f(SCREEN_W/2+130.f+pulse, 290.f);
    glVertex2f(SCREEN_W/2+130.f+pulse, 355.f);
    glVertex2f(SCREEN_W/2-130.f-pulse, 355.f);
    glEnd();
    glColor3f(0.15f, 0.90f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-118.f-pulse, 302.f);
    glVertex2f(SCREEN_W/2+118.f+pulse, 302.f);
    glVertex2f(SCREEN_W/2+118.f+pulse, 343.f);
    glVertex2f(SCREEN_W/2-118.f-pulse, 343.f);
    glEnd();
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-80.f, 318.f);
    glVertex2f(SCREEN_W/2+80.f, 318.f);
    glVertex2f(SCREEN_W/2+80.f, 328.f);
    glVertex2f(SCREEN_W/2-80.f, 328.f);
    glEnd();

    /* Kontrol açıklaması çubukları */
    float desc_y = 505.f;
    float desc_colors[][3] = {{0.6f,0.6f,0.9f},{0.9f,0.9f,0.3f},{0.9f,0.4f,0.4f}};
    for (int i = 0; i < 3; i++) {
        glColor3f(desc_colors[i][0], desc_colors[i][1], desc_colors[i][2]);
        glBegin(GL_QUADS);
        glVertex2f(SCREEN_W/2-200.f, desc_y + i*25.f);
        glVertex2f(SCREEN_W/2+200.f, desc_y + i*25.f);
        glVertex2f(SCREEN_W/2+200.f, desc_y + i*25.f + 16.f);
        glVertex2f(SCREEN_W/2-200.f, desc_y + i*25.f + 16.f);
        glEnd();
    }

    glEnable(GL_DEPTH_TEST);
}

/* ===== OYUN BİTTİ ===== */
static void draw_gameover_2d(Player *p) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, SCREEN_W, SCREEN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.5f, 0.0f, 0.0f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(SCREEN_W, 0);
    glVertex2f(SCREEN_W, SCREEN_H); glVertex2f(0, SCREEN_H);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(0.8f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-220.f, SCREEN_H/2-70.f);
    glVertex2f(SCREEN_W/2+220.f, SCREEN_H/2-70.f);
    glVertex2f(SCREEN_W/2+220.f, SCREEN_H/2+80.f);
    glVertex2f(SCREEN_W/2-220.f, SCREEN_H/2+80.f);
    glEnd();
    glColor3f(1.f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-200.f, SCREEN_H/2-55.f);
    glVertex2f(SCREEN_W/2+200.f, SCREEN_H/2-55.f);
    glVertex2f(SCREEN_W/2+200.f, SCREEN_H/2-20.f);
    glVertex2f(SCREEN_W/2-200.f, SCREEN_H/2-20.f);
    glEnd();

    int sw = p->score; if (sw > 350) sw = 350;
    glColor3f(1.f, 0.85f, 0.f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-175.f, SCREEN_H/2+5.f);
    glVertex2f(SCREEN_W/2-175.f+sw, SCREEN_H/2+5.f);
    glVertex2f(SCREEN_W/2-175.f+sw, SCREEN_H/2+25.f);
    glVertex2f(SCREEN_W/2-175.f, SCREEN_H/2+25.f);
    glEnd();

    glColor3f(0.7f, 0.3f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-200.f, SCREEN_H/2+40.f);
    glVertex2f(SCREEN_W/2+200.f, SCREEN_H/2+40.f);
    glVertex2f(SCREEN_W/2+200.f, SCREEN_H/2+60.f);
    glVertex2f(SCREEN_W/2-200.f, SCREEN_H/2+60.f);
    glEnd();

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

/* ===== KAZANDIN ===== */
static void draw_win_2d(Player *p, int frame) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, SCREEN_W, SCREEN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.4f, 0.0f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f(SCREEN_W,0);
    glVertex2f(SCREEN_W,SCREEN_H); glVertex2f(0,SCREEN_H);
    glEnd();
    glDisable(GL_BLEND);

    /* Kutlama blokları */
    srand(frame / 5);
    for (int i = 0; i < 12; i++) {
        float bx = (float)(rand() % SCREEN_W);
        float by = (float)(rand() % SCREEN_H);
        float r = (float)(rand()%100)/100.f;
        float g = (float)(rand()%100)/100.f;
        float b2 = (float)(rand()%100)/100.f;
        glColor3f(r, g, b2);
        glBegin(GL_QUADS);
        glVertex2f(bx, by); glVertex2f(bx+25, by);
        glVertex2f(bx+25, by+25); glVertex2f(bx, by+25);
        glEnd();
    }

    glColor3f(0.1f, 0.7f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-220.f, SCREEN_H/2-70.f);
    glVertex2f(SCREEN_W/2+220.f, SCREEN_H/2-70.f);
    glVertex2f(SCREEN_W/2+220.f, SCREEN_H/2+80.f);
    glVertex2f(SCREEN_W/2-220.f, SCREEN_H/2+80.f);
    glEnd();
    glColor3f(0.3f, 1.f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-200.f, SCREEN_H/2-55.f);
    glVertex2f(SCREEN_W/2+200.f, SCREEN_H/2-55.f);
    glVertex2f(SCREEN_W/2+200.f, SCREEN_H/2-20.f);
    glVertex2f(SCREEN_W/2-200.f, SCREEN_H/2-20.f);
    glEnd();

    int sw = p->score; if (sw > 350) sw = 350;
    glColor3f(1.f, 0.85f, 0.f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-175.f, SCREEN_H/2+5.f);
    glVertex2f(SCREEN_W/2-175.f+sw, SCREEN_H/2+5.f);
    glVertex2f(SCREEN_W/2-175.f+sw, SCREEN_H/2+30.f);
    glVertex2f(SCREEN_W/2-175.f, SCREEN_H/2+30.f);
    glEnd();

    glColor3f(1.f, 1.f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(SCREEN_W/2-200.f, SCREEN_H/2+45.f);
    glVertex2f(SCREEN_W/2+200.f, SCREEN_H/2+45.f);
    glVertex2f(SCREEN_W/2+200.f, SCREEN_H/2+65.f);
    glVertex2f(SCREEN_W/2-200.f, SCREEN_H/2+65.f);
    glEnd();

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

/* ===== HAREKET ===== */
static void move_player(Player *p, const Uint8 *keys, double dt) {
    double speed = 4.5 * dt;
    double turn  = 2.2 * dt;

    if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_Q]) p->yaw -= turn;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_E]) p->yaw += turn;

    double dx = 0, dz = 0;
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
        dx += sin(p->yaw) * speed;
        dz -= cos(p->yaw) * speed;
    }
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
        dx -= sin(p->yaw) * speed;
        dz += cos(p->yaw) * speed;
    }
    if (keys[SDL_SCANCODE_A]) {
        dx -= cos(p->yaw) * speed;
        dz -= sin(p->yaw) * speed;
    }
    if (keys[SDL_SCANCODE_D]) {
        dx += cos(p->yaw) * speed;
        dz += sin(p->yaw) * speed;
    }

    double r = 0.3;
    double nx = p->x + dx, nz = p->z + dz;
    if (!map_solid((int)(nx + r), (int)p->z) &&
        !map_solid((int)(nx - r), (int)p->z))
        p->x = nx;
    if (!map_solid((int)p->x, (int)(nz + r)) &&
        !map_solid((int)p->x, (int)(nz - r)))
        p->z = nz;
}

/* ===== DÜŞMAN GÜNCELLE ===== */
static void update_enemies(Enemy *enemies, Player *p, double dt) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        double dx = p->x - enemies[i].x;
        double dz = p->z - enemies[i].z;
        double dist = sqrt(dx*dx + dz*dz);
        if (dist < 0.1) continue;
        double nx = enemies[i].x + (dx/dist) * enemies[i].speed * dt * 60.0;
        double nz = enemies[i].z + (dz/dist) * enemies[i].speed * dt * 60.0;
        if (!map_solid((int)nx, (int)enemies[i].z)) enemies[i].x = nx;
        if (!map_solid((int)enemies[i].x, (int)nz)) enemies[i].z = nz;
        if (dist < 0.6) p->health -= (int)(dt * 15.0);
        if (p->health < 0) p->health = 0;
    }
}

/* ===== COİN KONTROL ===== */
static void check_coins(Player *p, Coin *coins) {
    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].collected) continue;
        double dx = coins[i].x + 0.5 - p->x;
        double dz = coins[i].z + 0.5 - p->z;
        if (dx*dx + dz*dz < 0.35*0.35) {
            coins[i].collected = 1;
            p->score += 10;
        }
    }
}

/* ===== ATEŞ ET ===== */
static void shoot(Player *p, Enemy *enemies) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        double dx = enemies[i].x - p->x;
        double dz = enemies[i].z - p->z;
        double dist = sqrt(dx*dx + dz*dz);
        if (dist > 5.0) continue;
        double angle = atan2(dx, -dz) - p->yaw;
        while (angle >  PI) angle -= 2*PI;
        while (angle < -PI) angle += 2*PI;
        if (fabs(angle) < 0.35) {
            enemies[i].health--;
            if (enemies[i].health <= 0) {
                enemies[i].alive = 0;
                p->score += 50;
            }
        }
    }
}

/* ===== KAMERA AYARLA ===== */
static void setup_camera(Player *p) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75.0, (double)SCREEN_W / SCREEN_H, 0.05, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double eyeX = p->x;
    double eyeY = p->y + 1.7;
    double eyeZ = p->z;

    double lookX = eyeX + sin(p->yaw) * cos(p->pitch);
    double lookY = eyeY + sin(p->pitch);
    double lookZ = eyeZ - cos(p->yaw) * cos(p->pitch);

    gluLookAt(eyeX, eyeY, eyeZ,
              lookX, lookY, lookZ,
              0, 1, 0);
}

/* ===== IŞIKLANDIRMA ===== */
static void setup_lighting(void) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat ambient[]  = {0.6f,  0.6f,  0.65f, 1.0f};
    GLfloat diffuse[]  = {1.0f,  0.95f, 0.85f, 1.0f};
    GLfloat pos0[]     = {10.f, 15.f, 10.f, 1.f};
    GLfloat pos1[]     = {10.f,  8.f, 10.f, 1.f};
    GLfloat diff1[]    = {0.4f,  0.5f, 0.7f, 1.f};

    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  diff1);
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);
}

/* ===== RESET ===== */
static void reset_game(Player *p, Coin *coins, Enemy *enemies,
                        int coin_pos[][2], int epos[][2]) {
    p->x = 2.5; p->y = 0.0; p->z = 2.5;
    p->yaw = 0; p->pitch = 0;
    p->health = 100; p->score = 0;
    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].x = coin_pos[i][0];
        coins[i].z = coin_pos[i][1];
        coins[i].y = 0.6;
        coins[i].collected = 0;
        coins[i].spin = 0;
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x = epos[i][0] + 0.5;
        enemies[i].z = epos[i][1] + 0.5;
        enemies[i].y = 0;
        enemies[i].alive = 1;
        enemies[i].health = 3;
        enemies[i].speed = 0.015 + i * 0.005;
        enemies[i].spin = 0;
    }
}

/* ===== ANA FONKSİYON ===== */
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL init hatasi: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *win = SDL_CreateWindow(
        "Roblox Tarzli 3D Oyun - OpenGL",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!win) {
        fprintf(stderr, "Pencere hatasi: %s\n", SDL_GetError());
        SDL_Quit(); return 1;
    }

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) {
        fprintf(stderr, "OpenGL context hatasi: %s\n", SDL_GetError());
        SDL_DestroyWindow(win); SDL_Quit(); return 1;
    }

    SDL_GL_SetSwapInterval(1); /* VSync */

    /* OpenGL başlangıç ayarları */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);   /* Winding order sorununu önler */
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);

    /* Oyun verileri */
    Player player = {.x=2.5, .y=0, .z=2.5, .yaw=0, .pitch=0, .health=100, .score=0};

    int coin_pos[MAX_COINS][2] = {
        {3,3},{5,5},{7,2},{9,9},{11,5},{13,3},{15,7},{17,2},{3,10},
        {6,12},{8,8},{10,14},{12,10},{14,6},{16,12},{4,16},{7,15},
        {10,17},{13,15},{16,16},{2,7},{18,7},{5,13},{15,13},{9,3}
    };
    int epos[MAX_ENEMIES][2] = {{10,10},{5,15},{15,5},{12,3},{3,12},{17,15}};

    Coin coins[MAX_COINS];
    Enemy enemies[MAX_ENEMIES];
    reset_game(&player, coins, enemies, coin_pos, epos);

    GameState state = STATE_MENU;
    int running = 1, frame = 0, mouse_cap = 0;
    Uint32 last_time = SDL_GetTicks();

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
                SDL_Keycode k = ev.key.keysym.sym;
                if (k == SDLK_ESCAPE) {
                    if (state == STATE_PLAYING) {
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                        mouse_cap = 0;
                        state = STATE_MENU;
                    } else running = 0;
                }
                if ((k == SDLK_RETURN || k == SDLK_SPACE) &&
                    (state == STATE_MENU)) {
                    reset_game(&player, coins, enemies, coin_pos, epos);
                    state = STATE_PLAYING;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    mouse_cap = 1;
                }
                if ((k == SDLK_RETURN || k == SDLK_SPACE) &&
                    (state == STATE_GAMEOVER || state == STATE_WIN))
                    state = STATE_MENU;
                if (k == SDLK_f && state == STATE_PLAYING)
                    shoot(&player, enemies);
            }

            if (ev.type == SDL_MOUSEBUTTONDOWN && state == STATE_PLAYING) {
                if (!mouse_cap) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    mouse_cap = 1;
                } else if (ev.button.button == SDL_BUTTON_LEFT) {
                    shoot(&player, enemies);
                }
            }

            if (ev.type == SDL_MOUSEMOTION && mouse_cap && state == STATE_PLAYING) {
                player.yaw   += ev.motion.xrel * 0.002;
                player.pitch -= ev.motion.yrel * 0.002;
                if (player.pitch >  1.2) player.pitch =  1.2;
                if (player.pitch < -1.2) player.pitch = -1.2;
            }
        }

        /* Güncelle */
        if (state == STATE_PLAYING) {
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            move_player(&player, keys, dt);
            update_enemies(enemies, &player, dt);
            check_coins(&player, coins);

            if (player.health <= 0) { state = STATE_GAMEOVER; SDL_SetRelativeMouseMode(SDL_FALSE); mouse_cap = 0; }
            int left = 0;
            for (int i = 0; i < MAX_COINS; i++) if (!coins[i].collected) left++;
            if (left == 0) { state = STATE_WIN; SDL_SetRelativeMouseMode(SDL_FALSE); mouse_cap = 0; }
        }

        /* Çiz */
        if (state == STATE_MENU) {
            draw_menu_3d(frame);
        } else if (state == STATE_PLAYING) {
            glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            /* Gökyüzü */
            draw_sky();

            setup_camera(&player);
            setup_lighting();

            /* 3D dünya */
            draw_floor();
            draw_map();
            draw_coins(coins);
            draw_enemies(enemies);

            /* HUD */
            int left = 0;
            for (int i = 0; i < MAX_COINS; i++) if (!coins[i].collected) left++;
            glDisable(GL_LIGHTING);
            draw_hud_2d(&player, left, MAX_COINS);
        } else if (state == STATE_GAMEOVER) {
            glClearColor(0.1f, 0.0f, 0.0f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            draw_gameover_2d(&player);
        } else if (state == STATE_WIN) {
            glClearColor(0.0f, 0.1f, 0.0f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            draw_win_2d(&player, frame);
        }

        SDL_GL_SwapWindow(win);

        Uint32 elapsed = SDL_GetTicks() - now;
        if (elapsed < FRAME_TIME) SDL_Delay(FRAME_TIME - elapsed);
    }

    if (mouse_cap) SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
