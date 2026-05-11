[shader("vertex")]
void vertMain() {}
[shader("fragment")]
void fragMain() {}

[vk_binding(0, 0)]
Sampler2D inputImage;

[vk_binding(1, 0)]
RWTexture2D<float4> outputImage;

struct pushConstants {
    int p;
    float sigma;
    float low_threshold;
    float high_threshold;
};

[vk_push_constant]
pushConstants pc;

static const float PI = 3.14159265359;
static const float l = 1.0 / sqrt(2.0 * PI * pc.sigma * pc.sigma);
/*
weights[x] = (1 / sqrt(2 * pi * sigma^2)) * exp(-(x^2) / (2 * sigma^2))
*/

float4 gaussian_blur(float2 uv, float2 texel_offset) {
    int radius = int(3 * pc.sigma);

    float3 result = inputImage.SampleLevel(uv, 0).rgb * l; // center
    float weightSum = l;

    if (pc.p == 0) { // horizontal
        for (int i = 1; i < radius; ++i) {
            float w = exp(-(i * i) / (2 * pc.sigma * pc.sigma));

            float2 offset = float2(texel_offset.x * i, 0.0);
            result += inputImage.SampleLevel(uv + offset, 0).rgb * w;
            result += inputImage.SampleLevel(uv - offset, 0).rgb * w;
            weightSum += 2 * w;
        }
    } else if (pc.p == 1) { // vertical
        for (int i = 1; i < radius; ++i) {
            float w = exp(-(i * i) / (2 * pc.sigma * pc.sigma));

            float2 offset = float2(0.0, texel_offset.y * i);
            result += inputImage.SampleLevel(uv + offset, 0).rgb * w;
            result += inputImage.SampleLevel(uv - offset, 0).rgb * w;
            weightSum += 2 * w;
        }
    }

    result /= weightSum;

    return float4(result, 1.0);
}

static const int2 offsets[9] = {
    int2(-1, -1), int2(0, -1), int2(1, -1),
    int2(-1, 0), int2(0, 0), int2(1, 0),
    int2(-1, 1), int2(0, 1), int2(1, 1)
};
void sobel(int2 texelCoord, uint2 size, out float Gx, out float Gy) {
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

    float colors[9];
    [unroll]
    for (int i = 0; i < 9; ++i) {
        int2 coord = clamp(texelCoord + offsets[i], int2(0, 0), size - 1);
        float3 rgb = inputImage.Load(int3(coord, 0)).rgb;
        colors[i]  = dot(rgb, float3(0.299, 0.587, 0.114));
    }

    float res1 = float(0.0);
    float res2 = float(0.0);
    [unroll]
    for (int i = 0; i < 9; ++i) {
        res1 += colors[i] * M_x[i];
        res2 += colors[i] * M_y[i];
    }

    Gx = res1;
    Gy = res2;
    // return float4(sqrt(res1 * res1 + res2 * res2), 1.0);
}

float edgeRounding(float deg) {
    if (deg < 0.0) {
        deg += 180.0;
    }

    if ((deg >= 0 && deg <= 22.5) || (deg >= 157.5 && deg <= 180.0)) {
        return 0.0;
    } else if (deg > 22.5 && deg <= 67.5) {
        return 45.0;
    } else if (deg > 67.5 && deg <= 112.5) {
        return 90.0;
    } else {
        return 135.0;
    }
}

// static const float low_threshold = 0.02;
// static const float high_threshold = 0.136;

float doubleThreshold(float i) {
    if (i < pc.low_threshold) {
        return 0.0;
    } else if (i > pc.high_threshold) {
        return 1.0;
    } else {
        return 0.5;
    }
}

float hysteresis(int2 texelCoord, uint2 size) {
    float center = inputImage.Load(int3(texelCoord, 0)).r;
    
    // not weak edge
    if (center < pc.low_threshold) return 0.0;
    if (center > pc.high_threshold) return 1.0;

    // weak edge
    [unroll]
    for (int i = 0; i < 9; ++i) {
        int2 coord = clamp(texelCoord + offsets[i], int2(0, 0), size - 1);
        float neighbor = inputImage.Load(int3(coord, 0)).r;
        if (neighbor > pc.high_threshold) {
            return 1.0; 
        }
    }
    
    // still weak edge for next hysteresis passes
    return 0.5; 
}

