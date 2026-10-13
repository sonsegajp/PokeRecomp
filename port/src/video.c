/* video.c — window and presentation through OpenGL 3.3.
 *
 * The frame (GBA screen, possibly widened) is uploaded as a texture and drawn
 * by one fragment shader that implements every filter, so switching filters
 * is just a uniform. Frames narrower than the current display width (menus
 * while widescreen is on) are pillarboxed at the same pixel scale, so the
 * picture doesn't jump in size when the field widens. */
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include <stdio.h>

#include "pc_port.h"
#include "pc_settings.h"
#include "pc_chat.h"

static SDL_Window *window;
static SDL_GLContext gl;
static GLuint program, vao, texture, overlay_texture;
static GLint u_tex, u_tex_size, u_out_size, u_filter, u_color;
static int tex_w, tex_h;
static HWND launcher_parent, native_window;
static int parent_width, parent_height;

void PcVideoSetParent(uintptr_t parent) { launcher_parent = (HWND)parent; }
int PcVideoIsEmbedded(void) { return launcher_parent != NULL; }

/* Inside the launcher: whenever the launcher is the active window and the game
 * is showing, the keyboard belongs to the game. (Coming back with Alt+Tab, the
 * launcher's own focus restore can land on its panel after it hands focus to
 * us, and the game would get no keys.) Launcher prompts are their own windows,
 * and its menus hide the game, so neither is interrupted. */
void PcVideoKeepFocus(void)
{
    if (!launcher_parent || !native_window || !IsWindowVisible(native_window)) return;
    if (GetForegroundWindow() == GetAncestor(launcher_parent, GA_ROOT) && GetFocus() != native_window)
        SetFocus(native_window);
}
void PcVideoRequestLauncherExit(void)
{
    if (launcher_parent && IsWindow(launcher_parent))
        PostMessage(GetAncestor(launcher_parent, GA_ROOT), WM_APP + 0x31, 0, 0);
}

/* The host owns the outside window. Keep the real SDL/OpenGL game drawable
 * inside its panel even while the user resizes or maximizes that host. */
int PcVideoMaintainParent(void)
{
    RECT rect;
    if (!launcher_parent) return 1;
    if (!IsWindow(launcher_parent) || !GetClientRect(launcher_parent, &rect)) return 0;
    int width = rect.right - rect.left, height = rect.bottom - rect.top;
    if (native_window && width > 0 && height > 0 &&
        (width != parent_width || height != parent_height)) {
        SetWindowPos(native_window, NULL, 0, 0, width, height,
                     SWP_NOACTIVATE | SWP_NOZORDER);
        parent_width = width;
        parent_height = height;
    }
    return 1;
}

static int embed_window(void)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(window, &info)) return -1;
    native_window = info.info.win.window;
    LONG_PTR style = GetWindowLongPtr(native_window, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU |
               WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    style |= WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    SetWindowLongPtr(native_window, GWL_STYLE, style);
    LONG_PTR extended = GetWindowLongPtr(native_window, GWL_EXSTYLE);
    extended &= ~(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);
    SetWindowLongPtr(native_window, GWL_EXSTYLE, extended);
    SetLastError(0);
    if (!SetParent(native_window, launcher_parent) && GetLastError()) return -1;
    SetWindowPos(native_window, NULL, 0, 0, 0, 0,
                 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_FRAMECHANGED);
    if (!PcVideoMaintainParent()) return -1;
    return 0;
}

#define GLFN(ret, name, args) static ret (APIENTRY *name) args;
GLFN(GLuint, p_glCreateShader, (GLenum))
GLFN(void, p_glShaderSource, (GLuint, GLsizei, const GLchar *const *, const GLint *))
GLFN(void, p_glCompileShader, (GLuint))
GLFN(void, p_glGetShaderiv, (GLuint, GLenum, GLint *))
GLFN(void, p_glGetShaderInfoLog, (GLuint, GLsizei, GLsizei *, GLchar *))
GLFN(GLuint, p_glCreateProgram, (void))
GLFN(void, p_glAttachShader, (GLuint, GLuint))
GLFN(void, p_glLinkProgram, (GLuint))
GLFN(void, p_glUseProgram, (GLuint))
GLFN(GLint, p_glGetUniformLocation, (GLuint, const GLchar *))
GLFN(void, p_glUniform1i, (GLint, GLint))
GLFN(void, p_glUniform2f, (GLint, GLfloat, GLfloat))
GLFN(void, p_glGenVertexArrays, (GLsizei, GLuint *))
GLFN(void, p_glBindVertexArray, (GLuint))
GLFN(void, p_glActiveTexture, (GLenum))

