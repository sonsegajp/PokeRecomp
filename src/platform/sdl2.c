#ifdef PLATFORM_SDL2
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <xinput.h>
#include <dbghelp.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

/* OpenGL headers */
#ifdef _WIN32
#include <GL/gl.h>
#include <GL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include "global.h"
#include "main.h"
#include "platform.h"
#include "gba/defines.h"
#include "gba/flash_internal.h"
#include "gba/m4a_internal.h"
#include "m4a.h"
#include "cgb_audio.h"
#include "gba/flash_internal.h"
#include "platform/dma.h"
#include "platform/framedraw.h"
#include "platform/rom_loader.h"
#include "platform/imgui_bridge.h"

extern IntrFunc gIntrTable[];

SDL_Thread *mainLoopThread;
SDL_Window *sdlWindow;
SDL_GLContext sdlGLContext;
SDL_sem *vBlankSemaphore;
SDL_atomic_t isFrameAvailable;
bool speedUp = false;
unsigned int videoScale = 3;
bool videoScaleChanged = false;
bool isRunning = true;
bool paused = false;
double simTime = 0;
double lastGameTime = 0;
double curGameTime = 0;
double fixedTimestep = 1.0 / 60.0; // 16.666667ms
double timeScale = 1.0;
struct SiiRtcInfo internalClock;

static FILE *sSaveFile = NULL;
static SDL_AudioDeviceID sAudioDeviceId = 0;

int gIsLeafGreen = 0;

extern void AgbMain(void);
extern void DoSoftReset(void);
extern int GetRenderWidth(void);
extern void ImGuiBridge_SyncShinyCharm(void);

static int sGameViewportX, sGameViewportY, sGameViewportW, sGameViewportH;
void Platform_GetGameViewport(int *x, int *y, int *w, int *h)
{
    *x = sGameViewportX; *y = sGameViewportY;
    *w = sGameViewportW; *h = sGameViewportH;
}

int DoMain(void *param);
void ProcessEvents(void);
void VDraw(void);

static void ReadSaveFile(char *path);
static void StoreSaveFile(void);
static void CloseSaveFile(void);

static void UpdateInternalClock(void);

/* ------------------------------------------------------------------ */
/* OpenGL function pointers (loaded at runtime on Windows)            */
/* ------------------------------------------------------------------ */
#ifdef _WIN32
/* Shader functions */
static PFNGLCREATESHADERPROC            glCreateShader;
static PFNGLSHADERSOURCEPROC            glShaderSource;
static PFNGLCOMPILESHADERPROC           glCompileShader;
static PFNGLGETSHADERIVPROC             glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
static PFNGLCREATEPROGRAMPROC           glCreateProgram;
static PFNGLATTACHSHADERPROC            glAttachShader;
static PFNGLLINKPROGRAMPROC             glLinkProgram;
static PFNGLGETPROGRAMIVPROC            glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
static PFNGLUSEPROGRAMPROC              glUseProgram;
static PFNGLDELETESHADERPROC            glDeleteShader;
static PFNGLDELETEPROGRAMPROC           glDeleteProgram;
static PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
static PFNGLUNIFORM1IPROC              glUniform1i;
static PFNGLUNIFORM1FPROC             glUniform1f;
static PFNGLUNIFORM2FPROC             glUniform2f;
static PFNGLUNIFORMMATRIX4FVPROC      glUniformMatrix4fv;
/* VAO/VBO functions */
static PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
static PFNGLGENBUFFERSPROC              glGenBuffers;
static PFNGLBINDBUFFERPROC              glBindBuffer;
static PFNGLBUFFERDATAPROC              glBufferData;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
static PFNGLACTIVETEXTUREPROC           glActiveTexture_;
static PFNGLBINDATTRIBLOCATIONPROC      glBindAttribLocation;
/* FBO functions */
static PFNGLGENFRAMEBUFFERSPROC         glGenFramebuffers;
static PFNGLBINDFRAMEBUFFERPROC         glBindFramebuffer;
static PFNGLFRAMEBUFFERTEXTURE2DPROC    glFramebufferTexture2D;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC  glCheckFramebufferStatus;
static PFNGLDELETEFRAMEBUFFERSPROC      glDeleteFramebuffers;

static void LoadGLFunctions(void)
{
    glCreateShader            = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource            = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader           = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv             = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog        = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glCreateProgram           = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    glAttachShader            = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    glLinkProgram             = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    glGetProgramiv            = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog       = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glUseProgram              = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    glDeleteShader            = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
    glDeleteProgram           = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
    glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    glUniform1i               = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
    glUniform1f               = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
    glUniform2f               = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
    glUniformMatrix4fv        = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
    glGenVertexArrays         = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
    glBindVertexArray         = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    glGenBuffers              = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
    glBindBuffer              = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
    glBufferData              = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
    glActiveTexture_          = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");
    glBindAttribLocation      = (PFNGLBINDATTRIBLOCATIONPROC)SDL_GL_GetProcAddress("glBindAttribLocation");
    glGenFramebuffers         = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
    glBindFramebuffer         = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D    = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
    glCheckFramebufferStatus  = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
    glDeleteFramebuffers      = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
}
#else
#define glActiveTexture_ glActiveTexture
static void LoadGLFunctions(void) {}
#endif

/* ------------------------------------------------------------------ */
/* Shader system — multi-pass libretro GLSL preset support            */
/* ------------------------------------------------------------------ */

#include "../../lib/stb_image.h"

static const char *sPassthroughVS =
    "#version 130\n"
    "in vec4 VertexCoord;\n"
    "in vec4 TexCoord;\n"
    "out vec2 vTexCoord;\n"
    "void main() {\n"
    "    gl_Position = VertexCoord;\n"
    "    vTexCoord = TexCoord.xy;\n"
    "}\n";

static const char *sPassthroughFS =
    "#version 130\n"
    "in vec2 vTexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D uTexture;\n"
    "void main() {\n"
    "    FragColor = vec4(texture(uTexture, vTexCoord).rgb, 1.0);\n"
    "}\n";

#define MAX_SHADERS 128
#define MAX_PASSES 8
#define MAX_PARAMS 64
#define MAX_LUT_TEXTURES 8

enum { SCALE_SOURCE = 0, SCALE_VIEWPORT = 1, SCALE_ABSOLUTE = 2 };

struct ShaderParam {
    char name[64];
    float value;
};

struct ShaderPass {
    GLuint program;
    GLuint fbo;
    GLuint fboTex;
    int scaleType;
    float scale;
    int filterLinear;
    char alias[64];
};

struct LutTexture {
    char name[64];
    GLuint tex;
};

struct ShaderEntry {
    char name[64];
    int numPasses;
    struct ShaderPass passes[MAX_PASSES];
    int numParams;
    struct ShaderParam params[MAX_PARAMS];
    int numLuts;
    struct LutTexture luts[MAX_LUT_TEXTURES];
};

static struct ShaderEntry sShaders[MAX_SHADERS];
static int sNumShaders = 0;
static int sActiveShader = 0;

/* GL objects */
static GLuint sGameTexture = 0;
static GLuint sQuadVAO = 0;
static GLuint sQuadVBO = 0;
static int sLastRenderWidth = 0;

static char *LoadFileText(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(len + 1);
    if (buf) { fread(buf, 1, len, f); buf[len] = 0; }
    fclose(f);
    return buf;
}

static void ParsePragmaParams(const char *src, struct ShaderEntry *entry)
{
    const char *p = src;
    while ((p = strstr(p, "#pragma parameter")) != NULL) {
        p += 17;
        while (*p == ' ' || *p == '\t') p++;
        if (entry->numParams >= MAX_PARAMS) break;
        struct ShaderParam *param = &entry->params[entry->numParams];
        char rest[256];
        int n = 0;
        while (*p && *p != '\n' && n < (int)sizeof(rest) - 1) rest[n++] = *p++;
        rest[n] = 0;
        char dummy[128];
        float defVal = 0;
        if (sscanf(rest, "%63s \"%127[^\"]\" %f", param->name, dummy, &defVal) >= 3) {
            param->value = defVal;
            int dup = 0;
            for (int i = 0; i < entry->numParams; i++)
                if (strcmp(entry->params[i].name, param->name) == 0) { dup = 1; break; }
            if (!dup) entry->numParams++;
        }
    }
}

