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
    float sigma;
};

[vk_push_constant]
pushConstants pc;

static const float PI = 3.14159265359;
static const float l = 1.0 / sqrt(2.0 * PI * pc.sigma * pc.sigma);
/*
weights[x] = (1 / sqrt(2 * pi * sigma^2)) * exp(-(x^2) / (2 * sigma^2))
*/

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

    int radius = int(3 * pc.sigma);

    float3 result = inputImage.SampleLevel(uv, 0).rgb * l; // center
    float weightSum = l;

    if (pc.p % 2 == 1) { // horizontal
        for (int i = 1; i < radius; ++i) {
            float w = exp(-(i * i) / (2 * pc.sigma * pc.sigma));
            float2 offset = float2(texel_offset.x * i, 0.0);
            result += inputImage.SampleLevel(uv + offset, 0).rgb * w;
            result += inputImage.SampleLevel(uv - offset, 0).rgb * w;
            weightSum += 2 * w;
        }
    } else { // vertical
        for (int i = 1; i < radius; ++i) {
            float w = exp(-(i * i) / (2 * pc.sigma * pc.sigma));
            float2 offset = float2(0.0, texel_offset.y * i);
            result += inputImage.SampleLevel(uv + offset, 0).rgb * w;
            result += inputImage.SampleLevel(uv - offset, 0).rgb * w;
            weightSum += 2 * w;
        }
    }

    result /= weightSum;

    outputImage[texelCoord] = float4(result, 1.0);
}