static void *load(const char *name)
{
    void *p = SDL_GL_GetProcAddress(name);
    if (!p) PcFatal("OpenGL function %s is missing", name);
    return p;
}

static void load_gl(void)
{
    p_glCreateShader = load("glCreateShader");
    p_glShaderSource = load("glShaderSource");
    p_glCompileShader = load("glCompileShader");
    p_glGetShaderiv = load("glGetShaderiv");
    p_glGetShaderInfoLog = load("glGetShaderInfoLog");
    p_glCreateProgram = load("glCreateProgram");
    p_glAttachShader = load("glAttachShader");
    p_glLinkProgram = load("glLinkProgram");
    p_glUseProgram = load("glUseProgram");
    p_glGetUniformLocation = load("glGetUniformLocation");
    p_glUniform1i = load("glUniform1i");
    p_glUniform2f = load("glUniform2f");
    p_glGenVertexArrays = load("glGenVertexArrays");
    p_glBindVertexArray = load("glBindVertexArray");
    p_glActiveTexture = load("glActiveTexture");
}

static const char *vertex_src =
    "#version 330 core\n"
    "out vec2 uv;\n"
    "void main() {\n"
    "    vec2 p = vec2((gl_VertexID & 1) * 2.0, (gl_VertexID >> 1) * 2.0);\n"
    "    uv = vec2(p.x, 1.0 - p.y);\n"   /* row 0 of the frame is the top */
    "    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);\n"
    "}\n";

static const char *fragment_src =
    "#version 330 core\n"
    "in vec2 uv;\n"
    "out vec4 color;\n"
    "uniform sampler2D tex;\n"
    "uniform vec2 tex_size;\n"   /* frame size in GBA pixels */
    "uniform vec2 out_size;\n"   /* drawn size in screen pixels */
    "uniform int filter_mode;\n"
    "uniform int color_correct;\n"
    "\n"
    "vec3 nearest(vec2 t) { return texture(tex, (floor(t) + 0.5) / tex_size).rgb; }\n"
    "\n"
    "/* Crisp pixels at any scale: sample bilinearly only within the last\n"
    "   screen pixel of each GBA pixel. */\n"
    "vec3 sharp(vec2 t) {\n"
    "    vec2 scale = out_size / tex_size;\n"
    "    vec2 f = fract(t) - 0.5;\n"
    "    vec2 region = 0.5 - 0.5 / scale;\n"
    "    vec2 g = (f - clamp(f, -region, region)) * scale + 0.5;\n"
    "    return texture(tex, (floor(t) + g) / tex_size).rgb;\n"
    "}\n"
    "\n"
    "/* GBA LCD: dark gaps between pixels and red/green/blue subpixel columns,\n"
    "   faded in as the scale grows so small windows don't turn muddy. */\n"
    "vec3 lcd(vec2 t) {\n"
    "    vec3 c = sharp(t);\n"
    "    float scale = out_size.y / tex_size.y;\n"
    "    float strength = clamp((scale - 2.0) / 3.0, 0.0, 1.0);\n"
    "    vec2 f = fract(t);\n"
    "    float grid = smoothstep(0.0, 0.14, f.x) * smoothstep(1.0, 0.86, f.x)\n"
    "               * smoothstep(0.0, 0.14, f.y) * smoothstep(1.0, 0.86, f.y);\n"
    "    vec3 stripe = vec3(f.x < 0.3334 ? 1.0 : 0.55, (f.x >= 0.3334 && f.x < 0.6667) ? 1.0 : 0.55, f.x >= 0.6667 ? 1.0 : 0.55);\n"
    "    vec3 lit = c * stripe * mix(0.62, 1.0, grid) * 1.32;\n"
    "    return mix(c, lit, strength);\n"
    "}\n"
    "\n"
    "/* CRT: gaussian scanlines, a slot mask, and a gentle horizontal blend. */\n"
    "vec3 crt(vec2 t) {\n"
    "    vec2 base = floor(t) + 0.5;\n"
    "    float dy = t.y - base.y;\n"
    "    vec2 off = vec2(0.0, sign(dy));\n"
    "    vec3 a = texture(tex, vec2(t.x, base.y) / tex_size).rgb;\n"
    "    vec3 b = texture(tex, vec2(t.x, base.y + off.y) / tex_size).rgb;\n"
    "    a = pow(a, vec3(2.2)); b = pow(b, vec3(2.2));\n"
    "    float wa = exp(-dy * dy / 0.09), wb = exp(-(1.0 - abs(dy)) * (1.0 - abs(dy)) / 0.09);\n"
    "    vec3 c = a * wa + b * wb;\n"
    "    float m = mod(gl_FragCoord.x, 3.0);\n"
    "    vec3 mask = vec3(m < 1.0 ? 1.0 : 0.7, (m >= 1.0 && m < 2.0) ? 1.0 : 0.7, m >= 2.0 ? 1.0 : 0.7);\n"
    "    c *= mask * 1.45;\n"
    "    return pow(c, vec3(1.0 / 2.2));\n"
    "}\n"
    "\n"
    "/* The GBA's LCD showed colours darker and less saturated than they are\n"
    "   stored; this approximates its response so art looks as designed. */\n"
    "vec3 gba_colors(vec3 c) {\n"
    "    vec3 l = pow(c, vec3(2.0));\n"
    "    vec3 m = vec3(0.80 * l.r + 0.18 * l.g + 0.02 * l.b,\n"
    "                  0.10 * l.r + 0.78 * l.g + 0.12 * l.b,\n"
    "                  0.12 * l.r + 0.14 * l.g + 0.74 * l.b);\n"
    "    return pow(m, vec3(1.0 / 2.2)) * 0.98;\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec2 t = uv * tex_size;\n"
    "    vec3 c;\n"
    "    if (filter_mode == 99) { color = texture(tex, (floor(t) + 0.5) / tex_size); return; }\n" /* chat overlay */
    "    if (filter_mode == 0) c = nearest(t);\n"
    "    else if (filter_mode == 1) c = sharp(t);\n"
    "    else if (filter_mode == 2) c = texture(tex, uv).rgb;\n"
    "    else if (filter_mode == 3) c = lcd(t);\n"
    "    else c = crt(t);\n"
    "    if (color_correct != 0) c = gba_colors(c);\n"
    "    color = vec4(c, 1.0);\n"
    "}\n";