static GLuint BuildLibretroShader(const char *glslSource, struct ShaderEntry *entry)
{
    static char strippedBuf[64 * 1024];
    {
        const char *r = glslSource;
        char *w = strippedBuf;
        char *end = strippedBuf + sizeof(strippedBuf) - 1;
        while (*r && w < end) {
            if (*r == '#' && strncmp(r, "#version", 8) == 0) {
                while (*r && *r != '\n') r++;
                if (*r == '\n') r++;
                continue;
            }
            if (strncmp(r, "out vec4 FragColor", 18) == 0) {
                while (*r && *r != '\n') r++;
                if (*r == '\n') r++;
                continue;
            }
            *w++ = *r++;
        }
        *w = '\0';
    }

    if (entry) ParsePragmaParams(glslSource, entry);

    const char *src = strippedBuf;
    const char *vertParts[3];
    const char *fragParts[3];

    vertParts[0] =
        "#version 130\n"
        "#define VERTEX\n"
        "#define PARAMETER_UNIFORM\n"
        "#define COMPAT_VARYING out\n"
        "#define COMPAT_ATTRIBUTE in\n"
        "#define COMPAT_TEXTURE texture\n"
        "#define COMPAT_PRECISION\n"
        "#define attribute in\n"
        "#define varying out\n"
        "#define texture2D texture\n"
        ;
    vertParts[1] = src;
    vertParts[2] = NULL;

    fragParts[0] =
        "#version 130\n"
        "#define FRAGMENT\n"
        "#define PARAMETER_UNIFORM\n"
        "#define COMPAT_VARYING in\n"
        "#define COMPAT_ATTRIBUTE in\n"
        "#define COMPAT_TEXTURE texture\n"
        "#define COMPAT_PRECISION\n"
        "#define varying in\n"
        "#define texture2D texture\n"
        "#define gl_FragColor FragColor\n"
        "out vec4 FragColor;\n"
        ;
    fragParts[1] = src;
    fragParts[2] = NULL;

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 2, vertParts, NULL);
    glCompileShader(vs);
    GLint ok;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(vs, sizeof(log), NULL, log);
        printf("VS compile error: %s\n", log);
        glDeleteShader(vs);
        return 0;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 2, fragParts, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(fs, sizeof(log), NULL, log);
        printf("FS compile error: %s\n", log);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glBindAttribLocation(prog, 0, "VertexCoord");
    glBindAttribLocation(prog, 1, "TexCoord");
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        printf("Shader link error: %s\n", log);
        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(prog);
        return 0;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

static GLuint BuildPassthroughProgram(void)
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &sPassthroughVS, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &sPassthroughFS, NULL);
    glCompileShader(fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glBindAttribLocation(prog, 0, "VertexCoord");
    glBindAttribLocation(prog, 1, "TexCoord");
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

static GLuint LoadTextureFromFile(const char *path, int filterLinear)
{
    int w, h, ch;
    unsigned char *data = stbi_load(path, &w, &h, &ch, 4);
    if (!data) { printf("Failed to load texture: %s\n", path); return 0; }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLenum filt = filterLinear ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    printf("  Loaded LUT texture: %s (%dx%d)\n", path, w, h);
    return tex;
}

static void CreatePassFBO(struct ShaderPass *pass, int w, int h)
{
    if (pass->fbo) { glDeleteFramebuffers(1, &pass->fbo); pass->fbo = 0; }
    if (pass->fboTex) { glDeleteTextures(1, &pass->fboTex); pass->fboTex = 0; }
    glGenTextures(1, &pass->fboTex);
    glBindTexture(GL_TEXTURE_2D, pass->fboTex);
    GLenum filt = pass->filterLinear ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenFramebuffers(1, &pass->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, pass->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pass->fboTex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void TrimValue(char *s)
{
    while (*s == ' ' || *s == '\t' || *s == '"') s++;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '"' || *end == '\n' || *end == '\r'))
        *end-- = 0;
}

static char *GetValue(char *line)
{
    char *eq = strchr(line, '=');
    if (!eq) return NULL;
    eq++;
    while (*eq == ' ' || *eq == '\t' || *eq == '"') eq++;
    char *end = eq + strlen(eq) - 1;
    while (end > eq && (*end == ' ' || *end == '\t' || *end == '"' || *end == '\n' || *end == '\r'))
        *end-- = 0;
    return eq;
}

static int LoadGLSLPreset(const char *presetPath, const char *presetDir, struct ShaderEntry *entry)
{
    char *text = LoadFileText(presetPath);
    if (!text) return 0;

    int numPasses = 0;
    char shaderFiles[MAX_PASSES][256];
    memset(shaderFiles, 0, sizeof(shaderFiles));
    memset(entry->passes, 0, sizeof(entry->passes));
    entry->numLuts = 0;

    for (int i = 0; i < MAX_PASSES; i++) {
        entry->passes[i].scale = 1.0f;
        entry->passes[i].scaleType = SCALE_SOURCE;
    }

    char textureNames[MAX_LUT_TEXTURES][64];
    char texturePaths[MAX_LUT_TEXTURES][256];
    int textureLinear[MAX_LUT_TEXTURES];
    int numTextures = 0;
    memset(textureNames, 0, sizeof(textureNames));
    memset(texturePaths, 0, sizeof(texturePaths));
    memset(textureLinear, 0, sizeof(textureLinear));

    char *textCopy = _strdup(text);
    char *line = strtok(textCopy, "\n\r");
    while (line) {
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '#' || *line == 0) { line = strtok(NULL, "\n\r"); continue; }

        int idx;
        char val[256];

        if (sscanf(line, "shaders = %d", &numPasses) == 1) {
            if (numPasses > MAX_PASSES) numPasses = MAX_PASSES;
        } else if (sscanf(line, "shader%d = %255[^\n\r]", &idx, val) == 2 && idx < MAX_PASSES) {
            TrimValue(val);
            strncpy(shaderFiles[idx], val, 255);
        } else if (sscanf(line, "filter_linear%d = %255[^\n\r]", &idx, val) == 2 && idx < MAX_PASSES) {
            TrimValue(val);
            entry->passes[idx].filterLinear = (strcmp(val, "true") == 0);
        } else if (sscanf(line, "scale_type%d = %255[^\n\r]", &idx, val) == 2 && idx < MAX_PASSES) {
            TrimValue(val);
            if (strcmp(val, "viewport") == 0) entry->passes[idx].scaleType = SCALE_VIEWPORT;
            else if (strcmp(val, "absolute") == 0) entry->passes[idx].scaleType = SCALE_ABSOLUTE;
            else entry->passes[idx].scaleType = SCALE_SOURCE;
        } else if (sscanf(line, "scale%d = %255[^\n\r]", &idx, val) == 2 && idx < MAX_PASSES) {
            TrimValue(val);
            entry->passes[idx].scale = (float)atof(val);
        } else if (sscanf(line, "alias%d = %255[^\n\r]", &idx, val) == 2 && idx < MAX_PASSES) {
            TrimValue(val);
            strncpy(entry->passes[idx].alias, val, 63);
        } else if (strncmp(line, "textures", 8) == 0 && (line[8] == ' ' || line[8] == '=')) {
            char *v = GetValue(line);
            if (v) {
                char *tok = strtok(v, ";,");
                while (tok && numTextures < MAX_LUT_TEXTURES) {
                    while (*tok == ' ') tok++;
                    char *te = tok + strlen(tok) - 1;
                    while (te > tok && *te == ' ') *te-- = 0;
                    strncpy(textureNames[numTextures], tok, 63);
                    numTextures++;
                    tok = strtok(NULL, ";,");
                }
            }
        } else {
            for (int t = 0; t < numTextures; t++) {
                if (textureNames[t][0] && strncmp(line, textureNames[t], strlen(textureNames[t])) == 0) {
                    char *rest = line + strlen(textureNames[t]);
                    if (strncmp(rest, "_linear", 7) == 0) {
                        char *v = GetValue(line);
                        if (v) textureLinear[t] = (strcmp(v, "true") == 0);
                    } else if (*rest == ' ' || *rest == '=') {
                        char *v = GetValue(line);
                        if (v) strncpy(texturePaths[t], v, 255);
                    }
                }
            }
            for (int i = 0; i < entry->numParams; i++) {
                if (strncmp(line, entry->params[i].name, strlen(entry->params[i].name)) == 0) {
                    char *v = GetValue(line);
                    if (v) entry->params[i].value = (float)atof(v);
                }
            }
        }
        line = strtok(NULL, "\n\r");
    }
    free(textCopy);
    free(text);

    if (numPasses == 0) return 0;
    entry->numPasses = numPasses;

    for (int i = 0; i < numPasses; i++) {
        for (char *p = shaderFiles[i]; *p; p++) if (*p == '\\') *p = '/';
        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", presetDir, shaderFiles[i]);
        char *src = LoadFileText(fullPath);
        if (!src) {
            const char *stripped = shaderFiles[i];
            if (strncmp(stripped, "shaders/", 8) == 0) stripped += 8;
            snprintf(fullPath, sizeof(fullPath), "%s/%s", presetDir, stripped);
            src = LoadFileText(fullPath);
        }
        if (!src) {
            printf("  Failed to load pass %d: %s\n", i, shaderFiles[i]);
            entry->numPasses = 0;
            return 0;
        }
        entry->passes[i].program = BuildLibretroShader(src, entry);
        free(src);
        if (!entry->passes[i].program) {
            printf("  Failed to compile pass %d\n", i);
            entry->numPasses = 0;
            return 0;
        }
    }

    for (int t = 0; t < numTextures; t++) {
        if (texturePaths[t][0] == 0) continue;
        for (char *p = texturePaths[t]; *p; p++) if (*p == '\\') *p = '/';
        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", presetDir, texturePaths[t]);
        GLuint tex = LoadTextureFromFile(fullPath, textureLinear[t]);
        if (tex && entry->numLuts < MAX_LUT_TEXTURES) {
            strncpy(entry->luts[entry->numLuts].name, textureNames[t], 63);
            entry->luts[entry->numLuts].tex = tex;
            entry->numLuts++;
        }
    }

    return 1;
}

static void SetPassUniforms(GLuint prog, int texW, int texH, int outW, int outH,
                            struct ShaderEntry *entry, int passIdx)
{
    GLint loc;
    static const char *samplerNames[] = {"Texture", "Source", "Original", "uTexture", "tex", "s_p", NULL};
    for (int i = 0; samplerNames[i]; i++) {
        loc = glGetUniformLocation(prog, samplerNames[i]);
        if (loc >= 0) { glUniform1i(loc, 0); break; }
    }

