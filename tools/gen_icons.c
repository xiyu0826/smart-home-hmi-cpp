/*
 * gen_icons.c — 生成 24位 BMP 设备图标（48×48 像素）
 * 编译: gcc gen_icons.c -o gen_icons -lm
 * 运行: ./gen_icons
 * 输出: images/ 目录下 28 个 BMP 文件
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define W  48
#define H  48

typedef struct { uint8_t b, g, r; } Color;

static uint8_t img[H][W][3]; /* BGR */

/* ====== BMP 写入 ====== */
static void save_bmp(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); return; }
    int row_size = ((W * 3 + 3) / 4) * 4;
    int img_size = row_size * H;
    uint8_t hdr[54] = {0};
    hdr[0]='B'; hdr[1]='M';
    *(uint32_t*)(hdr+2)  = 54 + img_size;
    *(uint32_t*)(hdr+10) = 54;
    *(uint32_t*)(hdr+14) = 40;
    *(uint32_t*)(hdr+18) = W;
    *(uint32_t*)(hdr+22) = H;
    *(uint16_t*)(hdr+26) = 1;
    *(uint16_t*)(hdr+28) = 24;
    *(uint32_t*)(hdr+30) = 0;               /* biCompression = BI_RGB */
    *(uint32_t*)(hdr+34) = img_size;        /* biSizeImage */
    *(uint32_t*)(hdr+38) = 2835;            /* biXPelsPerMeter = 72 DPI */
    *(uint32_t*)(hdr+42) = 2835;            /* biYPelsPerMeter = 72 DPI */
    fwrite(hdr, 1, 54, f);
    uint8_t *row = calloc(1, row_size);
    for (int y = H-1; y >= 0; y--) {
        memcpy(row, img[y], W*3);
        fwrite(row, 1, row_size, f);
    }
    free(row);
    fclose(f);
    printf("  %s\n", path);
}

/* ====== 绘图工具 ====== */
static void clear(Color c) {
    for (int y=0;y<H;y++) for (int x=0;x<W;x++)
        { img[y][x][0]=c.b; img[y][x][1]=c.g; img[y][x][2]=c.r; }
}
static void rect(int x1, int y1, int x2, int y2, Color c) {
    for (int y=y1;y<=y2;y++) for (int x=x1;x<=x2;x++)
        if (x>=0&&x<W&&y>=0&&y<H) { img[y][x][0]=c.b; img[y][x][1]=c.g; img[y][x][2]=c.r; }
}
static void circle(int cx, int cy, int r, Color c) {
    for (int y=cy-r; y<=cy+r; y++) for (int x=cx-r; x<=cx+r; x++)
        if (x>=0&&x<W&&y>=0&&y<H && (x-cx)*(x-cx)+(y-cy)*(y-cy) <= r*r)
            { img[y][x][0]=c.b; img[y][x][1]=c.g; img[y][x][2]=c.r; }
}
static void line_h(int y, int x1, int x2, Color c) { rect(x1,y,x2,y,c); }
static void line_v(int x, int y1, int y2, Color c) { rect(x,y1,x,y2,c); }

/* ====== 各图标绘制 ====== */
#define C(rr,gg,bb) ((Color){bb,gg,rr})
static Color BG = {0,0,0};      /* 透明黑背景 */
static Color WH = C(255,255,255);
static Color YW = C(255,200,0);
static Color BL = C(52,152,219);
static Color GR = C(46,204,113);
static Color OR = C(230,126,34);
static Color RD = C(231,76,60);
static Color PR = C(155,89,182);
static Color GY = C(149,165,166);
static Color PK = C(233,30,99);
static Color CY = C(26,188,156);
static Color AM = C(255,152,0);
static Color LG = C(139,195,74);
static Color BR = C(121,85,72);
static Color DG = C(52,73,94);
static Color SB = C(0,188,212);
static Color OG = C(255,87,34);
static Color BG2= C(96,125,139);

