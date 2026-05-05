[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

[vk_binding(0, 0)]
Sampler2D<float4> inputImage;

[vk_binding(1, 0)]
RWTexture2D<float4> outputImage;

[vk_push_constant]
float4 pc;

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 ThreadId: SV_DispatchThreadID) {
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (ThreadId.x >= size.x || ThreadId.y >= size.y) return;

    int2 texelCoord = int2(ThreadId.xy);
    float2 uv = float2(ThreadId.xy) / float2(size);

    float2 center = float2(size) * 0.5;

    float rotate = pc.x;
    float zoom = pc.y;               
    float2 pan = float2(pc.z, pc.w); // in pixels

    float c = cos(rotate);
    float s = sin(rotate);

    // move into centered space
    float2 pos = float2(ThreadId.xy) - center;

    // pan
    pos = pos - pan;

    // rotate
    float2 rotated = float2(
        pos.x * c - pos.y * s,
        pos.x * s + pos.y * c
    );

    // zoom
    rotated /= zoom;

    float2 finalPos = rotated + center;

    float2 finalUV = finalPos / float2(size);

    float4 color = inputImage.SampleLevel(finalUV, 0);

    outputImage[ThreadId.xy] = color;
}
