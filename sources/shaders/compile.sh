#!/usr/bin/env bash

spirv_dir="$SRCROOT/resources/spirv"
glslc_dir="$SRCROOT/libraries/vulkansdk-mac/bin"

shader_dir="$SRCROOT/sources/shaders"
compute_dir="$shader_dir/compute"

echo "shader dir: $shader_dir"
echo "spirv dir: $spirv_dir"
echo "glslc dir: $glslc_dir"

$glslc_dir/glslc $shader_dir/shader.vert -o $spirv_dir/vert.spv
$glslc_dir/glslc $shader_dir/shader.frag -o $spirv_dir/frag.spv
$glslc_dir/glslc $shader_dir/shader.comp -o $spirv_dir/comp.spv

$glslc_dir/glslc $shader_dir/swapchain.vert -o $spirv_dir/swapchain.vert.spv
$glslc_dir/glslc $shader_dir/swapchain.frag -o $spirv_dir/swapchain.frag.spv

$glslc_dir/glslc $compute_dir/interference1d.comp -o $spirv_dir/interference1d.spv
$glslc_dir/glslc $compute_dir/interference2d.comp -o $spirv_dir/interference2d.spv
