//  Copyright © 2021 Subph. All rights reserved.
//

#include "descriptor.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

Descriptor::~Descriptor() {}
Descriptor::Descriptor() : m_pDevice(System::Device()) {}

void Descriptor::cleanup() { m_cleaner.flush("Descriptor"); }

void Descriptor::setupLayout(uint id, uint count) {
    m_dataMap[id] = DescriptorSetData{ .id = id, .count = count };
}

void Descriptor::addLayoutBindings(uint id, uint binding, VkDescriptorType type, VkShaderStageFlags flags) {
    if (!m_dataMap.count(id)) setupLayout(id);
    if (!m_poolSizesMap.count(type)) m_poolSizesMap[type] = VkDescriptorPoolSize{ type, 0 };
    
    uint count = 1;
    
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding         = binding;
    layoutBinding.descriptorCount = count;
    layoutBinding.descriptorType  = type;
    layoutBinding.stageFlags      = flags;
    layoutBinding.pImmutableSamplers = nullptr;
    
    VkWriteDescriptorSet writeSet{};
    writeSet.sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstBinding      = binding;
    writeSet.descriptorCount = count;
    writeSet.descriptorType  = type;
    writeSet.dstArrayElement = 0;
    
    m_dataMap[id].layoutBindings.push_back(layoutBinding);
    m_dataMap[id].writeSets.push_back(writeSet);
    m_poolSizesMap[type].descriptorCount += m_dataMap[id].count;
}

void Descriptor::createLayout(uint id) {
    LOG("Descriptor::createLayout");
    VkDevice device = m_pDevice->getDevice();
    DescriptorSetData data = m_dataMap[id];
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = UINT32(data.layoutBindings.size());
    layoutInfo.pBindings    = data.layoutBindings.data();
    
    VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &data.layout);
    CHECK_VKRESULT(result, "failed to create descriptor set layout!");
    m_cleaner.push([=](){ vkDestroyDescriptorSetLayout(device, data.layout, nullptr); });
        
    m_dataMap[id] = data;
}

void Descriptor::createPool() {
    LOG("Descriptor::createPool");
    VkDevice device = m_pDevice->getDevice();
    DescriptorSetDataMap descDataMap = m_dataMap;
    VECTOR<VkDescriptorPoolSize> poolSizes = getPoolSizes();
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets       = getTotalDecriptorSets();
    poolInfo.poolSizeCount = UINT32(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();

    VkDescriptorPool pool;
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
    CHECK_VKRESULT(result, "failed to create descriptor pool!");
    m_cleaner.push([=](){ vkDestroyDescriptorPool(device, pool, nullptr); });
    
    m_pool = pool;
}

void Descriptor::allocate(uint id) {
    allocateDescriptorSet(&m_dataMap[id]);
}

void Descriptor::allocateAll() {
    DescriptorSetDataMap map = m_dataMap;
    DescriptorSetDataMap::iterator it;
    for (it = map.begin(); it != map.end(); ++it )
        allocateDescriptorSet(&it->second);
}

void Descriptor::setupPointerBuffer(uint id, uint setIdx, uint binding, VkDescriptorBufferInfo* pBufferInfo) {
    int idx = findWriteSetIdx(id, binding);
    m_dataMap[id].writeSets[idx].dstSet = m_dataMap[id].descriptorSets[setIdx];
    m_dataMap[id].writeSets[idx].pBufferInfo = pBufferInfo;
}

void Descriptor::setupPointerImage(uint id, uint setIdx, uint binding, VkDescriptorImageInfo* pImageInfo) {
    int idx = findWriteSetIdx(id, binding);
    m_dataMap[id].writeSets[idx].dstSet = m_dataMap[id].descriptorSets[setIdx];
    m_dataMap[id].writeSets[idx].pImageInfo = pImageInfo;
}

void Descriptor::update(uint id) {
    LOG("Descriptor::update");
    VkDevice device = m_pDevice->getDevice();
    VECTOR<VkWriteDescriptorSet> writeSets = m_dataMap[id].writeSets;
    vkUpdateDescriptorSets(device, UINT32(writeSets.size()), writeSets.data(), 0, nullptr);
}

VkDescriptorSetLayout Descriptor::getDescriptorLayout(uint id) {
    return m_dataMap[id].layout;
}

VkDescriptorSet Descriptor::getDescriptorSet(uint id) {
    return m_dataMap[id].descriptorSets[0];
}

VECTOR<VkDescriptorSet> Descriptor::getDescriptorSets(uint id) {
    return m_dataMap[id].descriptorSets;
}


// Private ==================================================


void Descriptor::allocateDescriptorSet(DescriptorSetData* data) {
    LOG("Descriptor::allocateData");
    VkDevice device = m_pDevice->getDevice();
    VkDescriptorPool pool = m_pool;
    
    VECTOR<VkDescriptorSetLayout> layouts(data->count, data->layout);
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = pool;
    allocInfo.descriptorSetCount = data->count;
    allocInfo.pSetLayouts        = layouts.data();
    
    data->descriptorSets.resize(data->count);
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, data->descriptorSets.data());
    CHECK_VKRESULT(result, "failed to allocate descriptor set!");
}

uint Descriptor::getTotalDecriptorSets() {
    uint total = 0;
    DescriptorSetDataMap map = m_dataMap;
    DescriptorSetDataMap::iterator it;
    for (it = map.begin(); it != map.end(); ++it )
        total += (it->second).count;
    return total;
}

VECTOR<VkDescriptorPoolSize> Descriptor::getPoolSizes() {
    VECTOR<VkDescriptorPoolSize> vec;
    DescriptorPoolSizeMap map = m_poolSizesMap;
    DescriptorPoolSizeMap::iterator it;
    for (it = map.begin(); it != map.end(); ++it )
        vec.push_back(it->second);
    return vec;
}

int Descriptor::findWriteSetIdx(uint id, uint binding) {
    VECTOR<VkWriteDescriptorSet> writeSets = m_dataMap[id].writeSets;
    for (uint i = 0; i < writeSets.size(); i++)
        if (writeSets[i].dstBinding == binding) return i;
    return -1;
}
