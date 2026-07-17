#pragma once
#include <stdint.h>

namespace nn
{
    using q8_8 = int16_t;
    constexpr int32_t SCALE = 256;

    enum class Activation : uint8_t
    {
        NONE,
        RELU
    };

    struct Layer
    {
        const q8_8 *weights;
        const q8_8 *biases;
        uint16_t input_size;
        uint16_t output_size;
        Activation activation;
    };

    q8_8 clamp(int32_t value);
    q8_8 mul(q8_8 left, q8_8 right);
    void forward(const Layer &layer, const q8_8 *input, q8_8 *output);
    uint16_t argmax(const q8_8 *values, uint16_t count);
    uint32_t mse(const q8_8 *expected, const q8_8 *actual, uint16_t count);
}
