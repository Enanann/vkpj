#pragma once

#include <string>

struct EffectParam {
    bool visible{true};
    std::string displayName;

    float defaultValue;
    float min;
    float max;
};

struct EffectParams {
    int passes{1};
    std::vector<float> paramsData;
};