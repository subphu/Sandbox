//  Copyright © 2022 Subph. All rights reserved.
//

#include "files.hpp"

Files::~Files() {}
Files::Files() {}

void Files::cleanup() { m_cleaner.flush("Files"); }

VECTOR<Image*> Files::getTexturePreviews() {
    if (m_textureSamples.size() > 0) return m_textureSamples;
    for (STRING name : TEXTURE_NAMES) {
        STRING path = PBR_PATH + name + "/" + name + TEXTURE_PREV_PATH;
        Image* pTexture = new Image();
        pTexture->setupForTexture(path);
        pTexture->createWithSampler();
        pTexture->cmdCopyRawDataToImage();
        pTexture->cmdTransitionToShaderR();
        m_textureSamples.push_back(pTexture);
        m_cleaner.push([=](){ m_textureSamples.back()->cleanup(); m_textureSamples.pop_back(); });
    }
    return m_textureSamples;
}

VECTOR<Image*> Files::getCubemapPreviews() {
    if (m_cubemapSamples.size() > 0) return m_cubemapSamples;
    for (STRING name : CUBEMAP_NAMES) {
        STRING path = CUBE_PATH + name + "/" + name + CUBEMAP_PREV_PATH;
        Image* pERect = new Image();
        pERect->setupForTexture(path);
        pERect->createWithSampler();
        pERect->cmdCopyRawDataToImage();
        pERect->cmdTransitionToShaderR();
        m_cubemapSamples.push_back(pERect);
        m_cleaner.push([=](){ m_cubemapSamples.back()->cleanup(); m_cubemapSamples.pop_back(); });
    }
    return m_cubemapSamples;
}

void Files::setTextureIdx(uint idx) { m_textureIdx = idx; }
void Files::setCubemapIdx(uint idx) { m_cubemapIdx = idx; }

uint Files::getTotalCubemap() { return UINT32(CUBEMAP_NAMES.size()); }
STRING Files::getCubemapName() { return CUBEMAP_NAMES[m_cubemapIdx] + "/" + CUBEMAP_NAMES[m_cubemapIdx]; }
STRING Files::getCubemapHDRPath() { return CUBE_PATH + getCubemapName() + CUBEMAP_HDR_PATH; }
STRING Files::getCubemapEnvPath() { return CUBE_PATH + getCubemapName() + CUBEMAP_ENV_PATH; }

uint Files::getTotalTexture() { return UINT32(TEXTURE_NAMES.size()); }
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
