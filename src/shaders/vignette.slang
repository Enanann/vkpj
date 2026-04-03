[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

[[vk_binding(0, 0)]]
Texture2D<float4> inputImage;

[[vk_binding(1, 0)]]
[[vk_image_format("rgba32f")]]
RWTexture2D<float4> outputImage;

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 threadId: SV_DispatchThreadID)
{
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (threadId.x >= size.x || threadId.y >= size.y) return;

    int2 texelCoord = int2(threadId.xy);

    float2 u = float2(texelCoord) / float2(size);
    u = saturate(u * (1.0 - u.yx));

    float4 color = inputImage.Load(int3(texelCoord, 0));

    float str = 25.0;
    float dia = 0.5;

    float v = max(u.x * u.y * str, 1e-5);
    float vignette = pow(v, dia);

    outputImage[texelCoord] = color * vignette;
}
