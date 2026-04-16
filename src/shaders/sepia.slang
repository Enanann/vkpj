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
// void compMain(uint3 threadId: SV_DispatchThreadID) {
//     uint2 size;
//     inputImage.GetDimensions(size.x, size.y);

//     if (threadId.x >= size.x || threadId.y >= size.y) return;

//     int2 texelCoord = int2(threadId.xy);
//     float4 color = inputImage.Load(int3(texelCoord, 0));

//     float nr = 0.393 * color.r + 0.769 * color.g + 0.189 * color.b;
//     float ng = 0.349 * color.r + 0.686 * color.g + 0.168 * color.b;
//     float nb = 0.272 * color.r + 0.534 * color.g + 0.131 * color.b;

//     outputImage[texelCoord] = float4(nr, ng, nb, color.a);
// }

/*
V2: Preserving the luminace
    https://leware.net/photo/blogSepia.html
    with different luminance (0.2126R + 0.7152G + 0.0722B)
*/
void compMain(uint3 threadId: SV_DispatchThreadID) {
    uint2 size;
    inputImage.GetDimensions(size.x, size.y);

    if (threadId.x >= size.x || threadId.y >= size.y) return;

    int2 texelCoord = int2(threadId.xy);
    float4 color = inputImage.Load(int3(texelCoord, 0));

    float luminance = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;

    float rg = 19.0 / 255.0;
    float gb = 27.0 / 255.0;
    
    float ar = rg * 0.7 + gb * 0.11;
    float ag = ar - rg;
    float ab = ar - rg - gb;

    float nr = luminance + ar;
    float ng = luminance + ag;
    float nb = luminance + ab;

    outputImage[texelCoord] = float4(nr, ng, nb, color.a);
}