static GLuint compile(GLenum type, const char *src)
{
    GLuint s = p_glCreateShader(type);
    GLint ok;

    p_glShaderSource(s, 1, &src, NULL);
    p_glCompileShader(s);
    p_glGetShaderiv(s, 0x8B81 /* GL_COMPILE_STATUS */, &ok);
    if (!ok) {
        char log[2048];
        p_glGetShaderInfoLog(s, sizeof log, NULL, log);
        PcFatal("shader compile failed:\n%s", log);
    }
    return s;
}

/* AUTO: as many columns as the window's shape holds at the current scaling,
 * so the field fills it edge to edge (even width, GBA_WIDTH..PC_WIDE_MAX_WIDTH). */
static int auto_width(void)
{
    int ww, wh, w;

    if (!window) return 284;
    SDL_GL_GetDrawableSize(window, &ww, &wh);
    if (ww <= 0 || wh <= 0) return 284;
    if (gPcSettings.scaling == PC_SCALE_INTEGER && wh >= GBA_HEIGHT)
        w = ww / (wh / GBA_HEIGHT);
    else
        w = (int)((double)ww * GBA_HEIGHT / wh + 0.5);
    w &= ~1;
    if (w < GBA_WIDTH) w = GBA_WIDTH;
    if (w > PC_WIDE_MAX_WIDTH) w = PC_WIDE_MAX_WIDTH;
    return w;
}

static int logical_width(void)
{
    switch (gPcSettings.widescreen) {
    case PC_WIDE_16_10: return 256;
    case PC_WIDE_16_9: return 284;
    case PC_WIDE_AUTO: return auto_width();
    default: return GBA_WIDTH;
    }
}

int PcVideoDisplayWidth(void) { return logical_width(); }

void PcSettingsApplyVideo(void)
{
    if (!window) return;
    if (launcher_parent) {
        PcVideoMaintainParent();
        SDL_GL_SetSwapInterval(gPcSettings.vsync ? 1 : 0);
        return;
    }
    SDL_SetWindowFullscreen(window, gPcSettings.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    if (!gPcSettings.fullscreen) {
        /* AUTO keeps the window's current shape (16:9 the first time). */
        SDL_SetWindowSize(window, logical_width() * gPcSettings.window_scale, GBA_HEIGHT * gPcSettings.window_scale);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
    SDL_GL_SetSwapInterval(gPcSettings.vsync ? 1 : 0);
}

int PcVideoInit(const char *title)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              logical_width() * gPcSettings.window_scale, GBA_HEIGHT * gPcSettings.window_scale,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI |
                              (launcher_parent ? SDL_WINDOW_HIDDEN : 0));
    if (!window) return -1;
    if (launcher_parent && embed_window() != 0) return -1;
    gl = SDL_GL_CreateContext(window);
    if (!gl) return -1;
    load_gl();

    program = p_glCreateProgram();
    p_glAttachShader(program, compile(0x8B31 /* GL_VERTEX_SHADER */, vertex_src));
    p_glAttachShader(program, compile(0x8B30 /* GL_FRAGMENT_SHADER */, fragment_src));
    p_glLinkProgram(program);
    p_glUseProgram(program);
    u_tex = p_glGetUniformLocation(program, "tex");
    u_tex_size = p_glGetUniformLocation(program, "tex_size");
    u_out_size = p_glGetUniformLocation(program, "out_size");
    u_filter = p_glGetUniformLocation(program, "filter_mode");
    u_color = p_glGetUniformLocation(program, "color_correct");
    p_glUniform1i(u_tex, 0);

    p_glGenVertexArrays(1, &vao);
    p_glBindVertexArray(vao);
    glGenTextures(1, &texture);
    p_glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    PcSettingsApplyVideo();
    if (launcher_parent) {
        SDL_ShowWindow(window);
        SetFocus(native_window);
    }
    return 0;
}