    loc = glGetUniformLocation(prog, "TextureSize");
    if (loc >= 0) glUniform2f(loc, (float)texW, (float)texH);
    loc = glGetUniformLocation(prog, "InputSize");
    if (loc >= 0) glUniform2f(loc, (float)texW, (float)texH);
    loc = glGetUniformLocation(prog, "OrigInputSize");
    if (loc >= 0) glUniform2f(loc, (float)texW, (float)texH);
    loc = glGetUniformLocation(prog, "OutputSize");
    if (loc >= 0) glUniform2f(loc, (float)outW, (float)outH);
    loc = glGetUniformLocation(prog, "uTextureSize");
    if (loc >= 0) glUniform2f(loc, (float)texW, (float)texH);
    loc = glGetUniformLocation(prog, "uOutputSize");
    if (loc >= 0) glUniform2f(loc, (float)outW, (float)outH);

    static int sFrameCount = 0;
    loc = glGetUniformLocation(prog, "FrameCount");
    if (loc >= 0) glUniform1i(loc, sFrameCount);
    loc = glGetUniformLocation(prog, "FrameDirection");
    if (loc >= 0) glUniform1i(loc, 1);
    if (passIdx == 0) sFrameCount++;

    loc = glGetUniformLocation(prog, "MVPMatrix");
    if (loc >= 0) {
        static const float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        glUniformMatrix4fv(loc, 1, GL_FALSE, identity);
    }

    for (int i = 0; i < entry->numParams; i++) {
        loc = glGetUniformLocation(prog, entry->params[i].name);
        if (loc >= 0) glUniform1f(loc, entry->params[i].value);
    }

    int texUnit = 1;
    for (int i = 0; i < entry->numLuts; i++) {
        loc = glGetUniformLocation(prog, entry->luts[i].name);
        if (loc >= 0) {
            glActiveTexture_(GL_TEXTURE0 + texUnit);
            glBindTexture(GL_TEXTURE_2D, entry->luts[i].tex);
            glUniform1i(loc, texUnit);
            texUnit++;
        }
        char sizeName[80];
        snprintf(sizeName, sizeof(sizeName), "%sSize", entry->luts[i].name);
        loc = glGetUniformLocation(prog, sizeName);
        if (loc >= 0) {
            int tw, th;
            glBindTexture(GL_TEXTURE_2D, entry->luts[i].tex);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
            glUniform2f(loc, (float)tw, (float)th);
        }
    }

    for (int i = 0; i < entry->numPasses; i++) {
        if (entry->passes[i].alias[0] && i < passIdx && entry->passes[i].fboTex) {
            loc = glGetUniformLocation(prog, entry->passes[i].alias);
            if (loc >= 0) {
                glActiveTexture_(GL_TEXTURE0 + texUnit);
                glBindTexture(GL_TEXTURE_2D, entry->passes[i].fboTex);
                glUniform1i(loc, texUnit);
                texUnit++;
            }
            char sizeName[80];
            snprintf(sizeName, sizeof(sizeName), "%sSize", entry->passes[i].alias);
            loc = glGetUniformLocation(prog, sizeName);
            if (loc >= 0) {
                int tw, th;
                glBindTexture(GL_TEXTURE_2D, entry->passes[i].fboTex);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
                glUniform2f(loc, (float)tw, (float)th);
            }
        }
    }
    glActiveTexture_(GL_TEXTURE0);
}

static void RenderShaderEntry(struct ShaderEntry *entry, int viewW, int viewH, int srcW, int srcH)
{
    GLuint inputTex = sGameTexture;
    int inW = srcW, inH = srcH;

    for (int p = 0; p < entry->numPasses; p++) {
        struct ShaderPass *pass = &entry->passes[p];
        int isLast = (p == entry->numPasses - 1);

        int outW, outH;
        if (isLast) {
            outW = viewW;
            outH = viewH;
        } else {
            switch (pass->scaleType) {
            case SCALE_VIEWPORT:
                outW = (int)(viewW * pass->scale);
                outH = (int)(viewH * pass->scale);
                break;
            case SCALE_ABSOLUTE:
                outW = (int)pass->scale;
                outH = (int)pass->scale;
                break;
            default:
                outW = (int)(inW * pass->scale);
                outH = (int)(inH * pass->scale);
                break;
            }
            if (outW < 1) outW = 1;
            if (outH < 1) outH = 1;
            CreatePassFBO(pass, outW, outH);
            glBindFramebuffer(GL_FRAMEBUFFER, pass->fbo);
            glViewport(0, 0, outW, outH);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        glUseProgram(pass->program);
        glActiveTexture_(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputTex);

        GLenum filt = pass->filterLinear ? GL_LINEAR : GL_NEAREST;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt);

        SetPassUniforms(pass->program, inW, inH, outW, outH, entry, p);

        glBindVertexArray(sQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        if (!isLast) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            inputTex = pass->fboTex;
            inW = outW;
            inH = outH;
        }
    }
    glUseProgram(0);
}

static void InitShaders(void)
{
    memset(sShaders, 0, sizeof(sShaders));
    sShaders[0].numPasses = 1;
    sShaders[0].passes[0].program = BuildPassthroughProgram();
    strncpy(sShaders[0].name, "None", sizeof(sShaders[0].name));
    sNumShaders = 1;

    const char *shaderDir = "shaders";
#ifdef _WIN32
    {
        char searchStack[32][256];
        int stackDepth = 0;
        strncpy(searchStack[0], shaderDir, 256);
        stackDepth = 1;

        while (stackDepth > 0 && sNumShaders < MAX_SHADERS) {
            stackDepth--;
            char currentDir[256];
            strncpy(currentDir, searchStack[stackDepth], 256);

            {
                char dirPattern[256];
                snprintf(dirPattern, sizeof(dirPattern), "%s\\*", currentDir);
                WIN32_FIND_DATAA dfd;
                HANDLE hDir = FindFirstFileA(dirPattern, &dfd);
                if (hDir != INVALID_HANDLE_VALUE) {
                    do {
                        if ((dfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                            strcmp(dfd.cFileName, ".") != 0 && strcmp(dfd.cFileName, "..") != 0 &&
                            stackDepth < 32) {
                            snprintf(searchStack[stackDepth], 256, "%s\\%s", currentDir, dfd.cFileName);
                            stackDepth++;
                        }
                    } while (FindNextFileA(hDir, &dfd));
                    FindClose(hDir);
                }
            }

            const char *exts[] = {"*.glslp", "*.glsl"};
            for (int e = 0; e < 2 && sNumShaders < MAX_SHADERS; e++) {
                char pattern[256];
                snprintf(pattern, sizeof(pattern), "%s\\%s", currentDir, exts[e]);
                WIN32_FIND_DATAA fd;
                HANDLE hFind = FindFirstFileA(pattern, &fd);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        if (sNumShaders >= MAX_SHADERS) break;
                        char baseName[64];
                        strncpy(baseName, fd.cFileName, sizeof(baseName));
                        char *dot = strrchr(baseName, '.');
                        if (dot) *dot = 0;
                        int duplicate = 0;
                        for (int i = 1; i < sNumShaders; i++)
                            if (strcmp(sShaders[i].name, baseName) == 0) { duplicate = 1; break; }
                        if (duplicate) continue;

                        char path[512];
                        snprintf(path, sizeof(path), "%s\\%s", currentDir, fd.cFileName);

                        struct ShaderEntry *entry = &sShaders[sNumShaders];
                        memset(entry, 0, sizeof(*entry));
                        strncpy(entry->name, baseName, sizeof(entry->name));

                        int ok = 0;
                        if (e == 0) {
                            printf("Loading preset: %s\n", path);
                            ok = LoadGLSLPreset(path, currentDir, entry);
                        } else {
                            char *src = LoadFileText(path);
                            if (src) {
                                entry->numPasses = 1;
                                entry->passes[0].scale = 1.0f;
                                entry->passes[0].scaleType = SCALE_SOURCE;
                                entry->passes[0].program = BuildLibretroShader(src, entry);
                                ok = (entry->passes[0].program != 0);
                                free(src);
                            }
                        }

                        if (ok) {
                            printf("Loaded shader: %s (%d passes, %d params, %d textures)\n",
                                   entry->name, entry->numPasses, entry->numParams, entry->numLuts);
                            sNumShaders++;
                        }
                    } while (FindNextFileA(hFind, &fd));
                    FindClose(hFind);
                }
            }
        }
    }
#endif
    printf("Total shaders loaded: %d\n", sNumShaders);
}

/* ------------------------------------------------------------------ */
/* Quad geometry                                                      */
/* ------------------------------------------------------------------ */

static void InitQuad(void)
{
    static const float verts[] = {
        -1.f, -1.f, 0.f, 1.f,   0.f, 1.f, 0.f, 0.f,
         1.f, -1.f, 0.f, 1.f,   1.f, 1.f, 0.f, 0.f,
         1.f,  1.f, 0.f, 1.f,   1.f, 0.f, 0.f, 0.f,
        -1.f, -1.f, 0.f, 1.f,   0.f, 1.f, 0.f, 0.f,
         1.f,  1.f, 0.f, 1.f,   1.f, 0.f, 0.f, 0.f,
        -1.f,  1.f, 0.f, 1.f,   0.f, 0.f, 0.f, 0.f,
    };
    glGenVertexArrays(1, &sQuadVAO);
    glGenBuffers(1, &sQuadVBO);
    glBindVertexArray(sQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(4 * sizeof(float)));
    glBindVertexArray(0);
}

