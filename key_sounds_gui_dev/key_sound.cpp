// ============================================================
// Key Sound - 多套音效方案 + 方案切换器 + 亚克力卡片
// ============================================================

#define _WIN32_WINNT 0x0A00

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <gdiplus.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cwchar>
#include <vector>
#include <string>
#include <algorithm>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "advapi32.lib")

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

using namespace Gdiplus;

// ---------------- 常量 ----------------
#define WINDOW_W        380
#define WINDOW_H        500
#define TITLE_H         40
#define WM_TRAYICON     (WM_USER + 1)
#define IDM_SHOW        1001
#define IDM_TOGGLE      1002
#define IDM_EXIT        1003
#define ID_TIMER_ANIM   1
#define ID_TIMER_SLIDE  2

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

// ---------------- 配色 ----------------
static const Color C_BG          (255, 236, 239, 244);
static const Color C_CARD        (170, 255, 255, 255);
static const Color C_CARD_HOVER  (210, 255, 255, 255);
static const Color C_ACCENT      (255, 156, 163, 175);
static const Color C_ACCENT_HOVER(255, 107, 114, 128);
static const Color C_ACCENT_DEEP (255,  75,  85,  99);
static const Color C_TRACK_OFF   (200, 229, 231, 235);
static const Color C_TRACK_HOVER (255, 209, 213, 219);
static const Color C_TEXT        (255,  75,  85,  99);
static const Color C_TEXT_DIM    (255, 107, 114, 128);
static const Color C_TEXT_BIG    (255,  55,  65,  81);
static const Color C_BORDER      (180, 255, 255, 255);
static const Color C_KNOB_BORDER (255, 209, 213, 219);
static const Color C_CLOSE_HOVER (255, 239,  68,  68);
static const Color C_WHITE       (255, 255, 255, 255);

// ---------------- 数据结构 ----------------
struct SoundPack {
    std::wstring name;
    std::vector<ma_sound*> sounds;
};

// ---------------- 全局状态 ----------------
static ma_engine       g_engine;
static std::vector<SoundPack> g_packs;
static int             g_currentPack  = 0;
static ma_sound*       g_currentSound = nullptr;
static BOOL            g_pressedFlag[256] = { FALSE };
static HWND            g_hwnd = NULL;
static HHOOK           g_hook = NULL;
static HINSTANCE       g_hInst = NULL;
static NOTIFYICONDATAW g_nid = { 0 };

static int   g_currentPage  = 0;
static float g_pageSlideX     = 0.0f;
static float g_pageSlideTarget= 0.0f;

static bool  g_enabled      = true;
static float g_switchPos    = 1.0f;
static float g_switchTarget = 1.0f;
static float g_volume       = 0.8f;
static bool  g_draggingVol  = false;
static bool  g_autoStart    = false;

static bool  g_hoverSwitch      = false;
static bool  g_hoverSettingsBtn = false;
static bool  g_hoverExitBtn     = false;
static bool  g_hoverBackBtn     = false;
static bool  g_hoverOpenBtn     = false;
static bool  g_hoverAutoStart   = false;
static bool  g_hoverCloseBtn    = false;
static bool  g_hoverMinBtn      = false;

static RECT  g_rcTitleBar = { 0 };
static RECT  g_rcCloseBtn = { 0 };
static RECT  g_rcMinBtn   = { 0 };
static RECT  g_rcMainSwitch      = { 0 };
static RECT  g_rcMainSettingsBtn = { 0 };
static RECT  g_rcMainExitBtn     = { 0 };
static RECT  g_rcSetBackBtn    = { 0 };
static RECT  g_rcSetCardPack   = { 0 };
static RECT  g_rcSetCardVolume = { 0 };
static RECT  g_rcSetSlider     = { 0 };
static RECT  g_rcSetOpenBtn    = { 0 };
static RECT  g_rcAutoStartBox  = { 0 };

static std::vector<RECT> g_rcPackChips;
static std::vector<bool> g_hoverPackChip;

static HICON  g_appIcon  = nullptr;
static HICON  g_trayIcon = nullptr;

// ============================================================
//                   GDI+ 绘制辅助
// ============================================================

static void AddRoundRect(GraphicsPath* path, const RectF& rect, float radius)
{
    float d = radius * 2.0f;
    if (d > rect.Width)  d = rect.Width;
    if (d > rect.Height) d = rect.Height;

    path->AddArc(rect.X,                    rect.Y,                    d, d, 180, 90);
    path->AddArc(rect.X + rect.Width - d,   rect.Y,                    d, d, 270, 90);
    path->AddArc(rect.X + rect.Width - d,   rect.Y + rect.Height - d,  d, d,   0, 90);
    path->AddArc(rect.X,                    rect.Y + rect.Height - d,  d, d,  90, 90);
    path->CloseFigure();
}

static void FillRoundRect(Graphics& g, const RectF& rect, float radius, const Color& color)
{
    GraphicsPath path;
    AddRoundRect(&path, rect, radius);
    SolidBrush brush(color);
    g.FillPath(&brush, &path);
}

static void StrokeRoundRect(Graphics& g, const RectF& rect, float radius, const Color& color, float width)
{
    GraphicsPath path;
    AddRoundRect(&path, rect, radius);
    Pen pen(color, width);
    g.DrawPath(&pen, &path);
}

static void DrawAcrylicCard(Graphics& g, const RectF& rect, float radius, bool hover)
{
    {
        RectF shadowRc(rect.X + 1, rect.Y + 3, rect.Width, rect.Height);
        GraphicsPath path;
        AddRoundRect(&path, shadowRc, radius);
        SolidBrush shadowBrush(Color(28, 0, 0, 0));
        g.FillPath(&shadowBrush, &path);
    }

    Color cardColor = hover ? C_CARD_HOVER : C_CARD;
    FillRoundRect(g, rect, radius, cardColor);
    StrokeRoundRect(g, rect, radius, C_BORDER, 1.0f);
}

