bool floatCmp(float value, float cmp) {
    float eps = 1e-6;
    return abs(value - cmp) < eps;
}

// https://en.wikipedia.org/wiki/HSL_and_HSV#General_approach
float3 RgbToHsl(float3 color) {
    float M = max3(color.r, color.g, color.b);
    float m = min3(color.r, color.g, color.b);
    float c = M - m;

    float eps = 1e-6;

    float _h = select(floatCmp(c, 0.0), 0,
                       select(floatCmp(M, color.r), fmod((color.g - color.b) / c, 6.0),
                              select(floatCmp(M, color.g), (color.b - color.r) / c + 2,
                                     (color.r - color.g) / c + 4)));
    float H = 1.0 * _h;
    if (H < 0) H += 6.0;

    float L = 0.5 * (M + m);

    float S = select(L > 1 - eps || L < eps, 0, c / (1 - abs(2 * L - 1)));

    return float3(H / 6.0, S, L);
}

// https://www.rapidtables.com/convert/color/hsl-to-rgb.html
float3 HslToRgb(float3 color) {
    float H = color.x * 6.0;
    float S = color.y;
    float L = color.z;

    float c = S * (1 - abs(2 * L - 1));
    float x = c * (1 - abs(fmod(H, 2.0) - 1));
    float m = L - c / 2;

    float3 _color = select(0 <= H && H < 1.0, float3(c, x, 0),
                           select(1.0 <= H && H < 2.0, float3(x, c, 0),
                                  select(2.0 <= H && H < 3.0, float3(0, c, x),
                                         select(3.0 <= H && H < 4.0, float3(0, x, c),
                                                select(4.0 <= H && H < 5.0, float3(x, 0, c),
                                                       select(5.0 <= H && H < 6.0, float3(c, 0, x), float3(c, 0, x)))))));
    return float3(_color + float3(m));
}