/* ------------------------------------------------------------------ */
/* Game texture                                                       */
/* ------------------------------------------------------------------ */

static void CreateGameTexture(int width, int height)
{
    if (sGameTexture)
        glDeleteTextures(1, &sGameTexture);
    glGenTextures(1, &sGameTexture);
    glBindTexture(GL_TEXTURE_2D, sGameTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    sLastRenderWidth = width;
}

/* ------------------------------------------------------------------ */
/* Crash filter                                                       */
/* ------------------------------------------------------------------ */

#ifdef _WIN32
static LONG CALLBACK VehHandler(EXCEPTION_POINTERS *ep)
{
    if (ep->ExceptionRecord->ExceptionCode != 0xC0000005)
        return EXCEPTION_CONTINUE_SEARCH;
    if (ep->ExceptionRecord->ExceptionInformation[0] > 1)
        return EXCEPTION_CONTINUE_SEARCH;

    return EXCEPTION_CONTINUE_SEARCH;
}

static LONG WINAPI CrashFilter(EXCEPTION_POINTERS *ep)
{

    FILE *f = fopen("crash.log", "w");
    if (f) {
        CONTEXT *c = ep->ContextRecord;
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();

        fprintf(f, "CRASH code=0x%08lX EIP=0x%08lX\n",
                (unsigned long)ep->ExceptionRecord->ExceptionCode,
                (unsigned long)c->Eip);
        fprintf(f, "EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX\n",
                (unsigned long)c->Eax, (unsigned long)c->Ebx,
                (unsigned long)c->Ecx, (unsigned long)c->Edx);
        fprintf(f, "ESP=%08lX EBP=%08lX ESI=%08lX EDI=%08lX\n",
                (unsigned long)c->Esp, (unsigned long)c->Ebp,
                (unsigned long)c->Esi, (unsigned long)c->Edi);

        if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION
            && ep->ExceptionRecord->NumberParameters >= 2) {
            fprintf(f, "Access: %s address 0x%08lX\n",
                    ep->ExceptionRecord->ExceptionInformation[0] ? "WRITE" : "READ",
                    (unsigned long)ep->ExceptionRecord->ExceptionInformation[1]);
        }

        unsigned long *stk = (unsigned long *)c->Esp;
        fprintf(f, "Stack:");
        for (int i = 0; i < 16; i++)
            fprintf(f, " %08lX", stk[i]);
        fprintf(f, "\n");

        SymInitialize(process, NULL, TRUE);
        STACKFRAME frame;
        memset(&frame, 0, sizeof(frame));
        frame.AddrPC.Offset = c->Eip;
        frame.AddrPC.Mode = AddrModeFlat;
        frame.AddrFrame.Offset = c->Ebp;
        frame.AddrFrame.Mode = AddrModeFlat;
        frame.AddrStack.Offset = c->Esp;
        frame.AddrStack.Mode = AddrModeFlat;

        fprintf(f, "\nCall stack:\n");
        for (int i = 0; i < 32; i++) {
            if (!StackWalk(IMAGE_FILE_MACHINE_I386, process, thread,
                           &frame, c, NULL,
                           SymFunctionTableAccess, SymGetModuleBase, NULL))
                break;
            DWORD addr = (DWORD)frame.AddrPC.Offset;
            char symBuf[sizeof(SYMBOL_INFO) + 256];
            SYMBOL_INFO *sym = (SYMBOL_INFO *)symBuf;
            sym->SizeOfStruct = sizeof(SYMBOL_INFO);
            sym->MaxNameLen = 255;
            DWORD disp = 0;
            if (SymFromAddr(process, addr, NULL, sym)) {
                IMAGEHLP_LINE line;
                line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
                if (SymGetLineFromAddr(process, addr, &disp, &line))
                    fprintf(f, "  [%d] 0x%08lX %s (%s:%lu)\n", i, (unsigned long)addr, sym->Name, line.FileName, line.LineNumber);
                else
                    fprintf(f, "  [%d] 0x%08lX %s+0x%lx\n", i, (unsigned long)addr, sym->Name, (unsigned long)(addr - (DWORD)sym->Address));
            } else {
                fprintf(f, "  [%d] 0x%08lX ???\n", i, (unsigned long)addr);
            }
        }
        SymCleanup(process);

        HMODULE hMod = GetModuleHandleA(NULL);
        DWORD slide = (DWORD)hMod - 0x00400000;
        fprintf(f, "\nImage base: 0x%08lX (slide: 0x%08lX)\n",
                (unsigned long)hMod, (unsigned long)slide);
        fprintf(f, "Static EIP: 0x%08lX\n",
                (unsigned long)(c->Eip - slide));
        unsigned long *stk2 = (unsigned long *)c->Esp;
        fprintf(f, "Static stack addrs:");
        for (int i = 0; i < 16; i++) {
            unsigned long val = stk2[i];
            if (val >= (DWORD)hMod && val < (DWORD)hMod + 0x800000)
                fprintf(f, " [%d]=0x%08lX", i, val - slide);
        }
        fprintf(f, "\n");

        fclose(f);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

extern int RunLauncherUI(void);
extern SDL_Window *GetLauncherWindow(void);
extern SDL_GLContext GetLauncherGLContext(void);
extern int rb_RunPokeRuby(void *window, void *glContext, int isSapphire);

int SDL_main(int argc, char **argv)
{
#ifdef _WIN32
    SetUnhandledExceptionFilter(CrashFilter);
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    /* Run the launcher UI to let user pick a game */
    int selectedGame;
game_select:
    selectedGame = RunLauncherUI();

    if (selectedGame < 0)
    {
        SDL_Quit();
        return 0;
    }

    /* Get the window/context the launcher created */
    SDL_Window *win = GetLauncherWindow();
    SDL_GLContext ctx = GetLauncherGLContext();

    /* Fallback: create our own window if launcher returned NULL */
    if (win == NULL)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        win = SDL_CreateWindow("pokefirered",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            DISPLAY_WIDTH * videoScale, DISPLAY_HEIGHT * videoScale,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        ctx = SDL_GL_CreateContext(win);
    }

    /* Set LeafGreen flag based on launcher selection */
    gIsLeafGreen = (selectedGame == 1) ? 1 : 0;

    /* Forward Ruby/Sapphire selections to the Ruby module */
    if (selectedGame == 2 || selectedGame == 3)
    {
        int isSapphire = (selectedGame == 3) ? 1 : 0;
        rb_RunPokeRuby(win, ctx, isSapphire);
        goto game_select;
    }

    /* Forward Gold/Silver/Crystal to the GBC module */
    if (selectedGame == 4)
    {
        extern int gd_RunPokeGold(void*, void*, int);
        gd_RunPokeGold(win, ctx, 0);
        goto game_select;
    }
    if (selectedGame == 5)
    {
        extern int sv_RunPokeGold(void*, void*, int);
        sv_RunPokeGold(win, ctx, 1);
        goto game_select;
    }
    if (selectedGame == 6)
    {
        extern int gd_RunPokeGold(void*, void*, int);
        gd_RunPokeGold(win, ctx, 2);
        goto game_select;
    }

    /* FireRed / LeafGreen path */
    if (!PromptAndLoadRom())
    {
        printf("Cannot start without a valid ROM. Exiting.\n");
#ifdef _WIN32
        MessageBoxA(NULL,
            "Pokemon FireRed ROM required.\n\n"
            "Please provide a legally-owned Pokemon FireRed (US) ROM file (.gba) to extract game assets.",
            "pokefirered", MB_OK | MB_ICONERROR);
#endif
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    ReadSaveFile(gIsLeafGreen ? "pokeleafgreen.sav" : "pokefirered.sav");

#ifdef _WIN32
    /* Reserve address space for ROM mapping */
    VirtualAlloc((void*)0x08000000, 0x02000000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#endif

    sdlWindow = win;
    sdlGLContext = ctx;
    SDL_GL_MakeCurrent(sdlWindow, sdlGLContext);
    SDL_GL_SetSwapInterval(0);

    LoadGLFunctions();

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("OpenGL renderer: %s\n", glGetString(GL_RENDERER));

    /* Initialize GL state */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);

    InitShaders();
    InitQuad();
    CreateGameTexture(DISPLAY_WIDTH, DISPLAY_HEIGHT);

    simTime = curGameTime = lastGameTime = SDL_GetPerformanceCounter();

    isFrameAvailable.value = 0;
    vBlankSemaphore = SDL_CreateSemaphore(0);

    SDL_AudioSpec want;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = 42048;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 1024;
    cgb_audio_init(want.freq);

    {
        SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
        printf("SDL_OpenAudioDevice result=%d\n", dev);
        if (dev == 0)
            printf("Audio FAILED: %s\n", SDL_GetError());
        else {
            printf("Audio OK dev=%d: freq=%d format=%d channels=%d\n", dev, want.freq, want.format, want.channels);
            SDL_PauseAudioDevice(dev, 0);
        }
        fflush(stdout);
        sAudioDeviceId = dev;
    }

    REG_KEYINPUT = KEYS_MASK;

    ImGuiBridge_Init(sdlWindow, sdlGLContext);

    VDraw();
    mainLoopThread = SDL_CreateThread(DoMain, "AgbMain", NULL);

    double accumulator = 0.0;

    memset(&internalClock, 0, sizeof(internalClock));
    internalClock.status = SIIRTCINFO_24HOUR;
    UpdateInternalClock();

    while (isRunning)
    {
        ProcessEvents();

        if (!paused)
        {
            double dt = fixedTimestep / timeScale;

            curGameTime = SDL_GetPerformanceCounter();
            double deltaTime = (double)((curGameTime - lastGameTime) / (double)SDL_GetPerformanceFrequency());
            if (deltaTime > (dt * 5))
                deltaTime = dt;
            lastGameTime = curGameTime;

            accumulator += deltaTime;

            while (accumulator >= dt)
            {
                REG_DISPSTAT |= INTR_FLAG_VBLANK;
                INTR_CHECK |= INTR_FLAG_VBLANK;
                gMain.intrCheck |= INTR_FLAG_VBLANK;

                VDraw();

                if (SDL_AtomicGet(&isFrameAvailable))
                {
                    SDL_AtomicSet(&isFrameAvailable, 0);
                    SDL_SemPost(vBlankSemaphore);
                }

                REG_DISPSTAT &= ~INTR_FLAG_VBLANK;

                accumulator -= dt;
                if (accumulator > dt * 4)
                    accumulator = dt;
            }

            /* --- OpenGL rendering (once per visual frame) --- */
            {
                int ww, wh;
                SDL_GetWindowSize(sdlWindow, &ww, &wh);
                glViewport(0, 0, ww, wh);
                glClear(GL_COLOR_BUFFER_BIT);

                int renderWidth = GetRenderWidth();
                int menuH = ImGuiBridge_IsOverlayVisible() ? ImGuiBridge_GetMenuBarHeight() : 0;
                int availH = wh - menuH;
                float scaleByW = (float)ww / renderWidth;
                float scaleByH = (float)availH / DISPLAY_HEIGHT;
                float fitScale = scaleByW < scaleByH ? scaleByW : scaleByH;
                int tw = (int)(renderWidth * fitScale);
                int th = (int)(DISPLAY_HEIGHT * fitScale);
                int vx = (ww - tw) / 2;
                int vy = wh - menuH - (availH + th) / 2;
                sGameViewportX = vx; sGameViewportY = vy;
                sGameViewportW = tw; sGameViewportH = th;
                glViewport(vx, vy, tw, th);

                int selectedShader = ImGuiBridge_GetSelectedShader();
                if (selectedShader < 0 || selectedShader >= sNumShaders)
                    selectedShader = 0;
                sActiveShader = selectedShader;

                glDisable(GL_BLEND);
                RenderShaderEntry(&sShaders[sActiveShader], tw, th, renderWidth, DISPLAY_HEIGHT);

                glViewport(0, 0, ww, wh);
            }
        }

        {
            extern void SetWidescreenEnabled(int);
            extern void SetDesiredRenderWidth(int);
            struct RenderConfig *cfg = ImGuiBridge_GetConfig();

            SetWidescreenEnabled(cfg->widescreen);
            if (cfg->widescreen) {
                SetDesiredRenderWidth(DISPLAY_HEIGHT * 16 / 9);
            }

            static int sLastResIndex = -1;
            if (cfg->resolution_index != sLastResIndex) {
                int rw, rh;
                ImGuiBridge_GetResolution(&rw, &rh);
                videoScale = rh / DISPLAY_HEIGHT;
                if (videoScale < 1) videoScale = 1;
                if (cfg->fullscreen) {
                    SDL_SetWindowSize(sdlWindow, rw, rh);
                    SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
                } else {
                    SDL_SetWindowSize(sdlWindow, rw, rh);
                }
                sLastResIndex = cfg->resolution_index;
            }

            static int sLastFullscreen = -1;
            if (cfg->fullscreen != sLastFullscreen) {
                if (cfg->fullscreen) {
                    int rw, rh;
                    ImGuiBridge_GetResolution(&rw, &rh);
                    SDL_SetWindowSize(sdlWindow, rw, rh);
                    SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
                } else {
                    SDL_SetWindowFullscreen(sdlWindow, 0);
                    int rw, rh;
                    ImGuiBridge_GetResolution(&rw, &rh);
                    SDL_SetWindowSize(sdlWindow, rw, rh);
                    SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                }
                sLastFullscreen = cfg->fullscreen;
            }
        }

        ImGuiBridge_NewFrame();
        ImGuiBridge_Render();
        ImGuiBridge_SyncShinyCharm();
        /* NetClient_Poll moved to game thread (main.c) */

        if (ImGuiBridge_ShouldQuit())
        {
            isRunning = false;
            /* Kill game thread and return to launcher */
            for (int i = 0; i < 10; i++)
                SDL_SemPost(vBlankSemaphore);
            SDL_DetachThread(mainLoopThread);
            mainLoopThread = NULL;
            ImGuiBridge_Shutdown();
            CloseSaveFile();
            if (sAudioDeviceId > 0) { SDL_CloseAudioDevice(sAudioDeviceId); sAudioDeviceId = 0; }
            if (vBlankSemaphore) { SDL_DestroySemaphore(vBlankSemaphore); vBlankSemaphore = NULL; }
            goto game_select;
        }

        SDL_GL_SwapWindow(sdlWindow);
    }

    /* Unblock game thread until it sees isRunning==false and exits */
    for (int i = 0; i < 10; i++)
        SDL_SemPost(vBlankSemaphore);
    SDL_WaitThread(mainLoopThread, NULL);
    mainLoopThread = NULL;

    ImGuiBridge_Shutdown();
    CloseSaveFile();
    if (sAudioDeviceId > 0) { SDL_CloseAudioDevice(sAudioDeviceId); sAudioDeviceId = 0; }

    if (vBlankSemaphore) { SDL_DestroySemaphore(vBlankSemaphore); vBlankSemaphore = NULL; }

    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    sdlWindow = NULL;
    sdlGLContext = NULL;
    return 0;
}

static void ReadSaveFile(char *path)
{
    // Check whether the saveFile exists, and create it if not
    sSaveFile = fopen(path, "r+b");
    if (sSaveFile == NULL)
    {
        sSaveFile = fopen(path, "w+b");
    }

    fseek(sSaveFile, 0, SEEK_END);
    int fileSize = ftell(sSaveFile);
    fseek(sSaveFile, 0, SEEK_SET);

    // Only read as many bytes as fit inside the buffer
    // or as many bytes as are in the file
    int bytesToRead = (fileSize < FLASH_ROM_SIZE_1M) ? fileSize : FLASH_ROM_SIZE_1M;

    int bytesRead = fread(FLASH_BASE, 1, bytesToRead, sSaveFile);

    // Fill the buffer if the savefile was just created or smaller than the buffer itself
    for (int i = bytesRead; i < FLASH_ROM_SIZE_1M; i++)
    {
        FLASH_BASE[i] = 0xFF;
    }
}

static void StoreSaveFile()
{
    if (sSaveFile != NULL)
    {
        fseek(sSaveFile, 0, SEEK_SET);
        fwrite(FLASH_BASE, 1, FLASH_ROM_SIZE_1M, sSaveFile);
    }
}

void Platform_WriteSaveToDisk(void)
{
    StoreSaveFile();
}

static int sSaveUploadPending = 0;

void Platform_StoreSaveFile(void)
{
    extern int NetClient_IsConnected(void);
    if (NetClient_IsConnected())
        sSaveUploadPending = 10;
    else
        StoreSaveFile();
}

void Platform_FlushSaveToServer(void)
{
    if (sSaveUploadPending <= 0) return;
    if (--sSaveUploadPending > 0) return;
    sSaveUploadPending = 0; // prevent re-trigger

    extern int NetClient_IsConnected(void);
    extern int NetClient_SendRaw(const void *data, int size);
    if (!NetClient_IsConnected()) return;

    int totalChunks = (FLASH_ROM_SIZE_1M + 4096 - 1) / 4096;
    for (int i = 0; i < totalChunks; i++)
    {
        struct {
            uint8_t type;
            uint16_t size;
            uint16_t chunkIndex;
            uint16_t totalChunks;
            uint16_t dataSize;
            uint8_t data[4096];
        } __attribute__((packed)) pkt;
        pkt.type = 0x0F;
        int remaining = FLASH_ROM_SIZE_1M - i * 4096;
        int chunkSize = remaining < 4096 ? remaining : 4096;
        pkt.chunkIndex = i;
        pkt.totalChunks = totalChunks;
        pkt.dataSize = chunkSize;
        memcpy(pkt.data, &FLASH_BASE[i * 4096], chunkSize);
        pkt.size = 3 + 2 + 2 + 2 + chunkSize;
        NetClient_SendRaw(&pkt, pkt.size);
    }
    printf("[NET] Save uploaded to server (%d chunks)\n", totalChunks); fflush(stdout);
}

void Platform_ReadFlash(u16 sectorNum, u32 offset, u8 *dest, u32 size)
{
    u32 flashOffset = (sectorNum << gFlash->sector.shift) + offset;
    if (flashOffset + size <= FLASH_ROM_SIZE_1M)
        memcpy(dest, &FLASH_BASE[flashOffset], size);
    else
        memset(dest, 0xFF, size);
}

void Platform_QueueAudio(float *audioBuffer, s32 samplesPerFrame)
{
    static int sCallCount = 0;
    if (sCallCount++ < 3) { printf("[QueueAudio] called #%d samples=%d buf[0]=%f\n", sCallCount, samplesPerFrame, audioBuffer ? audioBuffer[0] : -999.0f); fflush(stdout); }
    float vol = ImGuiBridge_GetConfig()->volume;
    if (vol < 1.0f)
    {
        int numSamples = samplesPerFrame / sizeof(float);
        for (int i = 0; i < numSamples; i++)
            audioBuffer[i] *= vol;
    }
    if (sAudioDeviceId > 0)
        SDL_QueueAudio(sAudioDeviceId, audioBuffer, samplesPerFrame);
}


static void CloseSaveFile()
{
    if (sSaveFile != NULL)
    {
        fclose(sSaveFile);
    }
}

static u16 keys;
static u16 GetGameControllerKeys(void);
static u16 GetJoystickKeys(void);

static const u16 sGbaButtonBits[] = {
    A_BUTTON, B_BUTTON, START_BUTTON, SELECT_BUTTON,
    L_BUTTON, R_BUTTON,
    DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT,
    0 // speed up has no GBA bit
};

void ProcessEvents(void)
{
    SDL_Event event;
    struct RenderConfig *cfg = ImGuiBridge_GetConfig();

    while (SDL_PollEvent(&event))
    {
        if (ImGuiBridge_ProcessEvent(&event))
            continue;

        switch (event.type)
        {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYUP:
        {
            SDL_Keycode sym = event.key.keysym.sym;
            for (int i = 0; i < 11; i++) {
                if (cfg->kb_binds[i] == (int)sym) {
                    if (i < 10)
                        keys &= ~sGbaButtonBits[i];
                    else if (speedUp) {
                        speedUp = false;
                        timeScale = 1.0;
                        SDL_ClearQueuedAudio(sAudioDeviceId);
                        SDL_PauseAudioDevice(sAudioDeviceId, 0);
                        Mix_ResumeMusic();
                    }
                }
            }
            break;
        }
        case SDL_KEYDOWN:
        {
            SDL_Keycode sym = event.key.keysym.sym;
            for (int i = 0; i < 11; i++) {
                if (cfg->kb_binds[i] == (int)sym) {
                    if (i < 10)
                        keys |= sGbaButtonBits[i];
                    else if (!speedUp) {
                        speedUp = true;
                        timeScale = (double)cfg->speed_multiplier;
                        SDL_PauseAudioDevice(sAudioDeviceId, 1);
                        Mix_PauseMusic();
                    }
                }
            }
            if (sym == SDLK_r && (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)))
                DoSoftReset();
            if (sym == SDLK_p && (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)))
                paused = !paused;
            break;
        }
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                unsigned int h = event.window.data2;
                videoScale = h / DISPLAY_HEIGHT;
                if (videoScale < 1) videoScale = 1;
            }
            break;
        }
    }
    {
        u16 gpKeys = GetGameControllerKeys();
        REG_KEYINPUT = KEYS_MASK & ~(keys | gpKeys);
    }
}

