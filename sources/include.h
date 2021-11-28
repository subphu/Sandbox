//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <math.h>

// std
#include <iostream>
#include <vector>
#include <array>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>

#pragma clang diagnostic pop

#ifdef NDEBUG
#define IS_DEBUG false
#else
#define IS_DEBUG true
#endif

#define RUNTIME_ERROR(m) throw std::runtime_error(m)

#define CHECK_BOOL(    v, m) if(!v)                RUNTIME_ERROR(m)
#define CHECK_ZERO(    v, m) if(!v)                RUNTIME_ERROR(m)
#define CHECK_NEGATIVE(v, m) if(v<0)               RUNTIME_ERROR(m)
#define CHECK_HANDLE(  v, m) if(v==VK_NULL_HANDLE) RUNTIME_ERROR(m)
#define CHECK_NULLPTR( v, m) if(v==nullptr)        RUNTIME_ERROR(m)
#define CHECK_VKRESULT(r, m) if(r!=VK_SUCCESS)     RUNTIME_ERROR(m)
#define CHECK_MAXINT32(r, m) if(r==UINT32_MAX)     RUNTIME_ERROR(m)

#define USE_VAR(v) {}
#define INP_VAR(v) {}
#define OUT_VAR(v) {}
#define MOD_VAR(v) {}
#define USE_FUNC(f) {}

#define LOG(v) std::cout << "LOG::" << v << std::endl
#define ERR(v) std::cout << "ERR::" << v << std::endl

#define PRINT1(  v1            ) std::cout << v1
#define PRINT2(  v1, v2        ) PRINT1(v1        ) << " " << v2
#define PRINT3(  v1, v2, v3    ) PRINT2(v1, v2    ) << " " << v3
#define PRINT4(  v1, v2, v3, v4) PRINT3(v1, v2, v3) << " " << v4

#define PRINTLN1(v1            ) PRINT1(v1            ) << std::endl
#define PRINTLN2(v1, v2        ) PRINT2(v1, v2        ) << std::endl
#define PRINTLN3(v1, v2, v3    ) PRINT3(v1, v2, v3    ) << std::endl
#define PRINTLN4(v1, v2, v3, v4) PRINT4(v1, v2, v3, v4) << std::endl

#define UINT32(v) static_cast<uint32_t>(v)
#define VECTOR std::vector

#define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"

#define PI 3.14159265358979323846

#define CHANNEL 4

const std::string SPIRV_PATH = "resources/spirv/";
const std::string MODEL_PATH = "resources/models/";
const std::string PBR_PATH   = "resources/textures/PBR/";

template<typename T> struct Size { T width, height, depth; };

typedef VkExtent2D UInt2D;

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<float, std::chrono::seconds::period> TimeDif;

struct Cleaner {
    std::stack<std::function<void()>> stack;

    void push(std::function<void()>&& function) {
        stack.push(function);
    }

    void flush(std::string name = "") {
        printf ("Clean::%s::%lu process \n", name.c_str(), stack.size());
        while (!stack.empty()) {
            std::function<void()> cleaning = stack.top();
            cleaning();
            stack.pop();
        }
    }
};