static Color LerpColor(const Color& a, const Color& b, float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    BYTE r  = (BYTE)(a.GetR() + (int)((int)b.GetR()  - (int)a.GetR())  * t);
    BYTE g  = (BYTE)(a.GetG() + (int)((int)b.GetG()  - (int)a.GetG())  * t);
    BYTE bl = (BYTE)(a.GetB() + (int)((int)b.GetB()  - (int)a.GetB())  * t);
    return Color(255, r, g, bl);
}

// ============================================================
//                   路径
// ============================================================

static void GetExeDir(char* out, size_t outSize)
{
    GetModuleFileNameA(NULL, out, (DWORD)outSize);
    char* last = strrchr(out, '\\');
    if (last) *last = '\0';
}

static void GetExeDirW(wchar_t* out, size_t outSize)
{
    GetModuleFileNameW(NULL, out, (DWORD)outSize);
    wchar_t* last = wcsrchr(out, L'\\');
    if (last) *last = L'\0';
}

static void GetExeSoundsDir(char* out, size_t outSize)
{
    char exeDir[MAX_PATH];
    GetExeDir(exeDir, MAX_PATH);
    sprintf_s(out, outSize, "%s\\sounds", exeDir);
}

static void GetAppDataSoundsDir(char* out, size_t outSize)
{
    char appData[MAX_PATH] = { 0 };
    if (GetEnvironmentVariableA("APPDATA", appData, MAX_PATH) == 0) {
        GetExeSoundsDir(out, outSize);
        return;
    }
    sprintf_s(out, outSize, "%s\\KeySound\\sounds", appData);
}

static BOOL IsSupportedExt(const char* filename)
{
    const char* dot = NULL;
    for (const char* p = filename; *p; ++p) if (*p == '.') dot = p;
    if (!dot) return FALSE;

    char ext[8] = { 0 };
    int i = 0;
    for (const char* p = dot; *p && i < 7; ++p, ++i)
        ext[i] = (char)tolower((unsigned char)*p);

    return strcmp(ext, ".wav")  == 0
        || strcmp(ext, ".mp3")  == 0
        || strcmp(ext, ".flac") == 0;
}

// ============================================================
//                   音效加载
// ============================================================

// 加载单个目录到 vector
static int LoadSoundsInto(const char* dir, std::vector<ma_sound*>& out)
{
    char searchPath[MAX_PATH];
    sprintf_s(searchPath, MAX_PATH, "%s\\*", dir);

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    int added = 0;
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (!IsSupportedExt(findData.cFileName)) continue;
        if ((int)out.size() >= 128) break;

        char fullPath[MAX_PATH];
        sprintf_s(fullPath, MAX_PATH, "%s\\%s", dir, findData.cFileName);

        ma_sound* snd = new ma_sound();
        if (ma_sound_init_from_file(&g_engine, fullPath,
                                    MA_SOUND_FLAG_DECODE,
                                    NULL, NULL, snd) == MA_SUCCESS) {
            out.push_back(snd);
            added++;
        } else {
            delete snd;
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    return added;
}

// 扫描目录，构建所有方案
static void LoadAllPacks(const char* baseDir)
{
    // 先找子文件夹
    std::vector<std::string> subdirs;
    {
        char searchPath[MAX_PATH];
        sprintf_s(searchPath, MAX_PATH, "%s\\*", baseDir);

        WIN32_FIND_DATAA fd;
        HANDLE hf = FindFirstFileA(searchPath, &fd);
        if (hf != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
                if (strcmp(fd.cFileName, ".") == 0) continue;
                if (strcmp(fd.cFileName, "..") == 0) continue;
                subdirs.push_back(fd.cFileName);
            } while (FindNextFileA(hf, &fd));
            FindClose(hf);
        }
    }

    std::sort(subdirs.begin(), subdirs.end());

    if (!subdirs.empty()) {
        for (auto& sub : subdirs) {
            char subPath[MAX_PATH];
            sprintf_s(subPath, MAX_PATH, "%s\\%s", baseDir, sub.c_str());

            SoundPack pack;
            wchar_t nameW[MAX_PATH] = { 0 };
            MultiByteToWideChar(CP_ACP, 0, sub.c_str(), -1, nameW, MAX_PATH);
            pack.name = nameW;

            LoadSoundsInto(subPath, pack.sounds);
            if (!pack.sounds.empty()) {
                g_packs.push_back(pack);
            }
        }
    } else {
        SoundPack pack;
        pack.name = L"方案1";
        LoadSoundsInto(baseDir, pack.sounds);
        if (!pack.sounds.empty()) {
            g_packs.push_back(pack);
        }
    }
}

static void FreeAllPacks()
{
    for (auto& pack : g_packs) {
        for (auto* snd : pack.sounds) {
            ma_sound_uninit(snd);
            delete snd;
        }
        pack.sounds.clear();
    }
    g_packs.clear();
    g_currentSound = nullptr;
    g_currentPack = 0;
}

static int CurrentPackCount()
{
    if (g_currentPack < 0 || g_currentPack >= (int)g_packs.size()) return 0;
    return (int)g_packs[g_currentPack].sounds.size();
}

static void PlayInterrupting(ma_sound* snd)
{
    if (!snd) return;
    if (g_currentSound && g_currentSound != snd) {
        ma_sound_stop(g_currentSound);
    }
    ma_sound_stop(snd);
    ma_sound_seek_to_pcm_frame(snd, 0);
    ma_sound_start(snd);
    g_currentSound = snd;
}

