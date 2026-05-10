[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

[vk_binding(0, 0)]
Sampler2D inputImage;

[vk_binding(1, 0)]
[vk_image_format("rgba32f")]
RWTexture2D<float4> outputImage;

struct pushConstants {
    int p;
};

[vk_push_constant]
pushConstants pc;

static const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
// static const float weight[15] = { 0.199471, 0.176033, 0.120985, 0.0647588, 0.0269955, 0.00876415, 0.00221592, 0.000436341, 6.69151e-05, 7.99187e-06, 7.4336e-07, 5.38488e-08, 3.03794e-09, 1.33478e-10, 4.56736e-12 };

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 threadId: SV_DispatchThreadID)
{
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (threadId.x >= size.x || threadId.y >= size.y) return;

    int2 texelCoord = int2(threadId.xy);

    float2 uv = (float2(texelCoord) + 0.5) / float2(size);

    float2 texel_offset = 1.0 / float2(size);
    float3 result = inputImage.SampleLevel(uv, 0).rgb * weight[0];

    if (pc.p % 2 == 1) { // horizontal
        for (int i = 1; i < weight.getCount(); ++i) {
            float2 offset = float2(texel_offset.x * i, 0.0);
            result += inputImage.SampleLevel(uv + offset, 0).rgb * weight[i];
            result += inputImage.SampleLevel(uv - offset, 0).rgb * weight[i];
        }
    } else { // vertical
        for (int i = 1; i < weight.getCount(); ++i) {
            float2 offset = float2(0.0, texel_offset.y * i);
            result += inputImage.SampleLevel(uv + offset, 0).rgb * weight[i];
            result += inputImage.SampleLevel(uv - offset, 0).rgb * weight[i];
        }
    }

    outputImage[texelCoord] = float4(result, 1.0);
}
