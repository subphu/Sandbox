//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../libraries/imgui/imgui.h"
#include "../libraries/imgui/backends/imgui_impl_glfw.h"
#include "../libraries/imgui/backends/imgui_impl_vulkan.h"

#include "../include.h"
#include "../renderer/commander.hpp"

class IMGUI {
    
public:
    
    static VkDescriptorPool CreateDescPool(VkDevice device) {
        // the size of the pool is very oversize, but it's copied from imgui demo itself.
        VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER                , 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE          , 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE          , 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   , 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   , 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER         , 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER         , 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC , 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC , 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       , 1000 }
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000;
        poolInfo.poolSizeCount = std::size(pool_sizes);
        poolInfo.pPoolSizes = pool_sizes;

        VkDescriptorPool imguiDescPool;
        VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiDescPool);
        CHECK_VKRESULT(result, "failed to create descriptor pool!");
        return imguiDescPool;
    }
    
    static void CreateFontsTexture(Commander* pCommander) {
        VkCommandBuffer commandBuffer = pCommander->createCommandBuffer();
        pCommander->beginSingleTimeCommands(commandBuffer);
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        pCommander->endSingleTimeCommands(commandBuffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
    
};
