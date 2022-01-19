//  Copyright © 2022 Subph. All rights reserved.
//

#include "files.hpp"

Files::~Files() {}
Files::Files() {}

STRING Files::getCubemapName() { return CUBEMAP_NAMES[m_cubemapIdx] + "/" + CUBEMAP_NAMES[m_cubemapIdx]; }
STRING Files::getCubemapHDRPath() { return CUBE_PATH + getCubemapName() + CUBEMAP_HDR_PATH; }
STRING Files::getCubemapEnvPath() { return CUBE_PATH + getCubemapName() + CUBEMAP_ENV_PATH; }

STRING Files::getTextureName() { return TEXTURE_NAMES[m_textureIdx] + "/" + TEXTURE_NAMES[m_textureIdx]; }
STRING Files::getTextureAlbedoPath()    { return PBR_PATH + getTextureName() + TEXTURE_ALBEDO_PATH; }
STRING Files::getTextureAOPath()        { return PBR_PATH + getTextureName() + TEXTURE_AO_PATH; }
STRING Files::getTextureMetallicPath()  { return PBR_PATH + getTextureName() + TEXTURE_METALLIC_PATH; }
STRING Files::getTextureNormalPath()    { return PBR_PATH + getTextureName() + TEXTURE_NORMAL_PATH; }
STRING Files::getTextureRoughnessPath() { return PBR_PATH + getTextureName() + TEXTURE_ROUGHNESS_PATH; }
VECTOR<STRING> Files::getTexturePBRPaths(){
    return {
        getTextureAlbedoPath(),
        getTextureAOPath(),
        getTextureMetallicPath(),
        getTextureNormalPath(),
        getTextureRoughnessPath() };
}
