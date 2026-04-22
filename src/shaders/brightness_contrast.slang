[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

[[vk_binding(0, 0)]]
Texture2D<float4> inputImage;

[[vk_binding(1, 0)]]
[[vk_image_format("rgba32f")]]
RWTexture2D<float4> outputImage;

[[vk_push_constant]]
float2 pushConstant; // brightness, contrast

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 ThreadId: SV_DispatchThreadID) {
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (ThreadId.x >= size.x || ThreadId.y >= size.y) return;

    int2 texelCoord = int2(ThreadId.xy);

    float2 u = float2(texelCoord) / float2(size);

    float4 color = inputImage.Load(int3(texelCoord, 0));

    float brightness = pushConstant.x;
    float contrast   = pushConstant.y;

    float nr = contrast * (color.r - 0.5) + 0.5 + brightness;
    float ng = contrast * (color.g - 0.5) + 0.5 + brightness;
    float nb = contrast * (color.b - 0.5) + 0.5 + brightness;

    outputImage[texelCoord] = float4(nr, ng, nb, color.a);
}