#define STICK_THRESHOLD 16000
#define GP_RTRIGGER_SENTINEL 0xF001

static SDL_GameController *sGameController = NULL;
static SDL_Joystick *sJoystick = NULL;
static bool sUsingJoystickFallback = false;

void Platform_OpenGameController(int deviceIndex)
{
    if (sGameController) { SDL_GameControllerClose(sGameController); sGameController = NULL; }
    if (sJoystick) { SDL_JoystickClose(sJoystick); sJoystick = NULL; }
    sUsingJoystickFallback = false;

    if (deviceIndex < 0) return;

    if (SDL_IsGameController(deviceIndex)) {
        sGameController = SDL_GameControllerOpen(deviceIndex);
        if (sGameController)
            printf("[CTRL] Opened GameController: %s\n", SDL_GameControllerName(sGameController));
    } else {
        sJoystick = SDL_JoystickOpen(deviceIndex);
        sUsingJoystickFallback = true;
        if (sJoystick)
            printf("[CTRL] Opened Joystick fallback: %s (axes=%d buttons=%d)\n",
                SDL_JoystickName(sJoystick), SDL_JoystickNumAxes(sJoystick), SDL_JoystickNumButtons(sJoystick));
    }
    fflush(stdout);
}

int Platform_GetGameControllerCount(void)
{
    return SDL_NumJoysticks();
}

