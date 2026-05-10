#pragma once

#include <string>

struct EffectParam {
    bool visible{true};
    std::string displayName;

    float defaultValue;
    float min;
    float max;
};
