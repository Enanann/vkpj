[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

[vk_binding(0, 0)]
Sampler2D inputImage;

[vk_binding(2, 0)]
Sampler2D mask;

[vk_binding(1, 0)]
[vk_image_format("rgba32f")]
RWTexture2D<float4> outputImage;

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 threadId: SV_DispatchThreadID) {
    uint2 size;
    inputImage.GetDimensions(size.x, size.y);

    if (threadId.x >= size.x || threadId.y >= size.y) return;

    int2 texelCoord = int2(threadId.xy);
    float2 uv = float2(texelCoord) / float2(size);

    float4 color = mask.SampleLevel(uv, 0).r * inputImage.SampleLevel(uv, 0);

    outputImage[texelCoord] = color;
}