void PcVideoPresent(const uint32_t *frame, int width, int height)
{
    int ww, wh, full_w = logical_width();
    float scale, sx, sy;

    if (width != tex_w || height != tex_h) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame);
        tex_w = width;
        tex_h = height;
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frame);
    }

    SDL_GL_GetDrawableSize(window, &ww, &wh);
    if (full_w < width) full_w = width;

    /* scale from the full display width so narrow frames are pillarboxed */
    sx = (float)ww / full_w;
    sy = (float)wh / height;
    switch (gPcSettings.scaling) {
    case PC_SCALE_INTEGER:
        scale = (float)(int)(sx < sy ? sx : sy);
        if (scale < 1.0f) scale = sx < sy ? sx : sy;
        sx = sy = scale;
        break;
    case PC_SCALE_STRETCH:
        break;
    default:
        sx = sy = sx < sy ? sx : sy;
        break;
    }
    int out_w = (int)(width * sx + 0.5f), out_h = (int)(height * sy + 0.5f);

    glViewport(0, 0, ww, wh);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport((ww - out_w) / 2, (wh - out_h) / 2, out_w, out_h);
    p_glUseProgram(program);
    p_glUniform2f(u_tex_size, (float)width, (float)height);
    p_glUniform2f(u_out_size, (float)out_w, (float)out_h);
    p_glUniform1i(u_filter, gPcSettings.filter);
    p_glUniform1i(u_color, gPcSettings.color_correct);
    p_glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /* online chat: the game's font at half size, over the game picture */
    {
        const uint32_t *chat = PcChatOverlay(width * 2, height * 2);
        if (chat) {
            if (!overlay_texture) {
                glGenTextures(1, &overlay_texture);
                glBindTexture(GL_TEXTURE_2D, overlay_texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            glBindTexture(GL_TEXTURE_2D, overlay_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width * 2, height * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, chat);
            p_glUniform2f(u_tex_size, (float)(width * 2), (float)(height * 2));
            p_glUniform1i(u_filter, 99);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, texture);
        }
    }
    SDL_GL_SwapWindow(window);
}

/* Save what is on screen (after filtering) as a BMP. */
void PcVideoDumpScreen(const char *path)
{
    int ww, wh;
    SDL_Surface *shot;

    SDL_GL_GetDrawableSize(window, &ww, &wh);
    shot = SDL_CreateRGBSurfaceWithFormat(0, ww, wh, 32, SDL_PIXELFORMAT_ABGR8888);
    if (!shot) return;
    glReadPixels(0, 0, ww, wh, GL_RGBA, GL_UNSIGNED_BYTE, shot->pixels);
    /* GL rows are bottom-up */
    for (int y = 0; y < wh / 2; y++) {
        Uint32 *a = (Uint32 *)shot->pixels + y * ww, *b = (Uint32 *)shot->pixels + (wh - 1 - y) * ww;
        for (int x = 0; x < ww; x++) { Uint32 t = a[x]; a[x] = b[x]; b[x] = t; }
    }
    SDL_SaveBMP(shot, path);
    SDL_FreeSurface(shot);
}

/* True when presenting with vsync on a ~60 Hz display can pace the game. */
int PcVideoVsyncPaces(void)
{
    SDL_DisplayMode mode;

    /* A child drawable can be composited without blocking SwapBuffers, even
     * with swap interval 1. Always use the host's explicit 59.73 Hz clock. */
    if (launcher_parent) return 0;

    if (!gPcSettings.vsync || SDL_GetWindowDisplayMode(window, &mode) != 0)
        return 0;
    return mode.refresh_rate >= 59 && mode.refresh_rate <= 61;
}

void PcVideoShutdown(void)
{
    if (gl) SDL_GL_DeleteContext(gl);
    if (window) SDL_DestroyWindow(window);
}