const char *Platform_GetGameControllerName(int idx)
{
    if (idx < SDL_NumJoysticks()) {
        const char *name = SDL_IsGameController(idx)
            ? SDL_GameControllerNameForIndex(idx)
            : SDL_JoystickNameForIndex(idx);
        return name ? name : "Unknown";
    }
    return "Unknown";
}

static u16 GetJoystickKeys(void)
{
    u16 gpKeys = 0;
    if (!sJoystick) return 0;
    if (!SDL_JoystickGetAttached(sJoystick)) {
        SDL_JoystickClose(sJoystick);
        sJoystick = NULL;
        return 0;
    }

    SDL_JoystickUpdate();
    int numButtons = SDL_JoystickNumButtons(sJoystick);
    int numAxes = SDL_JoystickNumAxes(sJoystick);

    // Debug: print raw state when any button pressed
    {
        static int sJoyDbg = 0;
        if (sJoyDbg < 10) {
            for (int b = 0; b < numButtons; b++) {
                if (SDL_JoystickGetButton(sJoystick, b)) {
                    printf("[JOY] Button %d pressed (of %d total). Axes: ", b, numButtons);
                    for (int a = 0; a < numAxes && a < 6; a++)
                        printf("ax%d=%d ", a, SDL_JoystickGetAxis(sJoystick, a));
                    printf("\n"); fflush(stdout);
                    sJoyDbg++;
                    break;
                }
            }
        }
    }

    // Standard mapping: buttons 0=A, 1=B, 2=X, 3=Y, 4=LB, 5=RB, 6=Back, 7=Start
    // Varies by controller but this covers most generic ones
    if (numButtons > 0 && SDL_JoystickGetButton(sJoystick, 0)) gpKeys |= A_BUTTON;
    if (numButtons > 1 && SDL_JoystickGetButton(sJoystick, 1)) gpKeys |= B_BUTTON;
    if (numButtons > 2 && SDL_JoystickGetButton(sJoystick, 2)) gpKeys |= A_BUTTON; // X as alt-A
    if (numButtons > 3 && SDL_JoystickGetButton(sJoystick, 3)) gpKeys |= B_BUTTON; // Y as alt-B
    if (numButtons > 4 && SDL_JoystickGetButton(sJoystick, 4)) gpKeys |= L_BUTTON;
    if (numButtons > 5 && SDL_JoystickGetButton(sJoystick, 5)) gpKeys |= R_BUTTON;
    if (numButtons > 6 && SDL_JoystickGetButton(sJoystick, 6)) gpKeys |= SELECT_BUTTON;
    if (numButtons > 7 && SDL_JoystickGetButton(sJoystick, 7)) gpKeys |= START_BUTTON;

    // D-pad as hat
    if (SDL_JoystickNumHats(sJoystick) > 0) {
        Uint8 hat = SDL_JoystickGetHat(sJoystick, 0);
        if (hat & SDL_HAT_UP)    gpKeys |= DPAD_UP;
        if (hat & SDL_HAT_DOWN)  gpKeys |= DPAD_DOWN;
        if (hat & SDL_HAT_LEFT)  gpKeys |= DPAD_LEFT;
        if (hat & SDL_HAT_RIGHT) gpKeys |= DPAD_RIGHT;
    }

    // Left stick
    if (numAxes >= 2) {
        int lx = SDL_JoystickGetAxis(sJoystick, 0);
        int ly = SDL_JoystickGetAxis(sJoystick, 1);
        if (lx < -STICK_THRESHOLD) gpKeys |= DPAD_LEFT;
        if (lx >  STICK_THRESHOLD) gpKeys |= DPAD_RIGHT;
        if (ly < -STICK_THRESHOLD) gpKeys |= DPAD_UP;
        if (ly >  STICK_THRESHOLD) gpKeys |= DPAD_DOWN;
    }

    return gpKeys;
}

