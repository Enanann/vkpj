[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

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

    static const float M_x[9] = {
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0 
    };
    static const float M_y[9] = {
        -1.0, -2.0, -1.0,
        0.0, 0.0, 0.0,
        1.0, 2.0, 1.0
    };

    static const int2 offsets[9] = {
        int2(-1, -1),  int2(0, -1),  int2(1, -1),   
        int2(-1,  0),  int2(0,  0),  int2(1,  0),        
        int2(-1,  1),  int2(0,  1),  int2(1,  1)   
    };

    float3 colors[9];
    [unroll]
    for (int i = 0; i < 9; ++i) {
        int2 coord = clamp(texelCoord + offsets[i], int2(0, 0), size - 1);
        colors[i] = inputImage.Load(int3(coord, 0)).rgb;
    }

    float3 res1 = float3(0.0);
    float3 res2 = float3(0.0);
    [unroll]
    for (int i = 0; i < 9; ++i) {
        res1 += colors[i] * M_x[i];
        res2 += colors[i] * M_y[i];
    }

    float3 res = sqrt(res1 * res1 + res2 * res2);

    outputImage[texelCoord] = float4(res, 1.0);
}
