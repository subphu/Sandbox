//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "device.hpp"
#include "commander.hpp"

class System {
    
public:
    Device*    m_pDevice    = nullptr;
    Commander* m_pCommander = nullptr;
    
    static Device*    Device   () { return Instance().m_pDevice;    }
    static Commander* Commander() { return Instance().m_pCommander; }
    
    static void setDevice   (class Device*    device   ) { Instance().m_pDevice = device; }
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
