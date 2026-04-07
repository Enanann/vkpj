[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

[[vk_binding(0, 0)]]
Texture2D<float4> inputImage;

[[vk_binding(1, 0)]]
[[vk_image_format("rgba32f")]]
RWTexture2D<float4> outputImage;

static float radius = 0.8;    // 0 to 1 
static float intensity = 1.0; // 0 to 1

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 threadId: SV_DispatchThreadID)
{
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (threadId.x >= size.x || threadId.y >= size.y) return;

    int2 texelCoord = int2(threadId.xy);

    float2 u = float2(texelCoord) / float2(size);

    float4 color = inputImage.Load(int3(texelCoord, 0));

    // Normalized distance from center
    float dist = sqrt(pow(u.x - 0.5, 2) + pow(u.y - 0.5, 2));

    // Vignette mask
    float mask = saturate(1.0 - dist * radius);
    mask = 1.0 - (1.0 - mask) * intensity;

    outputImage[texelCoord] = color * mask;
}
