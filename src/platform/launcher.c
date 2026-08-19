/*
 * PokeRecomp — Brigette (faithful C port of launcher.cpp)
 * Uses OpenGL immediate mode + stb_image + stb_truetype + SDL2_mixer.
 * Assets loaded from "launcher/" directory.
 */
#ifdef PLATFORM_SDL2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#include "../../lib/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../lib/stb_truetype.h"

#include "platform/pkmn_archive.h"
#include "platform/box_transfer.h"

/* ── Window ── */
#define LW 800
#define LH 480
#define NUM_GAMES 12
#define NUM_FACES 6
#define NUM_GENS 3
#define MAX_DIALOG 16
#define MAX_TXT 256
#define TEXT_SPEED_MS 35
#define BLINK_MIN 2000
#define BLINK_MAX 5000
#define BLINK_DUR 180

/* ── Face indices ── */
#define FACE_NEUTRAL 0
#define FACE_LOOK    1
#define FACE_BLINK   2
#define FACE_SQUINT  3
#define FACE_TALK    4
#define FACE_HAPPY   5

/* ── States ── */
typedef enum {
    ST_TITLE, ST_FADE_OUT, ST_INTRO_TEXT, ST_SELECT,
    ST_MAIN_MENU, ST_GEN_SELECT, ST_MENU, ST_MENU_FADE, ST_FAREWELL,
    ST_NEED_PKMN, ST_NEED_LEAFGREEN, ST_NEED_RUBY, ST_NEED_SAPPHIRE,
    ST_NEED_GOLD, ST_NEED_SILVER, ST_NEED_CRYSTAL,
    ST_EXTRACTING, ST_LAUNCH
} LState;

/* ── Game defs ── */
typedef struct { const char *id, *name, *region; int gen; int playable; } GameDef;
static const GameDef GAMES[NUM_GAMES] = {
    /* Gen 1 */
    {"red","Red","kanto",1,0}, {"blue","Blue","kanto",1,0},
    {"yellow","Yellow","kanto",1,0}, {"green","Green","kanto",1,0},
    /* Gen 2 */
    {"gold","Gold","johto",2,1}, {"silver","Silver","johto",2,1},
    {"crystal","Crystal","johto",2,1},
    /* Gen 3 */
    {"firered","FireRed","kanto",3,1}, {"leafgreen","LeafGreen","kanto",3,1},
    {"ruby","Ruby","hoenn",3,1}, {"sapphire","Sapphire","hoenn",3,1},
    {"emerald","Emerald","hoenn",3,0},
};

static const char *GEN_NAMES[NUM_GENS] = {"Generation I", "Generation II", "Generation III"};
static const int GEN_START[NUM_GENS] = {0, 4, 7};
static const int GEN_COUNT[NUM_GENS] = {4, 3, 5};
static int curGen = 2; /* default to Gen 3 */
static int genMenuIdx = 0;

typedef struct { char text[MAX_TXT]; int face; } DialogEntry;

/* ── Asset path helper ── */
static const char *sDir = "launcher";
static char sPathBuf[512];
static const char *LA(const char *name) {
    snprintf(sPathBuf, sizeof(sPathBuf), "%s/%s", sDir, name);
    return sPathBuf;
}

/* ── GL helpers ── */
static GLuint loadTex(const char *path, int *ow, int *oh) {
    int w, h, ch;
    unsigned char *d = stbi_load(path, &w, &h, &ch, 4);
    if (!d) { *ow = 0; *oh = 0; return 0; }
    GLuint t; glGenTextures(1, &t); glBindTexture(GL_TEXTURE_2D, t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, d);
    stbi_image_free(d); *ow = w; *oh = h; return t;
}

static GLuint bakeFont(const char *path, float sz, stbtt_bakedchar *chars) {
    FILE *f = fopen(path, "rb"); if (!f) return 0;
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    unsigned char *ttf = (unsigned char *)malloc(len);
    fread(ttf, 1, len, f); fclose(f);
    unsigned char bmp[512*512];
    stbtt_BakeFontBitmap(ttf, 0, sz, bmp, 512, 512, 32, 96, chars);
    free(ttf);
    unsigned char *rgba = (unsigned char *)malloc(512*512*4);
    for (int i = 0; i < 512*512; i++) { rgba[i*4]=255; rgba[i*4+1]=255; rgba[i*4+2]=255; rgba[i*4+3]=bmp[i]; }
    GLuint t; glGenTextures(1, &t); glBindTexture(GL_TEXTURE_2D, t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    free(rgba); return t;
}

static void dQ(GLuint t, float x, float y, float w, float h, float a) {
    if (!t) return; glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, t);
    glColor4f(1,1,1,a); glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2f(x,y); glTexCoord2f(1,0); glVertex2f(x+w,y);
    glTexCoord2f(1,1); glVertex2f(x+w,y+h); glTexCoord2f(0,1); glVertex2f(x,y+h);
    glEnd(); glDisable(GL_TEXTURE_2D);
}

static void dR(float x,float y,float w,float h,float r,float g,float b,float a) {
    glColor4f(r,g,b,a); glBegin(GL_QUADS);
    glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);
    glEnd();
}

static void dGV(float x,float y,float w,float h,float r1,float g1,float b1,float a1,float r2,float g2,float b2,float a2) {
    glBegin(GL_QUADS);
    glColor4f(r1,g1,b1,a1);glVertex2f(x,y);glVertex2f(x+w,y);
    glColor4f(r2,g2,b2,a2);glVertex2f(x+w,y+h);glVertex2f(x,y+h);
    glEnd();
}

static void dBorder(float x,float y,float w,float h,float t,float r,float g,float b,float a) {
    dR(x,y,w,t,r,g,b,a); dR(x,y+h-t,w,t,r,g,b,a);
    dR(x,y,t,h,r,g,b,a); dR(x+w-t,y,t,h,r,g,b,a);
}

