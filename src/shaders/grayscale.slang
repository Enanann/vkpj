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
    int2 texelCoord = int2(threadId.xy);

    // Read from the image
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);
    if (texelCoord.x >= size.x || texelCoord.y >= size.y) return;
    float4 value = inputImage.Load(int3(texelCoord, 0));
    
    // Modify the value
    float gray = dot(value.rgb, float3(0.299, 0.587, 0.114));

    // Write back to the image
    outputImage[texelCoord] = float4(float3(gray), value.a);
}
