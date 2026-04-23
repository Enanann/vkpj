[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

float3 med(float3 colors[3]) {
    return fmedian3(colors[0], colors[1], colors[2]);
}

[vk_binding(0, 0)]
Texture2D<float4> inputImage;

[vk_binding(1, 0)]
[vk_image_format("rgba32f")]
RWTexture2D<float4> outputImage;

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 ThreadId: SV_DispatchThreadID) {
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (ThreadId.x >= size.x || ThreadId.y >= size.y) return;

    int2 texelCoord = int2(ThreadId.xy);

    float4 color = inputImage.Load(int3(texelCoord, 0));

    static const int2 offsets[9] = {
        int2(-1, -1), int2(0, -1), int2(1, -1),
        int2(-1, 0), int2(0, 0), int2(1, 0),
        int2(-1, 1), int2(0, 1), int2(1, 1)
    };

    float3 color_r1 = med({
        inputImage.Load(int3(texelCoord + offsets[0], 0)).rgb,
        inputImage.Load(int3(texelCoord + offsets[1], 0)).rgb,
        inputImage.Load(int3(texelCoord + offsets[2], 0)).rgb
    });
    float3 color_r2 = med({
        inputImage.Load(int3(texelCoord + offsets[3], 0)).rgb,
        inputImage.Load(int3(texelCoord + offsets[4], 0)).rgb,
        inputImage.Load(int3(texelCoord + offsets[5], 0)).rgb
    });
    float3 color_r3 = med({
        inputImage.Load(int3(texelCoord + offsets[6], 0)).rgb,
        inputImage.Load(int3(texelCoord + offsets[7], 0)).rgb,
        inputImage.Load(int3(texelCoord + offsets[8], 0)).rgb
    });

    float3 fn = med({color_r1, color_r2, color_r3});

    outputImage[texelCoord] = float4(fn, color.a);
}