static void PlayRandomFromCurrentPack()
{
    if (g_currentPack < 0 || g_currentPack >= (int)g_packs.size()) return;
    auto& sounds = g_packs[g_currentPack].sounds;
    if (sounds.empty()) return;
    PlayInterrupting(sounds[rand() % (int)sounds.size()]);
}

static void StopCurrentSound()
{
    if (g_currentSound) {
        ma_sound_stop(g_currentSound);
        g_currentSound = nullptr;
    }
}

// ============================================================
//                   开机自启
// ============================================================

static bool IsAutoStartEnabled()
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    wchar_t buf[MAX_PATH * 2] = { 0 };
    DWORD size = sizeof(buf);
    LONG ret = RegQueryValueExW(hKey, L"KeySound", NULL, NULL,
                                (LPBYTE)buf, &size);
    RegCloseKey(hKey);
    return (ret == ERROR_SUCCESS);
}

static void SetAutoStart(bool enable)
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return;

    if (enable) {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        wchar_t quoted[MAX_PATH + 32];
        swprintf_s(quoted, MAX_PATH + 32, L"\"%s\" --startup", exePath);

        RegSetValueExW(hKey, L"KeySound", 0, REG_SZ,
                       (const BYTE*)quoted,
                       (DWORD)((wcslen(quoted) + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(hKey, L"KeySound");
    }
    RegCloseKey(hKey);
}

// ============================================================
//                   键盘钩子
// ============================================================

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && g_enabled) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (p->vkCode < 256 && !g_pressedFlag[p->vkCode]) {
                g_pressedFlag[p->vkCode] = TRUE;
                PlayRandomFromCurrentPack();
            }
        }
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (p->vkCode < 256) g_pressedFlag[p->vkCode] = FALSE;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// ============================================================
//                   布局
// ============================================================

static void LayoutControls()
{
    g_rcTitleBar.left   = 0;
    g_rcTitleBar.top    = 0;
    g_rcTitleBar.right  = WINDOW_W;
    g_rcTitleBar.bottom = TITLE_H;

    g_rcCloseBtn.left   = WINDOW_W - 40;
    g_rcCloseBtn.top    = 0;
    g_rcCloseBtn.right  = WINDOW_W;
    g_rcCloseBtn.bottom = TITLE_H;

    g_rcMinBtn.left   = WINDOW_W - 80;
    g_rcMinBtn.top    = 0;
    g_rcMinBtn.right  = WINDOW_W - 40;
    g_rcMinBtn.bottom = TITLE_H;

    int sw = 56, sh = 30;
    g_rcMainSwitch.left   = WINDOW_W - 30 - sw;
    g_rcMainSwitch.top    = 413;
    g_rcMainSwitch.right  = g_rcMainSwitch.left + sw;
    g_rcMainSwitch.bottom = g_rcMainSwitch.top + sh;

    g_rcMainSettingsBtn.left   = 30;
    g_rcMainSettingsBtn.top    = 406;
    g_rcMainSettingsBtn.right  = 125;
    g_rcMainSettingsBtn.bottom = 450;

    g_rcMainExitBtn.left   = 135;
    g_rcMainExitBtn.top    = 406;
    g_rcMainExitBtn.right  = 230;
    g_rcMainExitBtn.bottom = 450;

    // 设置页
    g_rcSetBackBtn.left   = 30;
    g_rcSetBackBtn.top    = 60;
    g_rcSetBackBtn.right  = 110;
    g_rcSetBackBtn.bottom = 96;

    // 方案卡片
    g_rcSetCardPack.left   = 30;
    g_rcSetCardPack.top    = 110;
    g_rcSetCardPack.right  = WINDOW_W - 30;
    g_rcSetCardPack.bottom = 200;

    // 音量卡片
    g_rcSetCardVolume.left   = 30;
    g_rcSetCardVolume.top    = 215;
    g_rcSetCardVolume.right  = WINDOW_W - 30;
    g_rcSetCardVolume.bottom = 320;

    // 音量滑块
    g_rcSetSlider.left   = 50;
    g_rcSetSlider.right  = WINDOW_W - 50;
    g_rcSetSlider.top    = 285;
    g_rcSetSlider.bottom = 305;

    // 打开文件夹按钮
    g_rcSetOpenBtn.left   = 30;
    g_rcSetOpenBtn.top    = 335;
    g_rcSetOpenBtn.right  = WINDOW_W - 30;
    g_rcSetOpenBtn.bottom = 379;

    // 开机自启
    g_rcAutoStartBox.left   = 30;
    g_rcAutoStartBox.top    = 390;
    g_rcAutoStartBox.right  = WINDOW_W - 30;
    g_rcAutoStartBox.bottom = 430;
}

static void LayoutPackChips()
{
    g_rcPackChips.clear();
    g_hoverPackChip.clear();

    int n = (int)g_packs.size();
    if (n == 0) return;

    int cardLeft = g_rcSetCardPack.left;
    int cardRight = g_rcSetCardPack.right;
    int padding = 14;
    int gap = 8;
    int availableW = cardRight - cardLeft - padding * 2;

    int chipW = (availableW - (n - 1) * gap) / n;
    if (chipW > 95) chipW = 95;
    if (chipW < 40) chipW = 40;

    int totalW = n * chipW + (n - 1) * gap;
    int startX = cardLeft + padding + (availableW - totalW) / 2;
    if (startX < cardLeft + padding) startX = cardLeft + padding;

    int chipY = g_rcSetCardPack.top + 45;
    int chipH = 32;

    for (int i = 0; i < n; ++i) {
        RECT rc;
        rc.left   = startX + i * (chipW + gap);
        rc.top    = chipY;
        rc.right  = rc.left + chipW;
        rc.bottom = chipY + chipH;
        g_rcPackChips.push_back(rc);
        g_hoverPackChip.push_back(false);
    }
}

