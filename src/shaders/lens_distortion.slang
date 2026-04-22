[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

[vk_binding(0, 0)]
Texture2D<float4> inputImage;

[vk_binding(1, 0)]
[vk_image_format("rgba32f")]
RWTexture2D<float4> outputImage;

[vk_push_constant]
float4 pushConstant; // k1, k2, k3, zoom

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 ThreadId: SV_DispatchThreadID) {
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (ThreadId.x >= size.x || ThreadId.y >= size.y) return;

    int2 texelCoord = int2(ThreadId.xy);

    float k1 = pushConstant.x;
    float k2 = pushConstant.y;
    float k3 = pushConstant.z;
    float zoom = pushConstant.w;

    // normalized to [-0.5, 0.5]
    float2 uv = float2(texelCoord) / float2(size) - 0.5;
    float2 center_uv = float2(0.0, 0.0);

    float r2 = dot(center_uv - uv, center_uv - uv);
    float r4 = r2 * r2;
    float r6 = r2 * r2 * r2;

    float factor = 1 + k1 * r2 + k2 * r4 + k3 * r6;

    float x_d = uv.x * factor / zoom + 0.5;
    float y_d = uv.y * factor / zoom + 0.5;

    outputImage[texelCoord] = inputImage.Load(int3(int(x_d * size.x), int(y_d * size.y), 0));
    // outputImage[texelCoord] = saturate(inputImage.Load(int3(texelCoord, 0)) + k1);
}
