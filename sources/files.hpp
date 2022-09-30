//  Copyright © 2022 Subph. All rights reserved.
//

#pragma once

#include "include.h"
#include "resources/image.hpp"

const std::string PBR_PATH   = "resources/textures/pbr/";
const std::string CUBE_PATH  = "resources/textures/cubemap/";

class Files {
public:
    
    Files();
    ~Files();
    
    void cleanup();
    
    uint m_textureIdx = 0;
    uint m_cubemapIdx = 2;
    
    void setTextureIdx(uint idx);
    void setCubemapIdx(uint idx);
    
    VECTOR<Image*> m_textureSamples;
    VECTOR<Image*> m_cubemapSamples;
    
    uint   getTotalTexture();
    STRING getTextureName();
    STRING getTextureAlbedoPath();
    STRING getTextureAOPath();
    STRING getTextureMetallicPath();
    STRING getTextureNormalPath();
    STRING getTextureRoughnessPath();
    VECTOR<STRING> getTexturePBRPaths();
    VECTOR<Image*> getTexturePreviews();
    
    uint   getTotalCubemap();
    STRING getCubemapName();
    STRING getCubemapHDRPath();
    STRING getCubemapEnvPath();
    VECTOR<Image*> getCubemapPreviews();
    
private:
    Cleaner m_cleaner;
    
    const VECTOR<STRING> CUBEMAP_NAMES = {"Arches_E_PineTree", "GravelPlaza", "Tokyo_BigSight", "ArboretumInBloom", "Hamarikyu_Bridge_B", "hdrvfx_0011_zanla", "Tropical_Beach", "Ueno-Shrine"};
    const STRING CUBEMAP_HDR_PATH = ".hdr";
    const STRING CUBEMAP_ENV_PATH = "_Env.hdr";
    const STRING CUBEMAP_PREV_PATH = "_Preview.jpg";
    
    const VECTOR<STRING> TEXTURE_NAMES = { "worn-wet-old-cobblestone", "cobblestylized", "greasypan", "layered-rock1", "rustediron", "slipperystonework", "roughrockface", "slimy-slippery-rock1", "cliffrockface", "limestone6"};
    const STRING TEXTURE_ALBEDO_PATH    = "_albedo.png";
    const STRING TEXTURE_AO_PATH        = "_ao.png";
    const STRING TEXTURE_METALLIC_PATH  = "_metallic.png";
    const STRING TEXTURE_NORMAL_PATH    = "_normal.png";
    const STRING TEXTURE_ROUGHNESS_PATH = "_roughness.png";
    const STRING TEXTURE_PREV_PATH = "_albedo.jpg";
};