static void draw_bulb() {
    clear(BG);
    circle(23,21,11,C(255,240,180)); /* 灯泡 */
    circle(23,22,5,C(255,255,240));  /* 高光 */
    rect(17,31,29,37,C(180,180,180)); /* 底座 */
    rect(20,37,26,40,C(150,150,150));
    /* 光线 */
    line_h(3,23,23,YW); line_h(43,23,43,YW);
    line_v(23,3,23,YW); line_v(23,43,43,YW);
    line_h(14,34,14,C(255,220,100)); line_v(34,14,14,C(255,220,100));
    line_h(14,12,14,C(255,220,100)); line_v(34,34,34,C(255,220,100));
}

static void draw_ac() {
    clear(BG);
    circle(24,22,16,BL);
    /* 雪花 */
    line_h(22,13,13,WH); line_h(22,35,35,WH);
    line_v(13,22,22,WH); line_v(35,22,22,WH);
    rect(18,18,22,22,C(100,180,240));
    circle(24,22,8,C(80,170,240));
    rect(20,20,28,28,C(60,160,230));
    circle(24,22,4,WH);
}

static void draw_curtain() {
    clear(BG);
    rect(6,4,42,44,PR);
    for (int i=0;i<3;i++) { line_v(12+i*12,4,44,C(130,70,160)); }
    rect(0,4,48,8,C(120,60,150));
    circle(8,22,3,C(180,130,200)); circle(40,22,3,C(180,130,200));
    circle(8,36,3,C(180,130,200)); circle(40,36,3,C(180,130,200));
}

static void draw_strip() {
    clear(BG);
    rect(4,8,44,14,C(255,100,100));
    rect(4,17,44,23,C(100,255,100));
    rect(4,26,44,32,C(100,100,255));
    rect(4,35,44,41,C(255,255,100));
    circle(24,6,3,WH); circle(24,44,3,WH);
}

static void draw_tv() {
    clear(BG);
    rect(4,10,40,36,DG);
    rect(8,14,36,32,C(30,50,70));
    rect(16,2,28,10,C(80,80,80));
    circle(22,20,6,C(80,200,255));
    rect(12,30,18,36,C(100,100,100));
    rect(26,30,32,36,C(100,100,100));
}

static void draw_robot() {
    clear(BG);
    circle(24,20,14,GY);
    rect(14,22,34,40,GY);
    circle(17,14,4,GY); circle(31,14,4,GY);
    circle(17,14,2,C(50,200,255)); circle(31,14,2,C(50,200,255));
    rect(20,34,28,38,C(200,50,50));
    line_v(24,40,44,GY);
    circle(24,44,4,GY);
}

static void draw_purifier() {
    clear(BG);
    circle(24,22,16,C(200,240,255));
    circle(24,22,10,CY);
    for (int i=0;i<3;i++) {
        int a = i*20; double ra = a*3.14159/180.0;
        int ex=24+14*cos(ra), ey=22+14*sin(ra);
        line_h(ey,ex-3,ex+3,WH);
    }
    rect(18,36,30,42,C(100,100,100));
}

static void draw_speaker() {
    clear(BG);
    rect(8,14,22,34,DG);
    rect(8,18,18,30,C(30,80,30));
    circle(24,24,14,DG);
    circle(24,24,10,C(40,60,40));
    circle(24,24,5,C(20,40,20));
    rect(26,14,34,18,DG);
    rect(26,30,34,34,DG);
}

static void draw_humidifier() {
    clear(BG);
    circle(24,20,12,SB);
    circle(24,20,7,C(150,230,255));
    rect(14,30,34,42,C(200,200,200));
    circle(18,26,3,C(100,200,240));
    circle(30,26,3,C(100,200,240));
    rect(20,34,28,42,C(180,180,180));
}

static void draw_closet() {
    clear(BG);
    rect(6,4,42,44,AM);
    rect(8,6,22,42,C(255,180,50));
    rect(26,6,40,42,C(255,180,50));
    circle(12,26,3,C(200,120,0));
    circle(36,26,3,C(200,120,0));
    line_h(6,7,41,C(220,140,20));
}