static void SwitchPage(int newPage)
{
    if (g_currentPage == newPage) return;
    g_currentPage = newPage;
    g_pageSlideTarget = (newPage == 0) ? 0.0f : -(float)WINDOW_W;
    SetTimer(g_hwnd, ID_TIMER_SLIDE, 16, NULL);
}

// ============================================================
//                   界面绘制
// ============================================================

static void DrawTitleBar(Graphics& g)
{
    SolidBrush dotBrush(C_ACCENT);
    g.FillEllipse(&dotBrush, 20, 15, 10, 10);

    {
        Font font(L"Microsoft YaHei UI", 14, FontStyleBold, UnitPixel);
        SolidBrush brush(C_TEXT);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentNear);
        sf.SetLineAlignment(StringAlignmentCenter);
        RectF rc(38, 0, 200, (REAL)TITLE_H);
        g.DrawString(L"Key Sound", -1, &font, rc, &sf, &brush);
    }

    {
        Color c = g_hoverMinBtn ? C_ACCENT_HOVER : C_TEXT_DIM;
        Pen pen(c, 1.5f);
        REAL cx = (REAL)((g_rcMinBtn.left + g_rcMinBtn.right) / 2.0f);
        REAL cy = (REAL)((g_rcMinBtn.top + g_rcMinBtn.bottom) / 2.0f);
        g.DrawLine(&pen, cx - 6, cy, cx + 6, cy);
    }

    {
        Color c = g_hoverCloseBtn ? C_CLOSE_HOVER : C_TEXT_DIM;
        Pen pen(c, 1.5f);
        REAL cx = (REAL)((g_rcCloseBtn.left + g_rcCloseBtn.right) / 2.0f);
        REAL cy = (REAL)((g_rcCloseBtn.top + g_rcCloseBtn.bottom) / 2.0f);
        g.DrawLine(&pen, cx - 6, cy - 6, cx + 6, cy + 6);
        g.DrawLine(&pen, cx + 6, cy - 6, cx - 6, cy + 6);
    }
}

static void DrawMainPage(Graphics& g)
{
    // 副标题
    {
        Font font(L"Microsoft YaHei UI", 13, FontStyleRegular, UnitPixel);
        SolidBrush brush(C_TEXT_DIM);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        RectF rc(30, 60, (REAL)(WINDOW_W - 60), 24);
        g.DrawString(L"按键音效播放器", -1, &font, rc, &sf, &brush);
    }

    // 中央大号状态卡片
    {
        REAL cardW = 220.0f;
        REAL cardH = 110.0f;
        REAL cardX = (WINDOW_W - cardW) / 2.0f;
        REAL cardY = 140.0f;

        RectF cardRc(cardX, cardY, cardW, cardH);
        DrawAcrylicCard(g, cardRc, 20.0f, false);

        const wchar_t* txt = g_enabled ? L"已开启" : L"已关闭";
        Color c = g_enabled ? C_TEXT_BIG : C_TEXT_DIM;
        Font font(L"Microsoft YaHei UI", 38, FontStyleBold, UnitPixel);
        SolidBrush brush(c);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(txt, -1, &font, cardRc, &sf, &brush);
    }

    // 当前方案 + 音效数量
    {
        wchar_t buf[128];
        if (!g_packs.empty()) {
            swprintf_s(buf, 128, L"%s · %d 个音效",
                       g_packs[g_currentPack].name.c_str(),
                       CurrentPackCount());
        } else {
            wcscpy_s(buf, 128, L"未找到音效");
        }
        Font font(L"Microsoft YaHei UI", 13, FontStyleRegular, UnitPixel);
        SolidBrush brush(C_TEXT_DIM);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        RectF rc(30, 270, (REAL)(WINDOW_W - 60), 24);
        g.DrawString(buf, -1, &font, rc, &sf, &brush);
    }

    // 设置按钮
    {
        REAL bx = (REAL)g_rcMainSettingsBtn.left;
        REAL by = (REAL)g_rcMainSettingsBtn.top;
        REAL bw = (REAL)(g_rcMainSettingsBtn.right - g_rcMainSettingsBtn.left);
        REAL bh = (REAL)(g_rcMainSettingsBtn.bottom - g_rcMainSettingsBtn.top);

        RectF btnRc(bx, by, bw, bh);
        DrawAcrylicCard(g, btnRc, 12.0f, g_hoverSettingsBtn);

        Font font(L"Microsoft YaHei UI", 13, FontStyleRegular, UnitPixel);
        SolidBrush brush(g_hoverSettingsBtn ? C_ACCENT_HOVER : C_TEXT);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(L"设置", -1, &font, btnRc, &sf, &brush);
    }

    // 退出按钮
    {
        REAL bx = (REAL)g_rcMainExitBtn.left;
        REAL by = (REAL)g_rcMainExitBtn.top;
        REAL bw = (REAL)(g_rcMainExitBtn.right - g_rcMainExitBtn.left);
        REAL bh = (REAL)(g_rcMainExitBtn.bottom - g_rcMainExitBtn.top);

        RectF btnRc(bx, by, bw, bh);
        DrawAcrylicCard(g, btnRc, 12.0f, g_hoverExitBtn);

        Color textColor = g_hoverExitBtn ? C_CLOSE_HOVER : C_TEXT;
        Font font(L"Microsoft YaHei UI", 13, FontStyleRegular, UnitPixel);
        SolidBrush brush(textColor);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(L"退出", -1, &font, btnRc, &sf, &brush);
    }

    // 开关
    {
        REAL tx = (REAL)g_rcMainSwitch.left;
        REAL ty = (REAL)g_rcMainSwitch.top;
        REAL tw = (REAL)(g_rcMainSwitch.right - g_rcMainSwitch.left);
        REAL th = (REAL)(g_rcMainSwitch.bottom - g_rcMainSwitch.top);

        Color offColor   = g_hoverSwitch ? C_TRACK_HOVER  : C_TRACK_OFF;
        Color onColor    = g_hoverSwitch ? C_ACCENT_HOVER : C_ACCENT;
        Color trackColor = LerpColor(offColor, onColor, g_switchPos);

        RectF trackRc(tx, ty, tw, th);
        FillRoundRect(g, trackRc, th / 2.0f, trackColor);

        REAL knobSize = th - 6.0f;
        REAL knobX = tx + 3.0f + (tw - knobSize - 6.0f) * g_switchPos;
        REAL knobY = ty + 3.0f;

        SolidBrush knobBrush(C_WHITE);
        g.FillEllipse(&knobBrush, knobX, knobY, knobSize, knobSize);
        Pen knobPen(C_KNOB_BORDER, 1.0f);
        g.DrawEllipse(&knobPen, knobX, knobY, knobSize, knobSize);
    }
}

