[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

#define s(a, b) { float3 temp = a; a = min(a, b); b = max(temp, b);}

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

    float3 v[9];

    v[0] = inputImage.Load(int3(texelCoord + offsets[0], 0)).rgb;
    v[1] = inputImage.Load(int3(texelCoord + offsets[1], 0)).rgb;
    v[2] = inputImage.Load(int3(texelCoord + offsets[2], 0)).rgb;
    v[3] = inputImage.Load(int3(texelCoord + offsets[3], 0)).rgb;
    v[4] = inputImage.Load(int3(texelCoord + offsets[4], 0)).rgb;
    v[5] = inputImage.Load(int3(texelCoord + offsets[5], 0)).rgb;
    v[6] = inputImage.Load(int3(texelCoord + offsets[6], 0)).rgb;
    v[7] = inputImage.Load(int3(texelCoord + offsets[7], 0)).rgb;
    v[8] = inputImage.Load(int3(texelCoord + offsets[8], 0)).rgb;

    // bubble sort
    // s(v[0], v[1]); s(v[1], v[2]); s(v[2], v[3]); s(v[3], v[4]); s(v[4], v[5]); s(v[5], v[6]); s(v[6], v[7]); s(v[7], v[8]);
    // s(v[0], v[1]); s(v[1], v[2]); s(v[2], v[3]); s(v[3], v[4]); s(v[4], v[5]); s(v[5], v[6]); s(v[6], v[7]);
    // s(v[0], v[1]); s(v[1], v[2]); s(v[2], v[3]); s(v[3], v[4]); s(v[4], v[5]); s(v[5], v[6]);
    // s(v[0], v[1]); s(v[1], v[2]); s(v[2], v[3]); s(v[3], v[4]); s(v[4], v[5]);
    // s(v[0], v[1]); s(v[1], v[2]); s(v[2], v[3]); s(v[3], v[4]);
    // s(v[0], v[1]); s(v[1], v[2]); s(v[2], v[3]);
    // s(v[0], v[1]); s(v[1], v[2]);
    // s(v[0], v[1]);

    // https://gist.github.com/yamamaya/3e7691b7f129fa0c47e1fe9c3851d2f4     
    s(v[0], v[1]);
    s(v[3], v[4]);
    s(v[6], v[7]);
    s(v[1], v[2]);
    s(v[4], v[5]);
    s(v[7], v[8]);
    s(v[0], v[1]);
    s(v[3], v[4]);
    s(v[6], v[7]);
    s(v[0], v[3]);
    s(v[3], v[6]);
    s(v[0], v[3]);
    s(v[1], v[4]);
    s(v[4], v[7]);
    s(v[1], v[4]);
    s(v[2], v[5]);
    s(v[5], v[8]);
    s(v[2], v[5]);
    s(v[1], v[3]);
    s(v[5], v[7]);
    s(v[2], v[6]);
    s(v[4], v[6]);
    s(v[2], v[4]);
    s(v[2], v[3]);
    s(v[5], v[6]);

    outputImage[texelCoord] = float4(v[4], color.a);
}
