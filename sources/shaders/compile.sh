#!/usr/bin/env bash

spirv_dir="$SRCROOT/resources/spirv"
glslc_dir="$SRCROOT/libraries/vulkansdk-mac/bin"

shader_dir="$SRCROOT/sources/shaders"
compute_dir="$shader_dir/compute"
pbr_dir="$shader_dir/PBR"

echo "shader dir: $shader_dir"
echo "spirv dir: $spirv_dir"
echo "glslc dir: $glslc_dir"

$glslc_dir/glslc $shader_dir/shader.vert -o $spirv_dir/vert.spv
$glslc_dir/glslc $shader_dir/shader.frag -o $spirv_dir/frag.spv
$glslc_dir/glslc $shader_dir/shader.comp -o $spirv_dir/comp.spv

$glslc_dir/glslc $shader_dir/swapchain.vert -o $spirv_dir/swapchain.vert.spv
$glslc_dir/glslc $shader_dir/swapchain.frag -o $spirv_dir/swapchain.frag.spv

$glslc_dir/glslc $compute_dir/interference1d.comp -o $spirv_dir/interference1d.spv
#$glslc_dir/glslc $compute_dir/interference2d.comp -o $spirv_dir/interference2d.spv

$glslc_dir/glslc $pbr_dir/main1d.vert -o $spirv_dir/main1d.vert.spv
$glslc_dir/glslc $pbr_dir/main1d.frag -o $spirv_dir/main1d.frag.spv
#$glslc_dir/glslc $pbr_dir/main2d.vert -o $spirv_dir/main2d.vert.spv
#$glslc_dir/glslc $pbr_dir/main2d.frag -o $spirv_dir/main2d.frag.spv
#$glslc_dir/glslc $pbr_dir/manual.vert -o $spirv_dir/manual.vert.spv
#$glslc_dir/glslc $pbr_dir/manual.frag -o $spirv_dir/manual.frag.spv
