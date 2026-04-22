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

[vk_push_constant]
float2 pushConstant;

static float PI_2 = 3.1415926538 * 2.0;

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 ThreadId: SV_DispatchThreadID) {
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (ThreadId.x >= size.x || ThreadId.y >= size.y) return;

    int2 texelCoord = int2(ThreadId.xy);

    float4 color = inputImage.Load(int3(texelCoord, 0));

    uint2 seed = uint2(pushConstant.x * 10000);
    float2 sample = float2(
        max(hash12(ThreadId.xy + seed), 1e-6),
        max(hash12(ThreadId.xy + seed + int2(10, 12)), 1e-6)
    );
    float  r = sqrt(-2.0 * log(sample.x));

    float z0 = r * cos(PI_2 * sample.y);
    // float z1 = r * sin(PI_2 * sample.y);

    float3 noise = float3(z0, z0, z0) * pushConstant.y;
    outputImage[texelCoord] = saturate(float4(color.rgb + noise, color.a));
    // outputImage[texelCoord] = float4(z0, z0, z0, 1.0);
}
