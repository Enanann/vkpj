[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

[vk_binding(0, 0)]
Sampler2D inputImage;
// Texture2D<float4> inputImage;

[vk_binding(1, 0)]
RWTexture2D<float4> outputImage;

[vk_push_constant]
float pc;

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 ThreadId: SV_DispatchThreadID) {
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (ThreadId.x >= size.x || ThreadId.y >= size.y) return;

    int2 texelCoord = int2(ThreadId.xy);

    float2 uv = float2(texelCoord) / float2(size);
    float dis = distance(uv, float2(0.5));
    float2 dir = normalize(uv - float2(0.5) + 1e-6);

    float rOffset = pc * dis * 0.5;
    float gOffset = pc * dis * 0.7;
    float bOffset = pc * dis * 0.9;

    float2 r_uv = ((uv + rOffset * dir) - float2(0.5)) / lerp(1.0, 1.8, pc) + float2(0.5);
    float2 g_uv = ((uv + gOffset * dir) - float2(0.5)) / lerp(1.0, 1.8, pc) + float2(0.5);
    float2 b_uv = ((uv + bOffset * dir) - float2(0.5)) / lerp(1.0, 1.8, pc) + float2(0.5);

    float4 color_r = inputImage.SampleLevel(r_uv, 0);
    float4 color_g = inputImage.SampleLevel(g_uv, 0);
    float4 color_b = inputImage.SampleLevel(b_uv, 0);

    outputImage[texelCoord] = float4(color_r.r, color_g.g, color_b.b, color_r.a);
}