static void draw_aroma() {
    clear(BG);
    circle(24,20,10,LG);
    circle(24,20,5,C(180,240,150));
    rect(18,28,30,42,C(120,100,80));
    rect(20,32,28,38,C(160,140,120));
    /* 花瓣 */
    for (int i=0;i<5;i++) {
        double a = i*72*3.14159/180.0;
        int px=24+12*cos(a), py=12+12*sin(a);
        circle(px,py,7,PK);
    }
    circle(24,11,4,WH);
}

static void draw_kitchen_light() {
    clear(BG);
    /* 吊灯形状 */
    line_v(24,2,10,C(100,100,100));
    rect(10,10,38,14,C(200,200,200));
    rect(8,14,40,22,C(255,240,180));
    rect(8,22,40,28,C(255,255,200));
    rect(14,14,34,20,C(255,255,240));
}

static void draw_outlet() {
    clear(BG);
    rect(8,8,40,40,C(220,220,220));
    rect(12,12,36,36,WH);
    rect(18,14,30,20,C(50,50,50));
    rect(18,28,30,34,C(50,50,50));
    circle(24,17,2,C(100,200,100));
    circle(24,31,2,C(255,100,100));
}

static void draw_fan() {
    clear(BG);
    circle(24,24,18,C(200,200,200));
    circle(24,24,6,GY);
    for (int i=0;i<3;i++) {
        double a = i*120*3.14159/180.0;
        for (int r=8;r<=18;r++) {
            int x=24+r*cos(a), y=24+r*sin(a);
            if (x>=6&&x<W&&y>=6&&y<H)
                img[y][x][0]=GY.b; img[y][x][1]=GY.g; img[y][x][2]=GY.r;
        }
    }
}

static void draw_fridge() {
    clear(BG);
    rect(8,4,40,44,C(200,210,220));
    rect(10,6,38,20,WH);
    rect(10,24,38,42,WH);
    rect(10,22,38,26,C(180,190,200));
    rect(14,28,22,34,C(200,220,240));
    rect(14,38,26,40,C(200,220,240));
}

static void draw_microwave() {
    clear(BG);
    rect(6,10,42,42,DG);
    rect(10,14,38,38,C(50,60,70));
    circle(28,30,12,C(60,70,80));
    circle(28,30,5,C(80,200,255));
    rect(12,26,24,34,C(80,200,80));
    rect(36,14,42,18,C(100,100,100));
    rect(36,20,42,22,C(100,100,100));
}

static void draw_dishwasher() {
    clear(BG);
    rect(8,6,40,44,WH);
    rect(10,8,38,42,C(230,240,250));
    rect(10,8,38,22,C(200,220,240));
    rect(16,26,32,36,C(220,230,240));
    rect(20,30,28,34,C(180,200,220));
    circle(36,16,3,C(100,180,100));
    circle(12,16,3,C(255,180,100));
    circle(24,16,3,C(100,100,255));
}

static void draw_oven() {
    clear(BG);
    rect(6,6,42,44,DG);
    rect(10,10,38,40,C(60,60,65));
    rect(14,14,34,26,C(80,40,20));
    rect(14,30,34,42,C(70,70,75));
    line_h(28,14,34,C(100,100,100));
    rect(28,32,34,36,C(255,100,50));
    circle(36,12,3,C(255,60,30));
}

static void draw_water() {
    clear(BG);
    /* 水龙头 */
    rect(8,18,40,26,C(180,190,200));
    circle(8,22,5,C(100,180,240));
    rect(8,4,16,22,C(100,120,140));
    circle(8,8,4,C(80,180,220));
    /* 水滴 */
    circle(30,34,6,C(52,152,219));
    circle(32,36,3,C(100,200,255));
}

static void draw_heater() {
    clear(BG);
    /* 火焰 */
    for (int y=8;y<42;y++) {
        int w = (42-y)*5/18;
        if (w<3) w=3;
        rect(24-w,y,24+w,y,RD);
    }
    rect(12,28,36,40,C(255,120,0));
    rect(16,34,32,38,YW);
    /* 容器 */
    rect(10,42,38,46,C(150,150,160));
    rect(14,38,34,42,C(180,180,190));
}

