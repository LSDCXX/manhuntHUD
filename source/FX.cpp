#include "plugin.h"
#include "CSprite2d.h"
#include "CTimer.h"
#include "CTxdStore.h"
#include <string>

using namespace plugin;

#define SCREEN_WIDTH ((float)RsGlobal.maximumWidth)
#define SCREEN_HEIGHT ((float)RsGlobal.maximumHeight)

static std::string GetPluginPath()
{
    char buffer[MAX_PATH];
    HMODULE hm = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&GetPluginPath, &hm);
    GetModuleFileNameA(hm, buffer, MAX_PATH);
    std::string path(buffer);
    return path.substr(0, path.find_last_of("\\/"));
}

class FX {
public:
    // Master switch (FX=0 disables everything below)
    static inline bool Enabled = true;
    // Independent overlays
    static inline bool CrtEnabled = true;
    static inline bool GrainEnabled = true;
    // 0-100
    static inline int CrtStrength = 100;
    // 0-255 overlay alpha (legacy default was 25)
    static inline int GrainStrength = 25;

    static void LoadConfig()
    {
        char path[MAX_PATH];
        sprintf(path, "%s\\ManhuntHud.SA.ini", GetPluginPath().c_str());
        Enabled = GetPrivateProfileIntA("Settings", "FX", 1, path) != 0;
        CrtEnabled = GetPrivateProfileIntA("Settings", "CRT", 1, path) != 0;
        GrainEnabled = GetPrivateProfileIntA("Settings", "FilmGrain", 1, path) != 0;
        CrtStrength = GetPrivateProfileIntA("Settings", "CRTStrength", 100, path);
        GrainStrength = GetPrivateProfileIntA("Settings", "GrainStrength", 25, path);

        if (CrtStrength < 0) CrtStrength = 0;
        if (CrtStrength > 100) CrtStrength = 100;
        if (GrainStrength < 0) GrainStrength = 0;
        if (GrainStrength > 255) GrainStrength = 255;
    }
};

class CRT {
public:
    static void Draw()
    {
        if (!FX::Enabled || !FX::CrtEnabled || FX::CrtStrength <= 0) return;

        float resScale = SCREEN_HEIGHT / 1080.0f;
        if (resScale < 0.25f) resScale = 0.25f;

        // Scale scan pattern with resolution so 1080p/4K look consistent
        float spacing = 4.5f * resScale;
        if (spacing < 2.0f) spacing = 2.0f;
        float thickness = 2.9f * resScale;
        if (thickness < 1.0f) thickness = 1.0f;

        // Legacy base alpha was 12 at 100% strength
        unsigned char alpha = (unsigned char)((12 * FX::CrtStrength) / 100);
        if (alpha == 0) return;

        float offset = fmod((float)CTimer::m_snTimeInMilliseconds * 0.09f * resScale, spacing);

        for (float y = offset; y < SCREEN_HEIGHT; y += spacing)
        {
            CSprite2d::DrawRect(CRect(0.0f, y, SCREEN_WIDTH, y + thickness), CRGBA(0, 0, 0, alpha));
        }
    }
};

class FilmGrain {
public:
    static inline float Scale = 0.8f;
    static inline CSprite2d Sprite;

    static void Draw()
    {
        if (!FX::Enabled || !FX::GrainEnabled || FX::GrainStrength <= 0 || !Sprite.m_pTexture) return;

        float tile = 256.0f * Scale;
        float xo = (float)(rand() % (int)tile) - tile;
        float yo = (float)(rand() % (int)tile) - tile;

        float u1 = 0, u2 = 1, v1 = 0, v2 = 1;
        int r = rand() % 4;
        if (r == 1) std::swap(u1, u2);
        if (r == 2) std::swap(v1, v2);
        if (r == 3) { std::swap(u1, u2); std::swap(v1, v2); }

        unsigned char strength = (unsigned char)FX::GrainStrength;

        for (float x = xo; x < SCREEN_WIDTH; x += tile)
        {
            for (float y = yo; y < SCREEN_HEIGHT; y += tile)
            {
                if (x + tile < 0 || y + tile < 0) continue;
                Sprite.Draw(CRect(x, y, x + tile, y + tile), CRGBA(255, 255, 255, strength), u1, v1, u2, v1, u1, v2, u2, v2);
            }
        }
    }

    static void LoadTexture()
    {
        char path[MAX_PATH];
        sprintf(path, "%s\\ManhuntHud.SA\\mhud.txd", GetPluginPath().c_str());
        int slot = CTxdStore::AddTxdSlot("fx_grain");
        if (CTxdStore::LoadTxd(slot, path))
        {
            CTxdStore::PushCurrentTxd();
            CTxdStore::SetCurrentTxd(slot);
            Sprite.SetTexture((char*)"noise");
            CTxdStore::PopCurrentTxd();
        }
    }
};

class FXPlugin {
public:
    FXPlugin()
    {
        Events::initGameEvent += [] {
            FX::LoadConfig();
            };

        Events::initRwEvent += [] {
            FilmGrain::LoadTexture();
            };

        Events::drawingEvent += [] {
            CRT::Draw();
            FilmGrain::Draw();
            };
    }
} fx;
