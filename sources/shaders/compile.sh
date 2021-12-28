#!/usr/bin/env bash

spirv_dir="$SRCROOT/resources/spirv"
glslc_dir="$SRCROOT/libraries/vulkansdk-mac/bin"

shader_dir="$SRCROOT/sources/shaders"
compute_dir="$shader_dir/compute"
pbr_dir="$shader_dir/pbr"
cubemap_dir="$shader_dir/cubemap"

echo "shader dir: $shader_dir"
echo "spirv dir: $spirv_dir"
echo "glslc dir: $glslc_dir"

#interference2d.comp
#main2d.vert
#main2d.frag
#manual.vert
#manual.frag

shader_folder=(
    $shader_dir/
    $shader_dir/
                
    $compute_dir/
    $compute_dir/
    $compute_dir/
                
    $pbr_dir/
    $pbr_dir/
    $pbr_dir/
    $pbr_dir/
        
    $cubemap_dir/
    $cubemap_dir/
    $cubemap_dir/
    $cubemap_dir/
)

shader_names=(
    swapchain.vert
    swapchain.frag
        
    hdr.comp
    fluid.comp
    interference1d.comp
                
    cubemap.vert
    cubemap.frag
    main1d.vert
    main1d.frag
    
    equirect.vert
    equirect.frag
    prefilter.frag
    environment.frag
)

for i in ${!shader_folder[@]}; do
    $glslc_dir/glslc --target-env=vulkan1.1 ${shader_folder[$i]}${shader_names[$i]} -o $spirv_dir/${shader_names[$i]}.spv
done