static void DrawSettingsPage(Graphics& g)
{
    // 返回按钮
    {
        REAL bx = (REAL)g_rcSetBackBtn.left;
        REAL by = (REAL)g_rcSetBackBtn.top;
        REAL bw = (REAL)(g_rcSetBackBtn.right - g_rcSetBackBtn.left);
        REAL bh = (REAL)(g_rcSetBackBtn.bottom - g_rcSetBackBtn.top);

        RectF btnRc(bx, by, bw, bh);
        DrawAcrylicCard(g, btnRc, 10.0f, g_hoverBackBtn);

        Font font(L"Microsoft YaHei UI", 12, FontStyleRegular, UnitPixel);
        SolidBrush brush(g_hoverBackBtn ? C_ACCENT_HOVER : C_TEXT);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(L"< 返回", -1, &font, btnRc, &sf, &brush);
    }

    // "设置"标题
    {
        Font font(L"Microsoft YaHei UI", 18, FontStyleBold, UnitPixel);
        SolidBrush brush(C_TEXT);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);
        RectF rc(0, 62, (REAL)WINDOW_W, 30);
        g.DrawString(L"设置", -1, &font, rc, &sf, &brush);
    }

    // 方案卡片
    {
        RectF cardRc(
            (REAL)g_rcSetCardPack.left, (REAL)g_rcSetCardPack.top,
            (REAL)(g_rcSetCardPack.right - g_rcSetCardPack.left),
            (REAL)(g_rcSetCardPack.bottom - g_rcSetCardPack.top));
        DrawAcrylicCard(g, cardRc, 16.0f, false);
    }

    // "音效方案" 标签
    {
        Font font(L"Microsoft YaHei UI", 13, FontStyleRegular, UnitPixel);
        SolidBrush brush(C_TEXT);
        StringFormat sf;
        RectF rc(50, (REAL)(g_rcSetCardPack.top + 12), 200, 24);
        g.DrawString(L"音效方案", -1, &font, rc, &sf, &brush);
    }

    // 方案芯片
    for (size_t i = 0; i < g_rcPackChips.size(); ++i) {
        RECT& rc = g_rcPackChips[i];
        bool isSelected = ((int)i == g_currentPack);
        bool isHover    = g_hoverPackChip[i];

        RectF chipRc((REAL)rc.left, (REAL)rc.top,
                     (REAL)(rc.right - rc.left),
                     (REAL)(rc.bottom - rc.top));

        if (isSelected) {
            FillRoundRect(g, chipRc, 8.0f, C_ACCENT_DEEP);
        } else {
            Color bg = isHover ? C_CARD_HOVER : Color(120, 255, 255, 255);
            FillRoundRect(g, chipRc, 8.0f, bg);
            StrokeRoundRect(g, chipRc, 8.0f,
                            isHover ? C_ACCENT_DEEP : C_BORDER, 1.0f);
        }

        Font font(L"Microsoft YaHei UI", 12, FontStyleRegular, UnitPixel);
        SolidBrush brush(isSelected ? C_WHITE : C_TEXT);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);

        const wchar_t* name = g_packs[i].name.c_str();
        g.DrawString(name, -1, &font, chipRc, &sf, &brush);
    }

    // 音量卡片
    {
        RectF cardRc(
            (REAL)g_rcSetCardVolume.left, (REAL)g_rcSetCardVolume.top,
            (REAL)(g_rcSetCardVolume.right - g_rcSetCardVolume.left),
            (REAL)(g_rcSetCardVolume.bottom - g_rcSetCardVolume.top));
        DrawAcrylicCard(g, cardRc, 16.0f, false);
    }

    // "音量" 标签
    {
        Font font(L"Microsoft YaHei UI", 13, FontStyleRegular, UnitPixel);
        SolidBrush brush(C_TEXT);
        StringFormat sf;
        RectF rc(50, (REAL)(g_rcSetCardVolume.top + 20), 200, 24);
        g.DrawString(L"音量", -1, &font, rc, &sf, &brush);
    }

    // 音量数值
    {
        wchar_t buf[16];
        swprintf_s(buf, L"%d%%", (int)(g_volume * 100 + 0.5f));
        Font font(L"Microsoft YaHei UI", 13, FontStyleRegular, UnitPixel);
        SolidBrush brush(C_TEXT_DIM);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentFar);
        RectF rc((REAL)(WINDOW_W - 130), (REAL)(g_rcSetCardVolume.top + 20), 80, 24);
        g.DrawString(buf, -1, &font, rc, &sf, &brush);
    }

    // 音量滑块
    {
        REAL trackH = 6.0f;
        REAL trackX = (REAL)g_rcSetSlider.left;
        REAL trackW = (REAL)(g_rcSetSlider.right - g_rcSetSlider.left);
        REAL trackY = (REAL)g_rcSetSlider.top +
                      ((REAL)(g_rcSetSlider.bottom - g_rcSetSlider.top) - trackH) / 2.0f;

        RectF bgTrack(trackX, trackY, trackW, trackH);
        FillRoundRect(g, bgTrack, trackH / 2.0f, C_TRACK_OFF);

        REAL filled = trackW * g_volume;
        if (filled > 1.0f) {
            RectF fillTrack(trackX, trackY, filled, trackH);
            FillRoundRect(g, fillTrack, trackH / 2.0f, C_ACCENT);
        }

        REAL knobR  = 8.0f;
        REAL knobCx = trackX + filled;
        REAL knobCy = trackY + trackH / 2.0f;

        SolidBrush knobBrush(C_WHITE);
        g.FillEllipse(&knobBrush, knobCx - knobR, knobCy - knobR,
                      knobR * 2, knobR * 2);
        Pen knobPen(C_ACCENT, 2.0f);
        g.DrawEllipse(&knobPen, knobCx - knobR, knobCy - knobR,
                      knobR * 2, knobR * 2);
    }

    // 打开音效文件夹按钮
    {
        REAL bx = (REAL)g_rcSetOpenBtn.left;
        REAL by = (REAL)g_rcSetOpenBtn.top;
        REAL bw = (REAL)(g_rcSetOpenBtn.right - g_rcSetOpenBtn.left);
        REAL bh = (REAL)(g_rcSetOpenBtn.bottom - g_rcSetOpenBtn.top);

        RectF btnRc(bx, by, bw, bh);
        DrawAcrylicCard(g, btnRc, 12.0f, g_hoverOpenBtn);

        Font font(L"Microsoft YaHei UI", 13, FontStyleRegular, UnitPixel);
        SolidBrush brush(g_hoverOpenBtn ? C_ACCENT_HOVER : C_TEXT);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(L"打开音效文件夹", -1, &font, btnRc, &sf, &brush);
    }

    // 开机自启
    {
        REAL bx = (REAL)g_rcAutoStartBox.left;
        REAL by = (REAL)g_rcAutoStartBox.top;
        REAL bw = (REAL)(g_rcAutoStartBox.right - g_rcAutoStartBox.left);
        REAL bh = (REAL)(g_rcAutoStartBox.bottom - g_rcAutoStartBox.top);

        RectF boxRc(bx, by, bw, bh);
        DrawAcrylicCard(g, boxRc, 12.0f, g_hoverAutoStart);

        REAL cbSize = 20.0f;
        REAL cbX = bx + 16.0f;
        REAL cbY = by + (bh - cbSize) / 2.0f;
        RectF cbRc(cbX, cbY, cbSize, cbSize);

        if (g_autoStart) {
            FillRoundRect(g, cbRc, 5.0f, C_ACCENT_DEEP);
            Pen pen(C_WHITE, 2.0f);
            pen.SetStartCap(LineCapRound);
            pen.SetEndCap(LineCapRound);
            g.DrawLine(&pen, cbX + 5, cbY + 10, cbX + 9, cbY + 14);
            g.DrawLine(&pen, cbX + 9, cbY + 14, cbX + 15, cbY + 6);
        } else {
            SolidBrush inner(Color(200, 255, 255, 255));
            GraphicsPath path;
            AddRoundRect(&path, cbRc, 5.0f);
            g.FillPath(&inner, &path);
            StrokeRoundRect(g, cbRc, 5.0f, C_KNOB_BORDER, 1.5f);
        }

        Font font(L"Microsoft YaHei UI", 13, FontStyleRegular, UnitPixel);
        SolidBrush brush(C_TEXT);
        StringFormat sf;
        sf.SetLineAlignment(StringAlignmentCenter);
        RectF rc(cbX + cbSize + 12, by, 200, bh);
        g.DrawString(L"开机自动启动", -1, &font, rc, &sf, &brush);
    }
}

