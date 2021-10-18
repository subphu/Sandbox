//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include "../libraries/stb_image/stb_image.h"
#include "include.h"

class STBI {
    
public:
    
    static unsigned char* LoadImage(const std::string filename, int* width, int* height, int* channels) {
        unsigned char *data = stbi_load(filename.c_str(), width, height, channels, STBI_rgb_alpha);
        if (data) return data;
        PRINTLN2("failed to load image ", filename);
        return nullptr;
    }

    static float* LoadHDR(const std::string filename, int* width, int* height, int* channels) {
        float *data = stbi_loadf(filename.c_str(), width, height, channels, STBI_default);
        if (data) return data;
        PRINTLN2("failed to load hdr image ", filename);
        return nullptr;
    }
    
};