static void draw_bath_heater() {
    clear(BG);
    circle(24,18,14,OG);
    circle(24,18,7,C(255,200,100));
    for (int i=0;i<8;i++) {
        double a = i*45*3.14159/180.0;
        int ex=24+20*cos(a), ey=18+20*sin(a);
        rect(ex-1,ey-1,ex+1,ey+1,YW);
    }
    rect(14,30,34,44,C(180,180,180));
}

static void draw_mirror() {
    clear(BG);
    rect(6,6,42,42,C(200,220,240));
    rect(10,10,38,38,C(180,240,255));
    circle(32,14,6,C(255,255,200));
    /* 高光 */
    rect(14,14,24,18,C(220,250,255));
}

static void draw_toilet() {
    clear(BG);
    rect(12,4,36,14,WH);
    rect(14,10,34,36,WH);
    rect(16,14,32,22,C(230,240,250));
    circle(24,18,4,C(100,200,240));
    rect(12,32,18,44,WH);
    rect(30,32,36,44,WH);
}

static void draw_snowflake() {
    clear(BG);
    circle(24,24,18,C(200,230,255));
    line_v(24,2,46,BL); line_h(24,2,46,BL);
    double ang[6]={0,60,120,180,240,300};
    for (int i=0;i<6;i++) {
        double a=ang[i]*3.14159/180.0;
        int x=24+20*cos(a), y=24+20*sin(a);
        line_h(y-2,x-7,x+7,BL);
    }
    circle(24,24,6,BL);
    circle(24,24,3,WH);
}

static void draw_fire() {
    clear(BG);
    for (int y=6;y<44;y++) {
        int w = (44-y)*6/20;
        if (w<2) w=2;
        rect(24-w,y,24+w,y,RD);
    }
    rect(10,30,38,42,C(255,100,0));
    rect(14,36,34,42,YW);
}

#define MAKE_ICON(name, fn) \
    do { char _path[64]; snprintf(_path, sizeof(_path), "%s/%s.bmp", ".", #name); \
         printf("Generating %s...\n", _path); fn(); save_bmp(_path); } while(0)

int main(void) {
    MAKE_ICON(light,          draw_bulb);
    MAKE_ICON(ac,             draw_ac);
    MAKE_ICON(curtain,        draw_curtain);
    MAKE_ICON(strip,          draw_strip);
    MAKE_ICON(tv,             draw_tv);
    MAKE_ICON(robot,          draw_robot);
    MAKE_ICON(purifier,       draw_purifier);
    MAKE_ICON(speaker,        draw_speaker);
    MAKE_ICON(humidifier,     draw_humidifier);
    MAKE_ICON(closet,         draw_closet);
    MAKE_ICON(aroma,          draw_aroma);
    MAKE_ICON(kitchen_light,  draw_kitchen_light);
    MAKE_ICON(outlet,         draw_outlet);
    MAKE_ICON(fan,            draw_fan);
    MAKE_ICON(fridge,         draw_fridge);
    MAKE_ICON(microwave,      draw_microwave);
    MAKE_ICON(dishwasher,     draw_dishwasher);
    MAKE_ICON(oven,           draw_oven);
    MAKE_ICON(water,          draw_water);
    MAKE_ICON(heater,         draw_heater);
    MAKE_ICON(bath_heater,    draw_bath_heater);
    MAKE_ICON(mirror,         draw_mirror);
    MAKE_ICON(toilet,         draw_toilet);
    MAKE_ICON(bath_light,     draw_bulb);
    MAKE_ICON(exhaust,        draw_fan);
    MAKE_ICON(bulb,           draw_bulb);
    MAKE_ICON(snowflake,      draw_snowflake);
    MAKE_ICON(fire,           draw_fire);

    printf("\nDone! %d icons generated.\n", 28);
    return 0;
}
