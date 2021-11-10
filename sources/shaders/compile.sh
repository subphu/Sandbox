#!/usr/bin/env bash

shader_dir="$SRCROOT/sources/shaders"
spirv_dir="$SRCROOT/resources/spirv"
glslc_dir="$SRCROOT/libraries/vulkansdk-mac/bin"

echo "shader dir: $shader_dir"
echo "spirv dir: $spirv_dir"
echo "glslc dir: $glslc_dir"

$glslc_dir/glslc $shader_dir/shader.vert -o $spirv_dir/vert.spv
$glslc_dir/glslc $shader_dir/shader.frag -o $spirv_dir/frag.spv
$glslc_dir/glslc $shader_dir/shader.comp -o $spirv_dir/comp.spv

$glslc_dir/glslc $shader_dir/swapchain.vert -o $spirv_dir/swapchain.vert.spv
$glslc_dir/glslc $shader_dir/swapchain.frag -o $spirv_dir/swapchain.frag.spv
