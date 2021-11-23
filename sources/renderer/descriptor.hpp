//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"

enum Set     { S0 = 0, S1 = 1, S2 = 2, S3 = 3, S4 = 4, S5 = 5, S6 = 6, S7 = 7 };
enum SetIdx  { I0 = 0, I1 = 1, I2 = 2, I3 = 3, I4 = 4, I5 = 5, I6 = 6, I7 = 7 };
enum Binding { B0 = 0, B1 = 1, B2 = 2, B3 = 3, B4 = 4, B5 = 5, B6 = 6, B7 = 7 };

class Descriptor {
    
public:
    Descriptor();
    ~Descriptor();
    
    void cleanup();
    
    void setupLayout(uint layoutId, uint count = 1);
    void createLayout(uint layoutId);
    void addLayoutBindings(uint layoutId, uint binding, VkDescriptorType type, VkShaderStageFlags flags);
    
    void createPool();
    
    void allocate(uint layoutId);
    void allocateAll();
    
    void setupPointerBuffer(uint set, uint binding, VkDescriptorBufferInfo* pBufferInfo);
    void setupPointerBuffer(uint set, uint setIdx, uint binding, VkDescriptorBufferInfo* pBufferInfo);
    void setupPointerImage(uint set, uint binding, VkDescriptorImageInfo* pImageInfo);
    void setupPointerImage(uint set, uint setIdx, uint binding, VkDescriptorImageInfo* pImageInfo);
    void update(uint layoutId);
    
    VkDescriptorSetLayout   getDescriptorLayout(uint layoutId);
    VkDescriptorSet         getDescriptorSet(uint layoutId);
    VECTOR<VkDescriptorSet> getDescriptorSets(uint layoutId);
    
private:
    
    struct DescriptorSetData {
        uint set;
        uint count = 1;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VECTOR<VkDescriptorSet> descriptorSets;
        VECTOR<VkDescriptorSetLayoutBinding> layoutBindings;
        VECTOR<VkWriteDescriptorSet> writeSets;
    };
    
    typedef std::map<VkDescriptorType, VkDescriptorPoolSize> DescriptorPoolSizeMap;
    typedef std::map<uint            , DescriptorSetData   > DescriptorSetDataMap;
    
    Cleaner m_cleaner;
    Device* m_pDevice;
    
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    
    DescriptorPoolSizeMap m_poolSizesMap;
    DescriptorSetDataMap  m_dataMap;
    
    void allocateDescriptorSet(DescriptorSetData* data);
    
    uint getTotalDecriptorSets();
    VECTOR<VkDescriptorPoolSize> getPoolSizes();
    
    int findWriteSetIdx(uint id, uint binding);
};
