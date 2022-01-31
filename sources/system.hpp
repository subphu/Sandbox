//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "files.hpp"
#include "device.hpp"
#include "commander.hpp"

struct Settings {
    bool ShowDemo  = false;
    bool LockFPS   = false;
    bool LockFocus = true;
    
    long Iteration = 0;
    
    glm::vec3 CameraPos = {};
    
    VkClearColorValue        ClearColor = {0.01f, 0.01f, 0.01f, 1.0f};
    VkClearDepthStencilValue ClearDepth = {1.0f, 0};
    uint  ClearStencil  = 0;
    
    bool   UseFluid  = false;
    bool   RunFluid  = true;
    UInt2D FluidSize = {800, 800};
    
    // Lights
    bool      LightMove   = true;
    int       TotalLight  = 4;
    float     Radiance    = 200.f;
    glm::vec2 Distance    = {8.f, 8.f};
    glm::vec4 LightColor  = {1.f, 1.f, 1.f, 1.f};
    
    // Interference
    uint   OPDSample        = 16384 / 2;
    uint   RSample          = 11;
    bool   Interference     = true;
    bool   PhaseShift       = false;
    float  ThicknessScale   = 1.f;
    float  RefractiveIndex  = 1.5f;
    float  ReflectanceValue = 0.5f;
    float  OPDOffset        = 0.f;
    
    // Materials
    glm::vec4 Albedo = {1.f, 1.f, 1.f, 1.f};
    float  Metallic  = 1.0;
    float  Roughness = 0.0;
    float  AO        = 1.0;
    
    // Objects
    bool   ObjectMove = false;
    bool   UseTexture = false;
    int    Textures  = 6;
    int    Cubemaps  = 1;
    int    Shapes    = 0;
    
    // Button
    bool BtnUpdateTexture = false;
    bool BtnUpdateCubemap = false;
    
};

struct RenderTime {
    TimeVal lastTime = ChronoTime::now();
    float frameDelay = 1.f/60.f;
    float lag = frameDelay;
    
    bool  checkLag    () { return lag < frameDelay; }
    float getSleepTime() { return (frameDelay - lag) * 1000000; }
    
    void updateTime   () { lastTime = ChronoTime::now(); };
    void startRender  () { lag -= frameDelay; };
    void addRenderTime() {
        lag += TimeDif(ChronoTime::now() - lastTime).count();
        updateTime();
    };
    void sleepIf(bool condition) {
        if (!condition || lag > frameDelay) return;
        usleep(getSleepTime());
        addRenderTime();
    }
};

class System {
    
public:
    Files*      m_pFiles      = nullptr;
    Device*     m_pDevice     = nullptr;
    Commander*  m_pCommander  = nullptr;
    Settings*   m_pSettings   = new struct Settings();
    RenderTime* m_pRenderTime = new struct RenderTime();
    
    static Files*      Files     () { return Instance().m_pFiles;     }
    static Device*     Device    () { return Instance().m_pDevice;     }
    static Commander*  Commander () { return Instance().m_pCommander;  }
    static Settings*   Settings  () { return Instance().m_pSettings;   }
    static RenderTime* RenderTime() { return Instance().m_pRenderTime; }
    
    static void initFiles() { Instance().m_pFiles = new class Files(); }
    
    static void setDevice   (class Device*    device   ) { Instance().m_pDevice    = device; }
    static void setCommander(class Commander* commander) { Instance().m_pCommander = commander; }
    
    static System& Instance() {
        static System instance; // Guaranteed to be destroyed. Instantiated on first use.
        return instance;
    }

    
private:
    
    ~System() {}
    System() {}

    // C++ 03
    // ========
    // Don't forget to declare these two. You want to make sure they
    // are unacceptable otherwise you may accidentally get copies of
    // your singleton appearing.
    System(System const&);         // Don't Implement
    void operator=(System const&); // Don't implement
    
};