[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 ThreadId: SV_DispatchThreadID) {
    uint2 size;
    outputImage.GetDimensions(size.x, size.y);

    if (ThreadId.x >= size.x || ThreadId.y >= size.y) return;

    int2 texelCoord = int2(ThreadId.xy);

    float2 uv = (float2(texelCoord) + 0.5) / float2(size);

    float2 texel_offset = 1.0 / float2(size);

    float4 result;

    /*
    NW  N  NE
      \ | /
    W - + - E
      / | \
    SW  S  SE
    */
    if (pc.p == 2) {
        float Gx;
        float Gy;
        sobel(texelCoord, size, Gx, Gy);
        float Mag = sqrt(Gx * Gx + Gy * Gy);
        float deg = degrees(atan2(Gy, Gx)); // already in grayscale
        float rounded = edgeRounding(deg);

        // gradient ⊥ edge
        if (rounded == 0.0) {         // edge's N-S
            // E
            float Gx_e;
            float Gy_e;
            sobel(texelCoord + int2(1, 0), size, Gx_e, Gy_e);
            float Mag_e = sqrt(Gx_e * Gx_e + Gy_e * Gy_e);

            // W
            float Gx_w;
            float Gy_w;
            sobel(texelCoord - int2(1, 0), size, Gx_w, Gy_w);
            float Mag_w = sqrt(Gx_w * Gx_w + Gy_w * Gy_w);

            if (max3(Mag_e, Mag_w, Mag) != Mag) {
                Mag = 0;
            }

        } else if (rounded == 45.0) { // edge's NW-SE
            // NE
            float Gx_ne;
            float Gy_ne;
            sobel(texelCoord + int2(1, -1), size, Gx_ne, Gy_ne);
            float Mag_ne = sqrt(Gx_ne * Gx_ne + Gy_ne * Gy_ne);

            // SW
            float Gx_sw;
            float Gy_sw;
            sobel(texelCoord + int2(-1, 1), size, Gx_sw, Gy_sw);
            float Mag_sw = sqrt(Gx_sw * Gx_sw + Gy_sw * Gy_sw);

            if (max3(Mag_ne, Mag_sw, Mag) != Mag) {
                Mag = 0;
            }
        } else if (rounded == 90.0) { // edge's E - W
            // N
            float Gx_n;
            float Gy_n;
            sobel(texelCoord + int2(0, -1), size, Gx_n, Gy_n);
            float Mag_n = sqrt(Gx_n * Gx_n + Gy_n * Gy_n);

            // S
            float Gx_s;
            float Gy_s;
            sobel(texelCoord + int2(0, 1), size, Gx_s, Gy_s);
            float Mag_s = sqrt(Gx_s * Gx_s + Gy_s * Gy_s);
            
            if (max3(Mag_s, Mag_n, Mag) != Mag) {
                Mag = 0;
            }
        } else { // edge's NE-SW
            // NW
            float Gx_nw;
            float Gy_nw;
            sobel(texelCoord + int2(-1, -1), size, Gx_nw, Gy_nw);
            float Mag_nw = sqrt(Gx_nw * Gx_nw + Gy_nw * Gy_nw);

            // SE
            float Gx_se;
            float Gy_se;
            sobel(texelCoord + int2(1, 1), size, Gx_se, Gy_se);
            float Mag_se = sqrt(Gx_se * Gx_se + Gy_se * Gy_se);

            if (max3(Mag_se, Mag_nw, Mag) != Mag) {
                Mag = 0;
            }
        }

        result = float4(float3(Mag), 1.0);
    } else if (pc.p == 4) {
        result = float4(float3(doubleThreshold(inputImage.SampleLevel(uv, 0).r)), 1.0);
    } else if (pc.p == 5) {
        result = float4(float3(hysteresis(texelCoord, size)), 1.0);
    } else if (pc.p == 6) {
        result = float4(float3(hysteresis(texelCoord, size)), 1.0);
    } else if (pc.p == 7) {
        result = float4(float3(hysteresis(texelCoord, size)), 1.0);
    } else if (pc.p == 8) {
        result = float4(float3(hysteresis(texelCoord, size)), 1.0);
    } else if (pc.p == 9) {
        result = float4(float3(hysteresis(texelCoord, size)), 1.0);
    } else if (pc.p == 10) {
        result = float4(float3(hysteresis(texelCoord, size)), 1.0);
    } else {
        result = gaussian_blur(uv, texel_offset);
    }

    outputImage[texelCoord] = result;
}
