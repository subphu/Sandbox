//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "device.hpp"

class Image {
    
public:
    ~Image();
    Image();

    void cleanup();
    
    void setupForDepth      (UInt2D size);
    void setupForColor      (UInt2D size);
    void setupForSwapchain  (VkImage image, VkFormat imageFormat);
    void setupForTexture    (const std::string filepath);
    void setupForHDRTexture (const std::string filepath);
    void setupForCubemap    (const std::string *filepaths);
    void setupForCubemap    (UInt2D size);
    
    void create              ();
    void createForTexture    ();
    void createForSwapchain  ();
     
    void createImage         ();
    void createImageView     ();
    void allocateImageMemory ();
    void createDescriptorInfo();
    void createSampler       ();
    
    void cmdCopyRawHDRToImage ();
    void cmdCopyRawDataToImage();
    void cmdCopyCubemapToImage();
    
    void cmdTransitionToTransferDest();
    void cmdTransitionPresentToShader(VkCommandBuffer cmdBuffer);
    void cmdTransitionShaderToPresent(VkCommandBuffer cmdBuffer);
    
    void cmdCopyImageToImage        (Image* srcImage);
    void cmdCopyBufferToImage       (VkBuffer buffer);
    void cmdGenerateMipmaps         ();
    
    VkImage          getImage      ();
    VkImageView      getImageView  ();
    VkDeviceMemory   getImageMemory();
    VkDeviceSize     getImageSize  ();
    VkSampler        getSampler    ();
    unsigned int     getChannelSize();
    VkDescriptorImageInfo* getDescriptorInfo();
    
    VkImageCreateInfo     getImageInfo();
    VkImageViewCreateInfo getImageViewInfo();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    
    unsigned char* m_desc;
    unsigned char* m_rawData;
    float        * m_rawHDR;
    VECTOR<unsigned char*> m_rawCubemap;

    VkImage          m_image          = VK_NULL_HANDLE;
    VkImageView      m_imageView      = VK_NULL_HANDLE;
    VkDeviceMemory   m_imageMemory    = VK_NULL_HANDLE;
    VkDescriptorImageInfo m_descriptorInfo{};
    
    VkImageCreateInfo     m_imageInfo{};
    VkImageViewCreateInfo m_imageViewInfo{};
    
    // For Texture
    VkSampler m_sampler = VK_NULL_HANDLE;
    
    uint32_t MaxMipLevel(int width, int height);
    static unsigned int GetChannelSize(VkFormat format);
    static VkImageCreateInfo     GetDefaultImageCreateInfo();
    static VkImageViewCreateInfo GetDefaultImageViewCreateInfo();
    static VkImageMemoryBarrier  GetDefaultImageMemoryBarrier();
    
};
