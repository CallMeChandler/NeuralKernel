#include "nn.h"

namespace nn
{
    q8_8 clamp(int32_t value)
    {
        if (value > 32767) return 32767;
        if (value < -32768) return -32768;
        return (q8_8)value;
    }

    q8_8 mul(q8_8 left, q8_8 right)
    {
        return clamp(((int32_t)left * (int32_t)right) >> 8);
    }

    void forward(const Layer &layer, const q8_8 *input, q8_8 *output)
    {
        for (uint16_t row = 0; row < layer.output_size; row++)
        {
            int32_t sum = layer.biases ? layer.biases[row] : 0;
            const q8_8 *weights = layer.weights + row * layer.input_size;

            for (uint16_t column = 0; column < layer.input_size; column++)
                sum += ((int32_t)weights[column] * input[column]) >> 8;

            q8_8 value = clamp(sum);
            if (layer.activation == Activation::RELU && value < 0) value = 0;
            output[row] = value;
        }
    }

    uint16_t argmax(const q8_8 *values, uint16_t count)
    {
        uint16_t best = 0;
        for (uint16_t i = 1; i < count; i++)
            if (values[i] > values[best]) best = i;
        return best;
    }

    uint32_t mse(const q8_8 *expected, const q8_8 *actual, uint16_t count)
    {
        if (!count) return 0;

        uint32_t total = 0;
        for (uint16_t i = 0; i < count; i++)
        {
            int32_t difference = (int32_t)expected[i] - actual[i];
            uint32_t magnitude = difference < 0 ? (uint32_t)-difference : (uint32_t)difference;
            uint32_t squared = magnitude * magnitude;
            total = 0xFFFFFFFFU - total < squared ? 0xFFFFFFFFU : total + squared;
        }
        return total / count;
    }
}
