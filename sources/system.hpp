//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

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
    
    bool   RunFluid  = true;
    UInt2D FluidSize = {800, 800};
    
    uint   OPDSample       = 65536;
    float  RefractiveIndex = 1.5f;
    
    // Lights
    int       TotalLight  = 4;
    float     Radiance    = 200.f;
    glm::vec2 Distance    = {8.f, 8.f};
    glm::vec4 LightColor  = {1.f, 1.f, 1.f, 1.f};
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
    Device*     m_pDevice     = nullptr;
    Commander*  m_pCommander  = nullptr;
    Settings*   m_pSettings   = new struct Settings();
    RenderTime* m_pRenderTime = new struct RenderTime();
    
    static Device*     Device    () { return Instance().m_pDevice;     }
    static Commander*  Commander () { return Instance().m_pCommander;  }
    static Settings*   Settings  () { return Instance().m_pSettings;   }
    static RenderTime* RenderTime() { return Instance().m_pRenderTime; }
    
    static void setDevice   (class Device*    device   ) { Instance().m_pDevice    = device; }
    static void setCommander(class Commander* commander) { Instance().m_pCommander = commander; }
    
    static System& Instance() {
        static System instance; // Guaranteed to be destroyed. Instantiated on first use.
        return instance;
    }

    
private:
    
    System() {}
    ~System() {}

    // C++ 03
    // ========
    // Don't forget to declare these two. You want to make sure they
    // are unacceptable otherwise you may accidentally get copies of
    // your singleton appearing.
    System(System const&);         // Don't Implement
    void operator=(System const&); // Don't implement
    
};