static void dStyleBorder(float x,float y,float w,float h,float t) {
    dGV(x,y,w,t,.65f,.67f,.72f,1,.45f,.47f,.52f,1);
    dGV(x,y+h-t,w,t,.45f,.47f,.52f,1,.35f,.37f,.42f,1);
    dGV(x,y,t,h,.65f,.67f,.72f,1,.35f,.37f,.42f,1);
    dGV(x+w-t,y,t,h,.65f,.67f,.72f,1,.35f,.37f,.42f,1);
}

static float dText(const char *s, float x, float y, GLuint ft, stbtt_bakedchar *ch, float r, float g, float b, float a) {
    if (!ft) return x; glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, ft);
    glColor4f(r,g,b,a); glBegin(GL_QUADS);
    float sx = x;
    while (*s) {
        if (*s == '\n') { y += 28; x = sx; s++; continue; }
        if (*s >= 32 && *s < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(ch, 512, 512, *s - 32, &x, &y, &q, 1);
            glTexCoord2f(q.s0,q.t0);glVertex2f(q.x0,q.y0);
            glTexCoord2f(q.s1,q.t0);glVertex2f(q.x1,q.y0);
            glTexCoord2f(q.s1,q.t1);glVertex2f(q.x1,q.y1);
            glTexCoord2f(q.s0,q.t1);glVertex2f(q.x0,q.y1);
        }
        s++;
    }
    glEnd(); glDisable(GL_TEXTURE_2D); return x;
}

static void dTextC(const char *s,float cx,float y,GLuint ft,stbtt_bakedchar *ch,float r,float g,float b) {
    float w=0; const char *p=s;
    while(*p){if(*p>=32&&*p<128) w+=ch[*p-32].xadvance; p++;}
    dText(s, cx-w/2, y, ft, ch, r, g, b, 1);
}

static int rr(int lo,int hi){return lo+rand()%(hi-lo+1);}

/* ── Launcher state ── */
static GLuint txBg, txChar[NUM_FACES], txFace[NUM_FACES], txBox[NUM_GAMES];
static int txBgW,txBgH, txChW[NUM_FACES],txChH[NUM_FACES], txFcW[NUM_FACES],txFcH[NUM_FACES], txBxW[NUM_GAMES],txBxH[NUM_GAMES];
static GLuint txFnt, txFntSm;
static stbtt_bakedchar fntCh[96], fntChSm[96];
static Mix_Music *bgMus;
static Mix_Chunk *sfxA;

static LState st;
static int frame, menuIdx, mainMenuIdx;
static float carAngle, carTarget;
static DialogEntry dlgQ[MAX_DIALOG]; static int dlgCount, dlgIdx;
static char fullTxt[MAX_TXT], shownTxt[MAX_TXT];
static int chIdx; static Uint32 txtTm; static int txtDone;
static LState afterDlg;
static int curFace, baseFace; static Uint32 nextBl; static int talkTog;
static int fadeA; static float scnA; static int boxFadeA;
static float charSlide, charSlideTarget;
static int selectedGame;
static int pkmnExists;
static int leafgreenPkmnExists;
static int rubyPkmnExists;
static int sapphirePkmnExists;
static int goldPkmnExists;
static int silverPkmnExists;
static int crystalPkmnExists;

static void setFace(int f){baseFace=f;curFace=f;}
static void playSfx(void){if(sfxA)Mix_PlayChannel(-1,sfxA,0);}

static void startCurDlg(void) {
    if(dlgIdx<dlgCount) {
        strncpy(fullTxt,dlgQ[dlgIdx].text,MAX_TXT);
        shownTxt[0]=0; chIdx=0; txtTm=SDL_GetTicks(); txtDone=0;
        setFace(dlgQ[dlgIdx].face);
    }
}
static void startDlg(DialogEntry *entries,int count,LState after) {
    dlgCount=count<MAX_DIALOG?count:MAX_DIALOG;
    memcpy(dlgQ,entries,dlgCount*sizeof(DialogEntry));
    dlgIdx=0; afterDlg=after; startCurDlg();
}
static void skipTxt(void){strncpy(shownTxt,fullTxt,MAX_TXT);chIdx=(int)strlen(fullTxt);txtDone=1;curFace=baseFace;}

static void enterSt(LState s);

static void advDlg(void){
    playSfx(); dlgIdx++;
    if(dlgIdx<dlgCount) startCurDlg(); else enterSt(afterDlg);
}

