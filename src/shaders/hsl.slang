[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

#include "utils.slang"

[[vk_binding(0, 0)]]
Texture2D<float4> inputImage;

[[vk_binding(1, 0)]]
[[vk_image_format("rgba32f")]]
RWTexture2D<float4> outputImage;

[[vk_push_constant]]
float3 pushConstant; // [hue, saturation, lightness]

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 threadId: SV_DispatchThreadID) {
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (threadId.x >= size.x || threadId.y >= size.y) return;

    int2 texelCoord = int2(threadId.xy);

    float4 color = inputImage.Load(int3(texelCoord, 0));

    float3 hsl = RgbToHsl(float3(color.rgb));
    hsl.x = fract(hsl.x + pushConstant.x);
    // hsl.y = clamp(hsl.y + pushConstant.y, 0.0, 1.0);
    hsl.z = clamp(hsl.z + pushConstant.z, 0.0, 1.0);

    color.rgb = HslToRgb(hsl);

    float gray = dot(color.rgb, float3(0.299, 0.587, 0.114));
    color.rgb = saturate(lerp(float3(gray), color.rgb, (pushConstant.y + 1)));

    outputImage[texelCoord] = color;
}
