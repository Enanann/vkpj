[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

#include "utils.slang"

[vk_binding(0, 0)]
Texture2D<float4> inputImage;

[vk_binding(1, 0)]
[vk_image_format("rgba32f")]
RWTexture2D<float4> outputImage;

struct pushConstants {
    int p;
    float seed;
    float intensity;
};
[vk_push_constant]
pushConstants pc;

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 ThreadId: SV_DispatchThreadID) {
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (ThreadId.x >= size.x || ThreadId.y >= size.y) return;

    int2 texelCoord = int2(ThreadId.xy);

    float4 color = inputImage.Load(int3(texelCoord, 0));

    uint2 seed = uint2(pc.seed * 10000);
    float u = hash12(ThreadId.xy + seed);

    float P = pc.intensity;

    float3 noise = color.rgb;

    if (u < P / 2.0) {
        noise = float3(0.0);
    } else if (u < P) {
        noise = float3(1.0);
    }

    outputImage[texelCoord] = float4(noise, color.a);
}