int Platform_PollControllerButton(void)
{
    if (!sGameController) return -1;
    if (!SDL_GameControllerGetAttached(sGameController)) return -1;

    SDL_GameControllerUpdate();

    struct { int sdlBtn; int mask; } map[] = {
        { SDL_CONTROLLER_BUTTON_A,             0x1000 },
        { SDL_CONTROLLER_BUTTON_B,             0x2000 },
        { SDL_CONTROLLER_BUTTON_X,             0x4000 },
        { SDL_CONTROLLER_BUTTON_Y,             0x8000 },
        { SDL_CONTROLLER_BUTTON_START,         0x0010 },
        { SDL_CONTROLLER_BUTTON_BACK,          0x0020 },
        { SDL_CONTROLLER_BUTTON_LEFTSHOULDER,  0x0100 },
        { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, 0x0200 },
        { SDL_CONTROLLER_BUTTON_DPAD_UP,       0x0001 },
        { SDL_CONTROLLER_BUTTON_DPAD_DOWN,     0x0002 },
        { SDL_CONTROLLER_BUTTON_DPAD_LEFT,     0x0004 },
        { SDL_CONTROLLER_BUTTON_DPAD_RIGHT,    0x0008 },
    };
    for (int i = 0; i < (int)(sizeof(map)/sizeof(map[0])); i++)
        if (SDL_GameControllerGetButton(sGameController, map[i].sdlBtn))
            return map[i].mask;

    if (SDL_GameControllerGetAxis(sGameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000)
        return 0xF001;

    return -1;
}

static u16 GetGameControllerKeys(void)
{
    u16 gpKeys = 0;

    // Auto-open first available device
    if (!sGameController && !sJoystick) {
        static int sEnumPrinted = 0;
        if (!sEnumPrinted) {
            int nj = SDL_NumJoysticks();
            printf("[CTRL] SDL_NumJoysticks = %d\n", nj);
            for (int i = 0; i < nj; i++)
                printf("[CTRL]   [%d] name='%s' isGameCtrl=%d\n", i, SDL_JoystickNameForIndex(i), SDL_IsGameController(i));
            fflush(stdout);
            sEnumPrinted = 1;
        }
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            Platform_OpenGameController(i);
            if (sGameController || sJoystick) break;
        }
        if (!sGameController && !sJoystick) return 0;
    }

    // Joystick fallback path
    if (sUsingJoystickFallback)
        return GetJoystickKeys();

    // GameController path
    if (!sGameController) return 0;
    if (!SDL_GameControllerGetAttached(sGameController)) {
        SDL_GameControllerClose(sGameController);
        sGameController = NULL;
        return 0;
    }

    SDL_GameControllerUpdate();

    // Debug GameController buttons
    {
        static int sGcDbg = 0;
        if (sGcDbg < 5) {
            for (int b = 0; b <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT; b++) {
                if (SDL_GameControllerGetButton(sGameController, b)) {
                    printf("[GC] Button %d (%s) pressed\n", b, SDL_GameControllerGetStringForButton(b));
                    fflush(stdout);
                    sGcDbg++;
                    break;
                }
            }
        }
    }

    struct RenderConfig *cfg = ImGuiBridge_GetConfig();

    struct { int sdlBtn; int mask; } btnMap[] = {
        { SDL_CONTROLLER_BUTTON_A,             0x1000 },
        { SDL_CONTROLLER_BUTTON_B,             0x2000 },
        { SDL_CONTROLLER_BUTTON_X,             0x4000 },
        { SDL_CONTROLLER_BUTTON_Y,             0x8000 },
        { SDL_CONTROLLER_BUTTON_START,         0x0010 },
        { SDL_CONTROLLER_BUTTON_BACK,          0x0020 },
        { SDL_CONTROLLER_BUTTON_LEFTSHOULDER,  0x0100 },
        { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, 0x0200 },
        { SDL_CONTROLLER_BUTTON_DPAD_UP,       0x0001 },
        { SDL_CONTROLLER_BUTTON_DPAD_DOWN,     0x0002 },
        { SDL_CONTROLLER_BUTTON_DPAD_LEFT,     0x0004 },
        { SDL_CONTROLLER_BUTTON_DPAD_RIGHT,    0x0008 },
    };
    int numBtns = sizeof(btnMap) / sizeof(btnMap[0]);

    bool rtPressed = SDL_GameControllerGetAxis(sGameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000;

    for (int i = 0; i < 11; i++) {
        int gpBtn = cfg->gp_binds[i];
        bool pressed = false;
        if (gpBtn == GP_RTRIGGER_SENTINEL)
            pressed = rtPressed;
        else {
            for (int b = 0; b < numBtns; b++)
                if (btnMap[b].mask == gpBtn)
                    pressed = SDL_GameControllerGetButton(sGameController, btnMap[b].sdlBtn);
        }
        if (pressed && i < 10)
            gpKeys |= sGbaButtonBits[i];
    }

    int lx = SDL_GameControllerGetAxis(sGameController, SDL_CONTROLLER_AXIS_LEFTX);
    int ly = SDL_GameControllerGetAxis(sGameController, SDL_CONTROLLER_AXIS_LEFTY);
    if (lx < -STICK_THRESHOLD) gpKeys |= DPAD_LEFT;
    if (lx >  STICK_THRESHOLD) gpKeys |= DPAD_RIGHT;
    if (ly < -STICK_THRESHOLD) gpKeys |= DPAD_UP;
    if (ly >  STICK_THRESHOLD) gpKeys |= DPAD_DOWN;

    bool gpSpeedUp = false;
    int speedBtn = cfg->gp_binds[10];
    if (speedBtn == GP_RTRIGGER_SENTINEL)
        gpSpeedUp = rtPressed;
    else {
        for (int b = 0; b < numBtns; b++)
            if (btnMap[b].mask == speedBtn)
                gpSpeedUp = SDL_GameControllerGetButton(sGameController, btnMap[b].sdlBtn);
    }

    double oldTimeScale = timeScale;
    timeScale = (gpSpeedUp || speedUp) ? (double)cfg->speed_multiplier : 1.0;
    if (oldTimeScale != timeScale) {
        if (timeScale > 1.0)
            SDL_PauseAudioDevice(sAudioDeviceId, 1);
        else {
            SDL_ClearQueuedAudio(sAudioDeviceId);
            SDL_PauseAudioDevice(sAudioDeviceId, 0);
        }
    }

    return gpKeys;
}

u16 Platform_GetKeyInput(void)
{
    u16 gamepadKeys = GetGameControllerKeys();
    return (gamepadKeys != 0) ? gamepadKeys : keys;
}

static uint16_t sColorLUT_Higan[32768];
static uint16_t sColorLUT_Libretro[32768];
static bool sColorLUTBuilt = false;

static void BuildColorLUTs(void)
{
    if (sColorLUTBuilt) return;

    for (int i = 0; i < 32768; i++)
    {
        int r5 = (i >> 0) & 31;
        int g5 = (i >> 5) & 31;
        int b5 = (i >> 10) & 31;

        // higan/byuu GBA color correction
        {
            double lr = pow(r5 / 31.0, 4.0);
            double lg = pow(g5 / 31.0, 4.0);
            double lb = pow(b5 / 31.0, 4.0);
            double invGamma = 1.0 / 2.2;
            double scale = 255.0 / 280.0;
            int ro = (int)(pow((255*lr + 50*lg +  0*lb) / 255.0, invGamma) * 31.0 * scale);
            int go = (int)(pow(( 10*lr +230*lg + 30*lb) / 255.0, invGamma) * 31.0 * scale);
            int bo = (int)(pow(( 50*lr + 10*lg +220*lb) / 255.0, invGamma) * 31.0 * scale);
            if (ro > 31) ro = 31; if (go > 31) go = 31; if (bo > 31) bo = 31;
            if (ro < 0) ro = 0; if (go < 0) go = 0; if (bo < 0) bo = 0;
            sColorLUT_Higan[i] = ro | (go << 5) | (bo << 10);
        }

        // libretro-style GBA color correction
        {
            double sr = pow(r5 / 31.0, 2.7);
            double sg = pow(g5 / 31.0, 2.7);
            double sb = pow(b5 / 31.0, 2.7);
            double invGamma = 1.0 / 2.2;
            double ro_f = 0.845*sr + 0.09*sg + 0.16*sb;
            double go_f = 0.17*sr + 0.68*sg + 0.085*sb;
            double bo_f = -0.015*sr + 0.23*sg + 0.755*sb;
            if (ro_f < 0) ro_f = 0; if (go_f < 0) go_f = 0; if (bo_f < 0) bo_f = 0;
            if (ro_f > 1) ro_f = 1; if (go_f > 1) go_f = 1; if (bo_f > 1) bo_f = 1;
            int ro = (int)(pow(ro_f * 0.94, invGamma) * 31.0);
            int go = (int)(pow(go_f * 0.94, invGamma) * 31.0);
            int bo = (int)(pow(bo_f * 0.94, invGamma) * 31.0);
            if (ro > 31) ro = 31; if (go > 31) go = 31; if (bo > 31) bo = 31;
            sColorLUT_Libretro[i] = ro | (go << 5) | (bo << 10);
        }
    }
    sColorLUTBuilt = true;
}

static void ApplyColorCorrection(uint16_t *image, int numPixels, int shader)
{
    const uint16_t *lut = (shader == 1) ? sColorLUT_Higan : sColorLUT_Libretro;
    for (int i = 0; i < numPixels; i++)
    {
        uint16_t px = image[i];
        image[i] = lut[px & 0x7FFF] | (px & 0x8000);
    }
}

void VDraw(void)
{
    static uint16_t image[400 * DISPLAY_HEIGHT];

    int renderWidth = GetRenderWidth();
    int totalPixels = renderWidth * DISPLAY_HEIGHT;

    memset(image, 0, totalPixels * sizeof(uint16_t));
    DrawFrame(image);

    int colorShader = ImGuiBridge_GetConfig()->color_shader;
    if (colorShader > 0)
    {
        BuildColorLUTs();
        ApplyColorCorrection(image, totalPixels, colorShader);
    }

    /* Recreate texture if render width changed (widescreen toggle) */
    if (renderWidth != sLastRenderWidth)
    {
        CreateGameTexture(renderWidth, DISPLAY_HEIGHT);
    }

    /* Upload pixel data to GL texture */
    glBindTexture(GL_TEXTURE_2D, sGameTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderWidth, DISPLAY_HEIGHT,
                    GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, image);
    glBindTexture(GL_TEXTURE_2D, 0);

    REG_VCOUNT = 161;
}

static LONG CALLBACK DivZeroHandler(EXCEPTION_POINTERS *ep)
{
    if (ep->ExceptionRecord->ExceptionCode == 0xC0000094) {
        ep->ContextRecord->Eax = 0;
        ep->ContextRecord->Edx = 0;
        ep->ContextRecord->Eip += 2;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (ep->ExceptionRecord->ExceptionCode == 0xC0000005) {
        DWORD addr = (DWORD)ep->ExceptionRecord->ExceptionInformation[1];
        if (addr < 0x10000 || addr >= 0xFFFF0000) {
            // Skip the faulting instruction — decode length for common MOV patterns
            u8 *ip = (u8 *)ep->ContextRecord->Eip;
            int skip = 2;
            int i = 0;
            while (ip[i] == 0x66 || ip[i] == 0x67 || ip[i] == 0xF2 || ip[i] == 0xF3 ||
                   ip[i] == 0x26 || ip[i] == 0x2E || ip[i] == 0x36 || ip[i] == 0x3E ||
                   ip[i] == 0x64 || ip[i] == 0x65) i++;
            u8 opcode = ip[i]; i++;
            if (opcode == 0x8B || opcode == 0x8A || opcode == 0x0F) {
                if (opcode == 0x0F) { i++; } // 2-byte opcode
                u8 modrm = ip[i]; i++;
                u8 mod = (modrm >> 6) & 3;
                u8 rm = modrm & 7;
                if (mod == 0 && rm == 5) i += 4;
                else if (mod == 0 && rm == 4) i++;
                else if (mod == 1) { if (rm == 4) i++; i++; }
                else if (mod == 2) { if (rm == 4) i++; i += 4; }
                skip = i;
            }
            ep->ContextRecord->Eax = 0;
            ep->ContextRecord->Eip += skip;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static LONG WINAPI DllCrashFilter(EXCEPTION_POINTERS *ep)
{
    CONTEXT *c = ep->ContextRecord;
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    FILE *f = fopen("crash.log", "w");
    if (f) {
        fprintf(f, "CRASH code=0x%08lX EIP=0x%08lX\n",
            (unsigned long)ep->ExceptionRecord->ExceptionCode,
            (unsigned long)c->Eip);
        fprintf(f, "EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX\n",
            (unsigned long)c->Eax, (unsigned long)c->Ebx,
            (unsigned long)c->Ecx, (unsigned long)c->Edx);
        fprintf(f, "ESP=%08lX EBP=%08lX ESI=%08lX EDI=%08lX\n",
            (unsigned long)c->Esp, (unsigned long)c->Ebp,
            (unsigned long)c->Esi, (unsigned long)c->Edi);
        if (ep->ExceptionRecord->ExceptionCode == 0xC0000005 && ep->ExceptionRecord->NumberParameters >= 2)
            fprintf(f, "Access: %s address 0x%08lX\n",
                ep->ExceptionRecord->ExceptionInformation[0] ? "WRITE" : "READ",
                (unsigned long)ep->ExceptionRecord->ExceptionInformation[1]);

        unsigned long *stk = (unsigned long *)c->Esp;
        fprintf(f, "Stack:");
        for (int i = 0; i < 32; i++)
            fprintf(f, " %08lX", stk[i]);
        fprintf(f, "\n");

        SymInitialize(process, NULL, TRUE);
        STACKFRAME frame;
        memset(&frame, 0, sizeof(frame));
        frame.AddrPC.Offset = c->Eip;
        frame.AddrPC.Mode = AddrModeFlat;
        frame.AddrFrame.Offset = c->Ebp;
        frame.AddrFrame.Mode = AddrModeFlat;
        frame.AddrStack.Offset = c->Esp;
        frame.AddrStack.Mode = AddrModeFlat;

        fprintf(f, "\nCall stack:\n");
        for (int i = 0; i < 32; i++) {
            if (!StackWalk(IMAGE_FILE_MACHINE_I386, process, thread,
                           &frame, c, NULL,
                           SymFunctionTableAccess, SymGetModuleBase, NULL))
                break;
            DWORD addr = (DWORD)frame.AddrPC.Offset;
            char symBuf[sizeof(SYMBOL_INFO) + 256];
            SYMBOL_INFO *sym = (SYMBOL_INFO *)symBuf;
            sym->SizeOfStruct = sizeof(SYMBOL_INFO);
            sym->MaxNameLen = 255;
            DWORD disp = 0;
            if (SymFromAddr(process, addr, NULL, sym))
                fprintf(f, "  [%d] 0x%08lX %s\n", i, (unsigned long)addr, sym->Name);
            else
                fprintf(f, "  [%d] 0x%08lX ???\n", i, (unsigned long)addr);
        }
        SymCleanup(process);
        fclose(f);
    }
    printf("[DoMain] CRASH: 0x%08lX at EIP=0x%08lX\n",
        (unsigned long)ep->ExceptionRecord->ExceptionCode,
        (unsigned long)c->Eip);
    fflush(stdout);
    isRunning = false;
    return EXCEPTION_EXECUTE_HANDLER;
}

int DoMain(void *data)
{
    extern void InstallCrashGuard(void);
    InstallCrashGuard();
    AddVectoredExceptionHandler(1, DivZeroHandler);
    SetUnhandledExceptionFilter(DllCrashFilter);
    AgbMain();
    return 0;
}

extern void m4aSoundMain(void);
extern void CopyBufferedValuesToGpuRegs(void);
extern void ProcessDma3Requests(void);

void VBlankIntrWait(void)
{
    if (!isRunning) return;

    if (gIntrTable[4])
        gIntrTable[4]();

    SDL_AtomicSet(&isFrameAvailable, 1);
    SDL_SemWait(vBlankSemaphore);
}

u8 BinToBcd(u8 bin)
{
    int placeCounter = 1;
    u8 out = 0;
    do
    {
        out |= (bin % 10) * placeCounter;
        placeCounter *= 16;
    }
    while ((bin /= 10) > 0);

    return out;
}

void Platform_GetStatus(struct SiiRtcInfo *rtc)
{
    rtc->status = internalClock.status;
}

void Platform_SetStatus(struct SiiRtcInfo *rtc)
{
    internalClock.status = rtc->status;
}

static void UpdateInternalClock(void)
{
    time_t rawTime = time(NULL);
    struct tm *time = localtime(&rawTime);

    internalClock.year = BinToBcd(time->tm_year - 100);
    internalClock.month = BinToBcd(time->tm_mon + 1);
    internalClock.day = BinToBcd(time->tm_mday);
    internalClock.dayOfWeek = BinToBcd(time->tm_wday);
    internalClock.hour = BinToBcd(time->tm_hour);
    internalClock.minute = BinToBcd(time->tm_min);
    internalClock.second = BinToBcd(time->tm_sec);
}

void Platform_GetDateTime(struct SiiRtcInfo *rtc)
{
    UpdateInternalClock();

    rtc->year = internalClock.year;
    rtc->month = internalClock.month;
    rtc->day = internalClock.day;
    rtc->dayOfWeek = internalClock.dayOfWeek;
    rtc->hour = internalClock.hour;
    rtc->minute = internalClock.minute;
    rtc->second = internalClock.second;
    DBGPRINTF("GetDateTime: %d-%02d-%02d %02d:%02d:%02d\n", ConvertBcdToBinary(rtc->year),
                                                         ConvertBcdToBinary(rtc->month),
                                                         ConvertBcdToBinary(rtc->day),
                                                         ConvertBcdToBinary(rtc->hour),
                                                         ConvertBcdToBinary(rtc->minute),
                                                         ConvertBcdToBinary(rtc->second));
}

void Platform_SetDateTime(struct SiiRtcInfo *rtc)
{
    internalClock.month = rtc->month;
    internalClock.day = rtc->day;
    internalClock.dayOfWeek = rtc->dayOfWeek;
    internalClock.hour = rtc->hour;
    internalClock.minute = rtc->minute;
    internalClock.second = rtc->second;
}

void Platform_GetTime(struct SiiRtcInfo *rtc)
{
    UpdateInternalClock();

    rtc->hour = internalClock.hour;
    rtc->minute = internalClock.minute;
    rtc->second = internalClock.second;
    DBGPRINTF("GetTime: %02d:%02d:%02d\n", ConvertBcdToBinary(rtc->hour),
                                        ConvertBcdToBinary(rtc->minute),
                                        ConvertBcdToBinary(rtc->second));
}

void Platform_SetTime(struct SiiRtcInfo *rtc)
{
    internalClock.hour = rtc->hour;
    internalClock.minute = rtc->minute;
    internalClock.second = rtc->second;
}

void Platform_SetAlarm(u8 *alarmData)
{
    // TODO
}

void SoftReset(u32 resetFlags)
{
    puts("Soft Reset called. Exiting.");
    exit(0);
}

int Platform_GetShaderCount(void)
{
    return sNumShaders;
}

const char *Platform_GetShaderName(int idx)
{
    if (idx < 0 || idx >= sNumShaders) return "None";
    return sShaders[idx].name;
}

// ---- Music Override System ----
static int sMixerInitialized = 0;
static Mix_Music *sCurrentOverrideMusic = NULL;

void Platform_PlayMusicOverride(const char *path)
{
    if (!sMixerInitialized)
    {
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
        sMixerInitialized = 1;
    }

    if (sCurrentOverrideMusic)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(sCurrentOverrideMusic);
        sCurrentOverrideMusic = NULL;
    }

    sCurrentOverrideMusic = Mix_LoadMUS(path);
    if (sCurrentOverrideMusic)
    {
        Mix_PlayMusic(sCurrentOverrideMusic, -1);
        // Mute M4A output while override plays
        SDL_PauseAudioDevice(sAudioDeviceId, 1);
    }
}

void Platform_StopMusicOverride(void)
{
    if (sCurrentOverrideMusic)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(sCurrentOverrideMusic);
        sCurrentOverrideMusic = NULL;
        SDL_PauseAudioDevice(sAudioDeviceId, 0);
    }
}

int Platform_IsMusicOverridePlaying(void)
{
    return sCurrentOverrideMusic != NULL && Mix_PlayingMusic();
}

#endif