static void DrawUI(Graphics& g, int w, int h)
{
    SolidBrush bgBrush(C_BG);
    g.FillRectangle(&bgBrush, 0, 0, w, h);

    {
        GraphicsState state = g.Save();
        g.TranslateTransform(g_pageSlideX, 0);
        DrawMainPage(g);
        g.Restore(state);
    }

    {
        GraphicsState state = g.Save();
        g.TranslateTransform(g_pageSlideX + (REAL)WINDOW_W, 0);
        DrawSettingsPage(g);
        g.Restore(state);
    }

    DrawTitleBar(g);
}

// ============================================================
//                   交互
// ============================================================

static void UpdateVolumeFromX(int x)
{
    REAL trackX = (REAL)g_rcSetSlider.left;
    REAL trackW = (REAL)(g_rcSetSlider.right - g_rcSetSlider.left);
    REAL v = (REAL)(x - trackX) / trackW;

    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    g_volume = v;
    ma_engine_set_volume(&g_engine, g_volume);
    InvalidateRect(g_hwnd, NULL, FALSE);
}

static void OpenSoundsFolder()
{
    char exeSoundsDir[MAX_PATH];
    GetExeSoundsDir(exeSoundsDir, MAX_PATH);

    if (GetFileAttributesA(exeSoundsDir) != INVALID_FILE_ATTRIBUTES) {
        ShellExecuteA(NULL, "open", exeSoundsDir, NULL, NULL, SW_SHOWNORMAL);
        return;
    }

    char appDataDir[MAX_PATH];
    GetAppDataSoundsDir(appDataDir, MAX_PATH);
    CreateDirectoryA(appDataDir, NULL);
    ShellExecuteA(NULL, "open", appDataDir, NULL, NULL, SW_SHOWNORMAL);
}

