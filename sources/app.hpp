//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "window.hpp"
#include "device.hpp"
#include "commander.hpp"

class App {
public:
    
    void run();

private:
    Window* m_pWindow;
    Device* m_pDevice;
    Commander* m_pCommander;
    
    
    void initWindow();
    void initDevice();
    void initCommander();
    
};