static void enterSt(LState s) {
    st=s;
    switch(s) {
    case ST_FADE_OUT: break;
    case ST_INTRO_TEXT:
        if(bgMus&&!Mix_PlayingMusic()) Mix_PlayMusic(bgMus,-1);
        {DialogEntry e[]={
            {"Hi there! Welcome!",FACE_HAPPY},
            {"I'm Brigette.\nNice to meet you!",FACE_NEUTRAL},
            {"I take care of trainers'\nPokemon storage system.",FACE_LOOK},
            {"But enough about me!\nLet's get you started.",FACE_HAPPY}};
        startDlg(e,4,ST_SELECT);}
        break;
    case ST_SELECT:
        {DialogEntry e[]={{"What would you like to do?",FACE_NEUTRAL}};
        startDlg(e,1,ST_MAIN_MENU);}
        break;
    case ST_MAIN_MENU: mainMenuIdx=0; charSlideTarget=0; break;
    case ST_GEN_SELECT: genMenuIdx=0; charSlideTarget=0; break;
    case ST_MENU: menuIdx=GEN_START[curGen]; carTarget=0; carAngle=0; boxFadeA=255; charSlideTarget=1; break;
    case ST_MENU_FADE: charSlideTarget=0; break;
    case ST_FAREWELL:
        /* Gen 1: Kanto (indices 0-3) */
        if(menuIdx==0){
            DialogEntry e[]={
                {"Kanto! The land where it\nall began.",FACE_HAPPY},
                {"Pallet Town, Viridian\nForest, Mt. Moon...",FACE_LOOK},
                {"I hear a powerful Pokemon\nsleeps deep within Cerulean\nCave...",FACE_NEUTRAL},
                {"So many legendary places\nto explore!",FACE_HAPPY},
                {"Have a great adventure!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        } else if(menuIdx==1){
            DialogEntry e[]={
                {"Kanto again! But there's\nalways more to discover.",FACE_HAPPY},
                {"The Sevii Islands are\nbeautiful this time of year.",FACE_LOOK},
                {"Maybe you'll find some\nrare Pokemon out there\non those shores!",FACE_NEUTRAL},
                {"Kanto has so many secrets\nstill waiting to be found.",FACE_HAPPY},
                {"Enjoy your journey!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        } else if(menuIdx<=3){
            DialogEntry e[]={
                {"Kanto calls! The original\nadventure awaits.",FACE_HAPPY},
                {"The S.S. Anne, Pokemon\nTower, Silph Co...",FACE_LOOK},
                {"Every corner of that region\nholds a memory.",FACE_NEUTRAL},
                {"Go make some of your own!",FACE_HAPPY},
                {"Good luck out there!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        }
        /* Gen 2: Johto (indices 4-6) */
        else if(menuIdx==4){
            DialogEntry e[]={
                {"Johto! Oh, how wonderful!",FACE_HAPPY},
                {"The Bell Tower, the Burned\nTower... so much history.",FACE_LOOK},
                {"They say a rainbow-winged\nPokemon watches over\nEcruteak City...",FACE_NEUTRAL},
                {"And the sunsets over the\nWhirl Islands? Breathtaking!",FACE_NEUTRAL},
                {"May the golden light\nguide your way!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        } else if(menuIdx==5){
            DialogEntry e[]={
                {"Johto calls to you!",FACE_HAPPY},
                {"The Ice Path, the Lake of\nRage... so many mysteries.",FACE_LOOK},
                {"I hear the sea guardian\nrests deep beneath the\nWhirl Islands...",FACE_NEUTRAL},
                {"The moonlit nights over\nOlivine are unforgettable.",FACE_NEUTRAL},
                {"May the silver wind carry\nyou safely!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        } else if(menuIdx==6){
            DialogEntry e[]={
                {"Johto awaits! And this\ntime, something new...",FACE_HAPPY},
                {"I hear the Tin Tower\nshimmers like crystal\nin the morning light.",FACE_LOOK},
                {"There are rumors of a\nPokemon racing through\nJohto's wilds...",FACE_NEUTRAL},
                {"The Battle Tower! Ruins of\nAlph! So much to discover!",FACE_NEUTRAL},
                {"May the north wind be at\nyour back!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        }
        /* Gen 3: Hoenn/Kanto (indices 7-11) */
        else if(menuIdx==7){
            DialogEntry e[]={
                {"Back to Kanto! But this\ntime, a fresh start.",FACE_HAPPY},
                {"The islands to the south,\nthe caves to explore...",FACE_LOOK},
                {"I hear there's a network\nof tunnels beneath the\nregion full of secrets!",FACE_NEUTRAL},
                {"Sevii has so many rare\nPokemon waiting for you.",FACE_HAPPY},
                {"Blaze a new trail!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        } else if(menuIdx==8){
            DialogEntry e[]={
                {"Kanto from a whole new\nangle! How exciting!",FACE_HAPPY},
                {"The forests feel different\nwhen the leaves change...",FACE_LOOK},
                {"And those islands to the\nsouth? Full of mysteries\nand rare Pokemon!",FACE_NEUTRAL},
                {"There's always something\nnew to find in Kanto.",FACE_HAPPY},
                {"Enjoy every step!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        } else if(menuIdx==9){
            DialogEntry e[]={
                {"Hoenn! Land of earth\nand fire!",FACE_HAPPY},
                {"Volcanoes, deserts, Secret\nBases hidden in the trees...",FACE_LOOK},
                {"They say the land itself\nwas shaped by a titan\nfrom deep underground.",FACE_NEUTRAL},
                {"Pokemon Contests, the\nBattle Tower... so much\nto do!",FACE_HAPPY},
                {"Go get 'em, champ!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        } else if(menuIdx==10){
            DialogEntry e[]={
                {"Hoenn! Land of sea\nand storms!",FACE_HAPPY},
                {"The diving spots, the\ncoral reefs, the rain...",FACE_LOOK},
                {"Somewhere deep below the\nwaves, an ancient guardian\nslumbers...",FACE_NEUTRAL},
                {"Just watch out for all\nthat water! Hehe.",FACE_HAPPY},
                {"Have a splash!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        } else {
            DialogEntry e[]={
                {"Hoenn in all its glory!\nThe definitive journey.",FACE_HAPPY},
                {"The Battle Frontier, the\ncontest halls, Mirage\nIsland...",FACE_LOOK},
                {"They say both ancient\ntitans stir in this\nversion of events...",FACE_NEUTRAL},
                {"So many secrets left to\nuncover!",FACE_HAPPY},
                {"Have fun out there!",FACE_HAPPY}};
            startDlg(e,5,ST_LAUNCH);
        }
        break;
    case ST_NEED_PKMN:
        {DialogEntry e[]={{"I need a Pokemon FireRed ROM\nto generate the game data.\nPress ENTER to select one!",FACE_SQUINT}};
        startDlg(e,1,ST_NEED_PKMN);}
        break;
    case ST_NEED_LEAFGREEN:
        {DialogEntry e[]={{"I need a Pokemon LeafGreen ROM\nto generate the game data.\nPress ENTER to select one!",FACE_SQUINT}};
        startDlg(e,1,ST_NEED_LEAFGREEN);}
        break;
    case ST_NEED_RUBY:
        {DialogEntry e[]={{"I need a Pokemon Ruby ROM\nto generate the game data.\nPress ENTER to select one!",FACE_SQUINT}};
        startDlg(e,1,ST_NEED_RUBY);}
        break;
    case ST_NEED_SAPPHIRE:
        {DialogEntry e[]={{"I need a Pokemon Sapphire ROM\nto generate the game data.\nPress ENTER to select one!",FACE_SQUINT}};
        startDlg(e,1,ST_NEED_SAPPHIRE);}
        break;
    case ST_NEED_GOLD:
        {DialogEntry e[]={{"I need a Pokemon Gold ROM\nto generate the game data.\nPress ENTER to select one!",FACE_SQUINT}};
        startDlg(e,1,ST_NEED_GOLD);}
        break;
    case ST_NEED_SILVER:
        {DialogEntry e[]={{"I need a Pokemon Silver ROM\nto generate the game data.\nPress ENTER to select one!",FACE_SQUINT}};
        startDlg(e,1,ST_NEED_SILVER);}
        break;
    case ST_NEED_CRYSTAL:
        {DialogEntry e[]={{"I need a Pokemon Crystal ROM\nto generate the game data.\nPress ENTER to select one!",FACE_SQUINT}};
        startDlg(e,1,ST_NEED_CRYSTAL);}
        break;
    case ST_LAUNCH:
        /* Map expanded game index to sdl2.c game codes */
        if(menuIdx==7) selectedGame=0;       /* FireRed */
        else if(menuIdx==8) selectedGame=1;  /* LeafGreen */
        else if(menuIdx==9) selectedGame=2;  /* Ruby */
        else if(menuIdx==10) selectedGame=3; /* Sapphire */
        else if(menuIdx==4) selectedGame=4;  /* Gold */
        else if(menuIdx==5) selectedGame=5;  /* Silver */
        else if(menuIdx==6) selectedGame=6;  /* Crystal */
        else selectedGame=menuIdx;
        fadeA=0; break;
    default: break;
    }
}

static void showExtractingScreen(SDL_Window *w, const char *gameName) {
    glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT);
    char buf[128];
    snprintf(buf,128,"Extracting %s data...",gameName);
    dTextC(buf,LW/2.f,LH/2.f-10,txFnt,fntCh,1,1,1);
    dTextC("This may take a moment.",LW/2.f,LH/2.f+20,txFntSm,fntChSm,.6f,.6f,.6f);
    SDL_GL_SwapWindow(w);
}

static void drawTextbox(void) {
    float bx=10,by=LH-120,bw=LW-20,bh=110;
    dStyleBorder(bx-4,by-4,bw+8,bh+8,4);
    dR(bx,by,bw,bh,.08f,.12f,.32f,.82f);
    dR(bx+1,by+1,bw-2,1,.3f,.35f,.55f,.4f);
    float px=bx+8,py=by+10,pw=80,ph=80;
    dStyleBorder(px-3,py-3,pw+6,ph+6,3);
    dR(px,py,pw,ph,.1f,.14f,.35f,.7f);
    if(curFace>=0&&curFace<NUM_FACES&&txFace[curFace])
        dQ(txFace[curFace],px+2,py+2,pw-4,ph-4,1);
    dR(px-3,py-17,pw+6,15,.06f,.08f,.2f,.9f);
    dText("BRIGETTE",px+4,py-5,txFntSm,fntChSm,1,1,1,1);
    dText(shownTxt,px+pw+14,by+30,txFnt,fntCh,1,1,1,1);
    if(txtDone){
        float yo=sinf(frame*.08f)*3;
        float ax=bx+bw-18,ay=by+bh-16+yo;
        glColor4f(.8f,.85f,1,.8f);
        glBegin(GL_TRIANGLES);
        glVertex2f(ax,ay);glVertex2f(ax+10,ay);glVertex2f(ax+5,ay+7);
        glEnd();
    }
}

static void drawCarousel(void) {
    float alpha=boxFadeA/255.0f;
    float centerX=LW*0.58f;
    int gStart=GEN_START[curGen], gCount=GEN_COUNT[curGen];
    int localMenuIdx=menuIdx-gStart;
    if(alpha>0.5f){
        float arA=alpha*(0.5f+0.5f*sinf(frame*0.05f));
        glColor4f(.8f,.85f,1,arA);
        float lx=centerX-180;
        glBegin(GL_TRIANGLES);glVertex2f(lx,LH/2-40);glVertex2f(lx+18,LH/2-55);glVertex2f(lx+18,LH/2-25);glEnd();
        float rx=centerX+180;
        glBegin(GL_TRIANGLES);glVertex2f(rx,LH/2-40);glVertex2f(rx-18,LH/2-55);glVertex2f(rx-18,LH/2-25);glEnd();
    }
    for(int off=-1;off<=1;off++){
        int localIdx=((int)roundf(carAngle)+off+gCount*100)%gCount;
        int idx=gStart+localIdx;
        float pos=off-(carAngle-roundf(carAngle));
        float scale=1.0f-fabsf(pos)*0.35f;
        if(scale<0.1f) continue;
        float bs=160*scale, cx=centerX+pos*190, cy=LH/2-50;
        float a=alpha*(scale*1.5f<1?scale*1.5f:1);
        dR(cx-bs/2+4,cy-bs/2+4,bs,bs,0,0,0,.35f*a);
        dStyleBorder(cx-bs/2-4,cy-bs/2-4,bs+8,bs+8,3);
        dR(cx-bs/2-1,cy-bs/2-1,bs+2,bs+2,.08f,.12f,.32f,.5f*a);
        if(txBox[idx]) dQ(txBox[idx],cx-bs/2,cy-bs/2,bs,bs,a);
        else dTextC("?",cx,cy,txFnt,fntCh,.5f,.5f,.6f);
        if(idx==menuIdx&&st==ST_MENU){
            float ba=.5f+.3f*sinf(frame*.1f);
            dBorder(cx-bs/2-2,cy-bs/2-2,bs+4,bs+4,2,.4f,.6f,1,ba*a);
        }
        if(a>0.5f){
            dTextC(GAMES[idx].name,cx,cy+bs/2+12,txFntSm,fntChSm,1,1,1);
            if(!GAMES[idx].playable)
                dTextC("Coming Soon",cx,cy+bs/2+26,txFntSm,fntChSm,.6f,.6f,.7f);
        }
    }
}

static void drawGenSelect(void) {
    float centerX=LW*0.72f, startY=LH/2-70;
    dTextC("Select Generation",centerX,startY-30,txFnt,fntCh,.8f,.85f,1);
    for(int i=0;i<NUM_GENS;i++){
        float y=startY+i*50;
        float bx=centerX-120, bw=240, bh=40;
        if(i==genMenuIdx){
            float ba=.5f+.3f*sinf(frame*.1f);
            dR(bx,y,bw,bh,.15f,.2f,.45f,.8f);
            dBorder(bx-2,y-2,bw+4,bh+4,2,.4f,.6f,1,ba);
        } else {
            dR(bx,y,bw,bh,.08f,.12f,.32f,.6f);
        }
        dStyleBorder(bx-3,y-3,bw+6,bh+6,3);
        dTextC(GEN_NAMES[i],centerX,y+26,txFnt,fntCh,1,1,1);
    }
}

static SDL_Window *sLauncherWin;
static SDL_GLContext sLauncherCtx;

SDL_Window *GetLauncherWindow(void) { return sLauncherWin; }
SDL_GLContext GetLauncherGLContext(void) { return sLauncherCtx; }

/* ── Main entry point ── */
int RunLauncherUI(void) {
    srand((unsigned)time(NULL));
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,1024);

    if (!sLauncherWin) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
        sLauncherWin=SDL_CreateWindow("PokeRecomp",
            SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,LW,LH,SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
        sLauncherCtx=SDL_GL_CreateContext(sLauncherWin);
    } else {
        SDL_SetWindowSize(sLauncherWin, LW, LH);
        SDL_SetWindowPosition(sLauncherWin, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_SetWindowTitle(sLauncherWin, "PokeRecomp");
        SDL_GL_MakeCurrent(sLauncherWin, sLauncherCtx);
        SDL_ShowWindow(sLauncherWin);
    }
    SDL_Window *win = sLauncherWin;
    SDL_GLContext ctx = sLauncherCtx;
    SDL_GL_SetSwapInterval(1);
    glViewport(0,0,LW,LH);

    /* load assets */
    txBg=loadTex(LA("sprite_background.png"),&txBgW,&txBgH);
    {const char *cf[]={"char_neutral.png","char_look.png","char_blink.png","char_squint.png","char_talk.png","char_happy.png"};
    for(int i=0;i<NUM_FACES;i++) txChar[i]=loadTex(LA(cf[i]),&txChW[i],&txChH[i]);}
    {const char *ff[]={"face_neutral.png","face_look.png","face_blink.png","face_squint.png","face_talk.png","face_happy.png"};
    for(int i=0;i<NUM_FACES;i++) txFace[i]=loadTex(LA(ff[i]),&txFcW[i],&txFcH[i]);}
    {const char *ext[]={".png",".jpg",".jpeg",".bmp"};
    for(int i=0;i<NUM_GAMES;i++){for(int e=0;e<4;e++){
        char p[128]; snprintf(p,128,"%s/boxart_%s%s",sDir,GAMES[i].id,ext[e]);
        txBox[i]=loadTex(p,&txBxW[i],&txBxH[i]); if(txBox[i]) break;}}}
    txFnt=bakeFont("C:\\Windows\\Fonts\\consola.ttf",22,fntCh);
    txFntSm=bakeFont("C:\\Windows\\Fonts\\consola.ttf",14,fntChSm);
    bgMus=Mix_LoadMUS(LA("Brigette's Room [Pok\xc3\xa9mon Box_ Ruby & Sapphire].mp3"));
    sfxA=Mix_LoadWAV(LA("Pokemon (A Button) - Sound Effect (HD).mp3"));

    {FILE *f=fopen("pokefirered.pkmn","rb");if(f){fclose(f);pkmnExists=1;}}
    {FILE *f=fopen("pokeleafgreen.pkmn","rb");if(f){fclose(f);leafgreenPkmnExists=1;}}
    {FILE *f=fopen("pokeruby.pkmn","rb");if(f){fclose(f);rubyPkmnExists=1;}}
    {FILE *f=fopen("pokesapphire.pkmn","rb");if(f){fclose(f);sapphirePkmnExists=1;}}
    {FILE *f=fopen("pokegold.pkmn","rb");if(f){fclose(f);goldPkmnExists=1;}}
    {FILE *f=fopen("pokesilver.pkmn","rb");if(f){fclose(f);silverPkmnExists=1;}}
    {FILE *f=fopen("pokecrystal.pkmn","rb");if(f){fclose(f);crystalPkmnExists=1;}}

    static int sHasLaunched = 0;
    if (sHasLaunched) {
        st=ST_SELECT; fadeA=0; scnA=1; selectedGame=-1; charSlideTarget=0; mainMenuIdx=0;
        if(bgMus && !Mix_PlayingMusic()) Mix_PlayMusic(bgMus,-1);
        {DialogEntry e[]={{"What would you like to do?",FACE_NEUTRAL}};
        startDlg(e,1,ST_MAIN_MENU);}
    } else {
        st=ST_TITLE; fadeA=0; scnA=0; selectedGame=-1;
        sHasLaunched = 1;
    }
    nextBl=SDL_GetTicks()+rr(BLINK_MIN,BLINK_MAX);
    int running=1;

    while(running) {
        SDL_Event ev;
        while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT){selectedGame=-1;running=0;break;}
            int confirm=0;
            if(ev.type==SDL_KEYDOWN){
                SDL_Keycode k=ev.key.keysym.sym;
                confirm=(k==SDLK_RETURN||k==SDLK_SPACE||k==SDLK_z);
                switch(st){
                case ST_TITLE: if(confirm) enterSt(ST_FADE_OUT); break;
                case ST_INTRO_TEXT: case ST_SELECT: case ST_FAREWELL:
                    if(confirm){if(!txtDone){skipTxt();playSfx();}else advDlg();}
                    break;
                case ST_MAIN_MENU:
                    if(k==SDLK_UP||k==SDLK_w){mainMenuIdx=(mainMenuIdx-1+3)%3;playSfx();}
                    else if(k==SDLK_DOWN||k==SDLK_s){mainMenuIdx=(mainMenuIdx+1)%3;playSfx();}
                    else if(confirm){playSfx();
                        if(mainMenuIdx==0){
                            DialogEntry e[1]={{"Which generation would\nyou like to play?",FACE_HAPPY}};
                            startDlg(e,1,ST_GEN_SELECT); st=ST_SELECT;
                        } else if(mainMenuIdx==1){
                            RunBoxTransferUI(win,ctx);
                            glViewport(0,0,LW,LH);
                        }
                    }
                    break;
                case ST_GEN_SELECT:
                    if(k==SDLK_UP||k==SDLK_w){genMenuIdx=(genMenuIdx-1+NUM_GENS)%NUM_GENS;playSfx();}
                    else if(k==SDLK_DOWN||k==SDLK_s){genMenuIdx=(genMenuIdx+1)%NUM_GENS;playSfx();}
                    else if(k==SDLK_ESCAPE||k==SDLK_x){playSfx();enterSt(ST_SELECT);}
                    else if(confirm){playSfx();
                        curGen=genMenuIdx;
                        {DialogEntry e[1]={{"Alright! Let's choose\nan adventure.",FACE_HAPPY}};
                        startDlg(e,1,ST_MENU); st=ST_SELECT;}
                    }
                    break;
                case ST_MENU:
                    {int gStart=GEN_START[curGen], gCount=GEN_COUNT[curGen];
                    int localIdx=menuIdx-gStart;
                    if(k==SDLK_LEFT||k==SDLK_a){localIdx=(localIdx-1+gCount)%gCount;menuIdx=gStart+localIdx;carTarget=(float)localIdx;playSfx();}
                    else if(k==SDLK_RIGHT||k==SDLK_d){localIdx=(localIdx+1)%gCount;menuIdx=gStart+localIdx;carTarget=(float)localIdx;playSfx();}
                    else if(k==SDLK_ESCAPE||k==SDLK_x){playSfx();enterSt(ST_GEN_SELECT);}
                    else if(confirm){
                        if(!GAMES[menuIdx].playable){
                            DialogEntry e[1]={{"That game isn't available\njust yet. Coming soon!",FACE_SQUINT}};
                            startDlg(e,1,ST_MENU); st=ST_SELECT;
                        }
                        else if(menuIdx==7&&!pkmnExists) enterSt(ST_NEED_PKMN);
                        else if(menuIdx==8&&!leafgreenPkmnExists) enterSt(ST_NEED_LEAFGREEN);
                        else if(menuIdx==9&&!rubyPkmnExists) enterSt(ST_NEED_RUBY);
                        else if(menuIdx==10&&!sapphirePkmnExists) enterSt(ST_NEED_SAPPHIRE);
                        else if(menuIdx==4&&!goldPkmnExists) enterSt(ST_NEED_GOLD);
                        else if(menuIdx==5&&!silverPkmnExists) enterSt(ST_NEED_SILVER);
                        else if(menuIdx==6&&!crystalPkmnExists) enterSt(ST_NEED_CRYSTAL);
                        else {boxFadeA=255;enterSt(ST_MENU_FADE);}
                    }}
                    break;
                case ST_NEED_PKMN:
                    if(confirm){
                        if(!txtDone) skipTxt();
                        else {
#ifdef _WIN32
                            OPENFILENAMEA ofn; char rp[MAX_PATH]={0};
                            memset(&ofn,0,sizeof(ofn)); ofn.lStructSize=sizeof(ofn);
                            ofn.lpstrFilter="GBA ROM (*.gba)\0*.gba\0All (*.*)\0*.*\0";
                            ofn.lpstrFile=rp; ofn.nMaxFile=MAX_PATH;
                            ofn.lpstrTitle="Select Pokemon FireRed ROM";
                            ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
                            if(GetOpenFileNameA(&ofn)){
                                showExtractingScreen(win,"FireRed");
                                pkmnExists=PkmnArchive_CreateFromRom(rp,"pokefirered.pkmn");
                            }
#endif
                            if(pkmnExists) enterSt(ST_MENU); else enterSt(ST_MENU);
                        }
                    }
                    break;
                case ST_NEED_LEAFGREEN:
                    if(confirm){
                        if(!txtDone) skipTxt();
                        else {
#ifdef _WIN32
                            OPENFILENAMEA ofn; char rp[MAX_PATH]={0};
                            memset(&ofn,0,sizeof(ofn)); ofn.lStructSize=sizeof(ofn);
                            ofn.lpstrFilter="GBA ROM (*.gba)\0*.gba\0All (*.*)\0*.*\0";
                            ofn.lpstrFile=rp; ofn.nMaxFile=MAX_PATH;
                            ofn.lpstrTitle="Select Pokemon LeafGreen ROM";
                            ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
                            if(GetOpenFileNameA(&ofn)){
                                showExtractingScreen(win,"LeafGreen");
                                leafgreenPkmnExists=PkmnArchive_CreateFromRom(rp,"pokeleafgreen.pkmn");
                            }
#endif
                            if(leafgreenPkmnExists) enterSt(ST_MENU); else enterSt(ST_MENU);
                        }
                    }
                    break;
                case ST_NEED_RUBY:
                    if(confirm){
                        if(!txtDone) skipTxt();
                        else {
#ifdef _WIN32
                            OPENFILENAMEA ofn; char rp[MAX_PATH]={0};
                            memset(&ofn,0,sizeof(ofn)); ofn.lStructSize=sizeof(ofn);
                            ofn.lpstrFilter="GBA ROM (*.gba)\0*.gba\0All (*.*)\0*.*\0";
                            ofn.lpstrFile=rp; ofn.nMaxFile=MAX_PATH;
                            ofn.lpstrTitle="Select Pokemon Ruby ROM";
                            ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
                            if(GetOpenFileNameA(&ofn)){
                                showExtractingScreen(win,"Ruby");
                                rubyPkmnExists=PkmnArchive_CreateFromRom(rp,"pokeruby.pkmn");
                            }
#endif
                            if(rubyPkmnExists) enterSt(ST_MENU); else enterSt(ST_MENU);
                        }
                    }
                    break;
                case ST_NEED_SAPPHIRE:
                    if(confirm){
                        if(!txtDone) skipTxt();
                        else {
#ifdef _WIN32
                            OPENFILENAMEA ofn; char rp[MAX_PATH]={0};
                            memset(&ofn,0,sizeof(ofn)); ofn.lStructSize=sizeof(ofn);
                            ofn.lpstrFilter="GBA ROM (*.gba)\0*.gba\0All (*.*)\0*.*\0";
                            ofn.lpstrFile=rp; ofn.nMaxFile=MAX_PATH;
                            ofn.lpstrTitle="Select Pokemon Sapphire ROM";
                            ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
                            if(GetOpenFileNameA(&ofn)){
                                showExtractingScreen(win,"Sapphire");
                                sapphirePkmnExists=PkmnArchive_CreateFromRom(rp,"pokesapphire.pkmn");
                            }
#endif
                            if(sapphirePkmnExists) enterSt(ST_MENU); else enterSt(ST_MENU);
                        }
                    }
                    break;
                case ST_NEED_GOLD:
                    if(confirm){
                        if(!txtDone) skipTxt();
                        else {
#ifdef _WIN32
                            extern int GBCRom_CreatePkmn(const char*, const char*);
                            OPENFILENAMEA ofn; char rp[MAX_PATH]={0};
                            memset(&ofn,0,sizeof(ofn)); ofn.lStructSize=sizeof(ofn);
                            ofn.lpstrFilter="GBC ROM (*.gbc)\0*.gbc\0GB ROM (*.gb)\0*.gb\0All (*.*)\0*.*\0";
                            ofn.lpstrFile=rp; ofn.nMaxFile=MAX_PATH;
                            ofn.lpstrTitle="Select Pokemon Gold ROM";
                            ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
                            if(GetOpenFileNameA(&ofn)){
                                showExtractingScreen(win,"Gold");
                                goldPkmnExists=GBCRom_CreatePkmn(rp,"pokegold.pkmn");
                            }
#endif
                            if(goldPkmnExists) enterSt(ST_MENU); else enterSt(ST_MENU);
                        }
                    }
                    break;
                case ST_NEED_SILVER:
                    if(confirm){
                        if(!txtDone) skipTxt();
                        else {
#ifdef _WIN32
                            extern int GBCRom_CreatePkmn(const char*, const char*);
                            OPENFILENAMEA ofn; char rp[MAX_PATH]={0};
                            memset(&ofn,0,sizeof(ofn)); ofn.lStructSize=sizeof(ofn);
                            ofn.lpstrFilter="GBC ROM (*.gbc)\0*.gbc\0GB ROM (*.gb)\0*.gb\0All (*.*)\0*.*\0";
                            ofn.lpstrFile=rp; ofn.nMaxFile=MAX_PATH;
                            ofn.lpstrTitle="Select Pokemon Silver ROM";
                            ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
                            if(GetOpenFileNameA(&ofn)){
                                showExtractingScreen(win,"Silver");
                                silverPkmnExists=GBCRom_CreatePkmn(rp,"pokesilver.pkmn");
                            }
#endif
                            if(silverPkmnExists) enterSt(ST_MENU); else enterSt(ST_MENU);
                        }
                    }
                    break;
                case ST_NEED_CRYSTAL:
                    if(confirm){
                        if(!txtDone) skipTxt();
                        else {
#ifdef _WIN32
                            extern int GBCRom_CreatePkmn(const char*, const char*);
                            OPENFILENAMEA ofn; char rp[MAX_PATH]={0};
                            memset(&ofn,0,sizeof(ofn)); ofn.lStructSize=sizeof(ofn);
                            ofn.lpstrFilter="GBC ROM (*.gbc)\0*.gbc\0GB ROM (*.gb)\0*.gb\0All (*.*)\0*.*\0";
                            ofn.lpstrFile=rp; ofn.nMaxFile=MAX_PATH;
                            ofn.lpstrTitle="Select Pokemon Crystal ROM";
                            ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
                            if(GetOpenFileNameA(&ofn)){
                                showExtractingScreen(win,"Crystal");
                                crystalPkmnExists=GBCRom_CreatePkmn(rp,"pokecrystal.pkmn");
                            }
#endif
                            if(crystalPkmnExists) enterSt(ST_MENU); else enterSt(ST_MENU);
                        }
                    }
                    break;
                default: break;
                }
            }
            if(ev.type==SDL_MOUSEBUTTONDOWN&&ev.button.button==SDL_BUTTON_LEFT){
                if(st==ST_TITLE) enterSt(ST_FADE_OUT);
                else if(st==ST_MENU){
                    int mx=ev.button.x;
                    if(mx<LW/2-100){menuIdx=(menuIdx-1+NUM_GAMES)%NUM_GAMES;carTarget=(float)menuIdx;}
                    else if(mx>LW/2+100){menuIdx=(menuIdx+1)%NUM_GAMES;carTarget=(float)menuIdx;}
                    else{
                        if(menuIdx==0&&!pkmnExists) enterSt(ST_NEED_PKMN);
                        else if(menuIdx==1&&!leafgreenPkmnExists) enterSt(ST_NEED_LEAFGREEN);
                        else if(menuIdx==2&&!rubyPkmnExists) enterSt(ST_NEED_RUBY);
                        else if(menuIdx==3&&!sapphirePkmnExists) enterSt(ST_NEED_SAPPHIRE);
                        else if(menuIdx==4&&!goldPkmnExists) enterSt(ST_NEED_GOLD);
                        else if(menuIdx==5&&!silverPkmnExists) enterSt(ST_NEED_SILVER);
                        else if(menuIdx==6&&!crystalPkmnExists) enterSt(ST_NEED_CRYSTAL);
                        else {boxFadeA=255;enterSt(ST_MENU_FADE);}
                    }
                }
                else if(st==ST_INTRO_TEXT||st==ST_SELECT||st==ST_FAREWELL){
                    if(!txtDone) skipTxt(); else advDlg();
                }
            }
        }

        /* update */
        frame++;
        Uint32 now=SDL_GetTicks();
        if(!txtDone&&chIdx<(int)strlen(fullTxt)){
            if(now-txtTm>=TEXT_SPEED_MS){
                txtTm=now; chIdx++;
                memcpy(shownTxt,fullTxt,chIdx); shownTxt[chIdx]=0;
                if(chIdx%3==0){talkTog=!talkTog;curFace=talkTog?FACE_TALK:baseFace;}
                if(chIdx>=(int)strlen(fullTxt)){txtDone=1;curFace=baseFace;}
            }
        }
        if(txtDone&&st!=ST_TITLE&&st!=ST_FADE_OUT&&st!=ST_LAUNCH){
            if(now>=nextBl&&now<nextBl+BLINK_DUR) curFace=FACE_BLINK;
            else if(now>=nextBl+BLINK_DUR){curFace=baseFace;nextBl=now+rr(BLINK_MIN,BLINK_MAX);}
        }
        if(st==ST_FADE_OUT){fadeA+=6;if(fadeA>=255){fadeA=255;enterSt(ST_INTRO_TEXT);}}
        if(st!=ST_TITLE&&st!=ST_FADE_OUT){scnA+=0.012f;if(scnA>1)scnA=1;}
        if(st==ST_MENU_FADE){boxFadeA-=6;if(boxFadeA<=0){boxFadeA=0;enterSt(ST_FAREWELL);}}
        if(st==ST_LAUNCH){fadeA+=3;if(fadeA>255){fadeA=255;running=0;}}
        else if(st==ST_INTRO_TEXT||st==ST_SELECT||st==ST_MENU||st==ST_MENU_FADE||st==ST_FAREWELL){fadeA-=4;if(fadeA<0)fadeA=0;}
        carAngle+=(carTarget-carAngle)*0.12f;
        charSlide+=(charSlideTarget-charSlide)*0.08f;

        /* draw */
        /* Handle window resize - maintain aspect ratio */
        {int ww,wh; SDL_GetWindowSize(win,&ww,&wh);
        float sx=(float)ww/LW, sy=(float)wh/LH;
        float s=sx<sy?sx:sy;
        int vw=(int)(LW*s), vh=(int)(LH*s);
        glViewport((ww-vw)/2,(wh-vh)/2,vw,vh);}
        glClearColor(.04f,.04f,.1f,1); glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0,LW,LH,0,-1,1);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        if(st==ST_TITLE||st==ST_FADE_OUT){
            dGV(0,0,LW,LH,.06f,.04f,.16f,1,.02f,.02f,.06f,1);
            dR(0,LH/2-50,LW,1,.3f,.24f,.55f,.4f);
            dR(0,LH/2+70,LW,1,.3f,.24f,.55f,.2f);
            dTextC("P O K E",LW/2.f,LH/2.f-60,txFntSm,fntChSm,.5f,.5f,.5f);
            dTextC("RECOMP",LW/2.f,LH/2.f-30,txFnt,fntCh,1,.8f,0);
            dTextC("- BRIGETTE -",LW/2.f,LH/2.f+5,txFntSm,fntChSm,.47f,.4f,.67f);
            if(st==ST_TITLE&&(frame%40)<25)
                dTextC("PRESS START",LW/2.f,LH/2.f+50,txFntSm,fntChSm,1,1,1);
        }

        if(st!=ST_TITLE&&st!=ST_FADE_OUT){
            if(txBg){
                float sx=(float)LW/txBgW,sy=(float)LH/txBgH;
                float s=sx>sy?sx:sy;
                float dw=txBgW*s,dh=txBgH*s;
                dQ(txBg,(LW-dw)/2,(LH-dh)/2,dw,dh,scnA);
            }
            if(st==ST_MENU||st==ST_MENU_FADE){
                float dm=st==ST_MENU_FADE?.35f*(boxFadeA/255.f):.35f;
                dR(0,0,LW,LH,0,0,0,dm);
            }
            {int ci=curFace<0?0:curFace>=NUM_FACES?NUM_FACES-1:curFace;
            if(txChar[ci]){
                float cs=1.05f;
                float cw=txChW[ci]*cs,ch=txChH[ci]*cs;
                float ctrX=LW/2.f-cw/2,leftX=20;
                float cx=ctrX+(leftX-ctrX)*charSlide;
                dQ(txChar[ci],cx,LH-ch-10,cw,ch,scnA);
            }}
        }

        if(st==ST_MAIN_MENU){
            const char *opts[]={"GAMES","BOXES","SETTINGS"};
            float bx=LW-200,by=LH-250;
            dStyleBorder(bx-6,by-10,180,148,3);
            dR(bx-2,by-6,172,140,.08f,.12f,.32f,.85f);
            for(int i=0;i<3;i++){
                float iy=by+i*45;
                if(i==mainMenuIdx){
                    dR(bx+2,iy-2,160,34,.15f,.2f,.45f,.6f);
                    dText(">",bx+6,iy+18,txFnt,fntCh,1,.8f,0,1);
                }
                dText(opts[i],bx+26,iy+18,txFnt,fntCh,1,1,1,1);
            }
        }

        if(st==ST_MENU||st==ST_MENU_FADE) drawCarousel();
        if(st==ST_GEN_SELECT) drawGenSelect();

        if(st==ST_INTRO_TEXT||st==ST_SELECT||st==ST_GEN_SELECT||st==ST_MAIN_MENU||st==ST_MENU||st==ST_FAREWELL||st==ST_NEED_PKMN||st==ST_NEED_LEAFGREEN||st==ST_NEED_RUBY||st==ST_NEED_SAPPHIRE)
            drawTextbox();

        if(st==ST_LAUNCH&&fadeA>200){
            char buf[64]; snprintf(buf,64,"POKEMON %s",GAMES[menuIdx].name);
            for(char *p=buf;*p;p++) if(*p>='a'&&*p<='z') *p-=32;
            dTextC(buf,LW/2.f,LH/2.f-10,txFnt,fntCh,1,.8f,0);
            if((frame%40)<25) dTextC("Launching...",LW/2.f,LH/2.f+20,txFntSm,fntChSm,.67f,.67f,.67f);
        }

        if(fadeA>0) dR(0,0,LW,LH,0,0,0,fadeA/255.f);

        SDL_GL_SwapWindow(win);
    }

    /* DON'T free launcher resources — keep CRT heap pages committed.
     * Freeing these causes the CRT heap to decommit pages, which then
     * get accessed through stale pointers during Pokemon Center transitions. */
    if(bgMus){Mix_HaltMusic();} /* stop playback but don't free the buffer */

    return selectedGame;
}
#endif