static void ShowTrayMenu()
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, IDM_SHOW, L"显示主窗口");
    AppendMenuW(hMenu,
                MF_STRING | (g_enabled ? MF_CHECKED : 0),
                IDM_TOGGLE, L"启用音效");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"退出");

    SetForegroundWindow(g_hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, g_hwnd, NULL);
    DestroyMenu(hMenu);
}

// ============================================================
//                   窗口过程
// ============================================================

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_NCHITTEST:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &pt);

        if (PtInRect(&g_rcCloseBtn, pt) || PtInRect(&g_rcMinBtn, pt))
            return HTCLIENT;

        if (pt.y >= 0 && pt.y < TITLE_H)
            return HTCAPTION;

        return HTCLIENT;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int w = rc.right;
        int h = rc.bottom;

        HDC     memDC  = CreateCompatibleDC(hdc);
        HBITMAP memBmp = CreateCompatibleBitmap(hdc, w, h);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

        {
            Graphics g(memDC);
            g.SetSmoothingMode(SmoothingModeAntiAlias);
            g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
            DrawUI(g, w, h);
        }

        BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBmp);
        DeleteObject(memBmp);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        POINT pt = { x, y };

        if (PtInRect(&g_rcCloseBtn, pt)) {
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        if (PtInRect(&g_rcMinBtn, pt)) {
            ShowWindow(hwnd, SW_MINIMIZE);
            return 0;
        }

        if (fabsf(g_pageSlideTarget - g_pageSlideX) > 1.0f) return 0;

        if (g_currentPage == 0)
        {
            if (PtInRect(&g_rcMainSwitch, pt)) {
                g_enabled = !g_enabled;
                g_switchTarget = g_enabled ? 1.0f : 0.0f;
                SetTimer(hwnd, ID_TIMER_ANIM, 16, NULL);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (PtInRect(&g_rcMainSettingsBtn, pt)) {
                SwitchPage(1);
            }
            else if (PtInRect(&g_rcMainExitBtn, pt)) {
                DestroyWindow(hwnd);
            }
        }
        else
        {
            if (PtInRect(&g_rcSetBackBtn, pt)) {
                SwitchPage(0);
            }
            else if (PtInRect(&g_rcSetSlider, pt)) {
                g_draggingVol = true;
                SetCapture(hwnd);
                UpdateVolumeFromX(x);
            }
            else if (PtInRect(&g_rcSetOpenBtn, pt)) {
                OpenSoundsFolder();
            }
            else if (PtInRect(&g_rcAutoStartBox, pt)) {
                g_autoStart = !g_autoStart;
                SetAutoStart(g_autoStart);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else {
                // 方案芯片
                for (size_t i = 0; i < g_rcPackChips.size(); ++i) {
                    if (PtInRect(&g_rcPackChips[i], pt)) {
                        if ((int)i != g_currentPack) {
                            StopCurrentSound();
                            g_currentPack = (int)i;
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                        return 0;
                    }
                }
            }
        }
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        if (g_draggingVol) {
            UpdateVolumeFromX(GET_X_LPARAM(lParam));
            return 0;
        }

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        bool changed = false;

        bool hc = PtInRect(&g_rcCloseBtn, pt) != 0;
        bool hm = PtInRect(&g_rcMinBtn, pt) != 0;
        if (hc != g_hoverCloseBtn || hm != g_hoverMinBtn) {
            g_hoverCloseBtn = hc;
            g_hoverMinBtn   = hm;
            changed = true;
        }

        if (fabsf(g_pageSlideTarget - g_pageSlideX) < 1.0f) {
            if (g_currentPage == 0) {
                bool hs    = PtInRect(&g_rcMainSwitch, pt) != 0;
                bool hset  = PtInRect(&g_rcMainSettingsBtn, pt) != 0;
                bool hexit = PtInRect(&g_rcMainExitBtn, pt) != 0;
                if (hs != g_hoverSwitch || hset != g_hoverSettingsBtn || hexit != g_hoverExitBtn) {
                    g_hoverSwitch      = hs;
                    g_hoverSettingsBtn = hset;
                    g_hoverExitBtn     = hexit;
                    changed = true;
                }
            } else {
                bool hb = PtInRect(&g_rcSetBackBtn, pt) != 0;
                bool ho = PtInRect(&g_rcSetOpenBtn, pt) != 0;
                bool ha = PtInRect(&g_rcAutoStartBox, pt) != 0;
                if (hb != g_hoverBackBtn || ho != g_hoverOpenBtn || ha != g_hoverAutoStart) {
                    g_hoverBackBtn   = hb;
                    g_hoverOpenBtn   = ho;
                    g_hoverAutoStart = ha;
                    changed = true;
                }

                // 方案芯片 hover
                for (size_t i = 0; i < g_rcPackChips.size(); ++i) {
                    bool h = PtInRect(&g_rcPackChips[i], pt) != 0;
                    if (h != g_hoverPackChip[i]) {
                        g_hoverPackChip[i] = h;
                        changed = true;
                    }
                }
            }
        }

        if (changed) InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }

    case WM_LBUTTONUP:
        if (g_draggingVol) {
            g_draggingVol = false;
            ReleaseCapture();
        }
        return 0;

    case WM_TIMER:
        if (wParam == ID_TIMER_ANIM) {
            float diff = g_switchTarget - g_switchPos;
            if (fabsf(diff) < 0.01f) {
                g_switchPos = g_switchTarget;
                KillTimer(hwnd, ID_TIMER_ANIM);
            } else {
                g_switchPos += diff * 0.35f;
            }
            InvalidateRect(hwnd, NULL, FALSE);
        }
        else if (wParam == ID_TIMER_SLIDE) {
            float diff = g_pageSlideTarget - g_pageSlideX;
            if (fabsf(diff) < 0.5f) {
                g_pageSlideX = g_pageSlideTarget;
                KillTimer(hwnd, ID_TIMER_SLIDE);
            } else {
                g_pageSlideX += diff * 0.28f;
            }
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;

    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP) {
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
        }
        else if (lParam == WM_RBUTTONUP) {
            ShowTrayMenu();
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_SHOW:
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
            break;
        case IDM_TOGGLE:
            g_enabled = !g_enabled;
            g_switchTarget = g_enabled ? 1.0f : 0.0f;
            SetTimer(hwnd, ID_TIMER_ANIM, 16, NULL);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;
        }
        return 0;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ============================================================
//                   入口
// ============================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"KeySound_SingleInstance_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 0;
    }

    g_hInst = hInstance;

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken = 0;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    ma_engine_config cfg = ma_engine_config_init();
    cfg.periodSizeInFrames = 256;
    if (ma_engine_init(&cfg, &g_engine) != MA_SUCCESS) {
        MessageBoxW(NULL, L"音频引擎初始化失败。", L"Key Sound", MB_OK | MB_ICONERROR);
        GdiplusShutdown(gdiplusToken);
        return 1;
    }
    ma_engine_set_volume(&g_engine, g_volume);

    // 加载方案：优先 exe 目录，读不到再读 APPDATA
    char exeSoundsDir[MAX_PATH];
    GetExeSoundsDir(exeSoundsDir, MAX_PATH);
    LoadAllPacks(exeSoundsDir);

    if (g_packs.empty()) {
        char appDataDir[MAX_PATH];
        GetAppDataSoundsDir(appDataDir, MAX_PATH);
        CreateDirectoryA(appDataDir, NULL);
        LoadAllPacks(appDataDir);
    }

    if (!g_packs.empty()) {
        g_currentPack = 0;
    }

    g_autoStart = IsAutoStartEnabled();

    // 图标
    {
        wchar_t exeDirW[MAX_PATH];
        GetExeDirW(exeDirW, MAX_PATH);

        wchar_t icoPath[MAX_PATH];
        swprintf_s(icoPath, MAX_PATH, L"%s\\app.ico", exeDirW);

        g_appIcon = (HICON)LoadImageW(
            NULL, icoPath, IMAGE_ICON,
            GetSystemMetrics(SM_CXICON),
            GetSystemMetrics(SM_CYICON),
            LR_LOADFROMFILE);
        if (!g_appIcon) g_appIcon = LoadIconW(NULL, IDI_APPLICATION);

        g_trayIcon = (HICON)LoadImageW(
            NULL, icoPath, IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON),
            LR_LOADFROMFILE);
        if (!g_trayIcon) g_trayIcon = LoadIconW(NULL, IDI_APPLICATION);
    }

    LayoutControls();
    LayoutPackChips();

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = L"KeySoundMainWnd";
    wc.hIcon         = g_appIcon;
    RegisterClassW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - WINDOW_W) / 2;
    int posY = (screenH - WINDOW_H) / 2;

    g_hwnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"KeySoundMainWnd",
        L"Key Sound",
        WS_POPUP,
        posX, posY, WINDOW_W, WINDOW_H,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hwnd) {
        FreeAllPacks();
        ma_engine_uninit(&g_engine);
        GdiplusShutdown(gdiplusToken);
        return 1;
    }

    {
        int pref = 2;
        DwmSetWindowAttribute(g_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
                              &pref, sizeof(pref));
    }
    {
        BOOL useDark = FALSE;
        DwmSetWindowAttribute(g_hwnd, 20, &useDark, sizeof(useDark));
        DwmSetWindowAttribute(g_hwnd, 19, &useDark, sizeof(useDark));
    }

    bool isStartupLaunch = false;
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv) {
            for (int i = 1; i < argc; ++i) {
                if (_wcsicmp(argv[i], L"--startup") == 0) {
                    isStartupLaunch = true;
                    break;
                }
            }
            LocalFree(argv);
        }
    }

    ShowWindow(g_hwnd, isStartupLaunch ? SW_HIDE : SW_SHOW);
    UpdateWindow(g_hwnd);

    g_nid.cbSize           = sizeof(g_nid);
    g_nid.hWnd             = g_hwnd;
    g_nid.uID              = 1;
    g_nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon            = g_trayIcon;
    wcscpy_s(g_nid.szTip, _countof(g_nid.szTip), L"Key Sound - 按键音效");
    Shell_NotifyIconW(NIM_ADD, &g_nid);

    g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc,
                               GetModuleHandleW(NULL), 0);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_hook) UnhookWindowsHookEx(g_hook);
    Shell_NotifyIconW(NIM_DELETE, &g_nid);

    FreeAllPacks();

    if (g_appIcon && g_appIcon != LoadIconW(NULL, IDI_APPLICATION)) DestroyIcon(g_appIcon);
    if (g_trayIcon && g_trayIcon != LoadIconW(NULL, IDI_APPLICATION)) DestroyIcon(g_trayIcon);

    ma_engine_uninit(&g_engine);
    GdiplusShutdown(gdiplusToken);

    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    return 0;
}