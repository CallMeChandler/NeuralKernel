#include "nn_pmm.h"
#include "nn.h"
#include "nn_weights.h"

namespace nn_pmm
{
    static nn::q8_8 normalize(uint32_t value, uint32_t maximum)
    {
        if (!maximum) return 0;
        if (value >= maximum) return 256;
        return (nn::q8_8)((value * 256U) / maximum);
    }

    int32_t score(const Features &features)
    {
        nn::q8_8 input[8] = {
            normalize(features.recency, 1000),
            normalize(features.frequency, 100),
            normalize(features.allocation_age, 5000),
            normalize(features.task_affinity, 15),
            normalize(features.access_stride, 64),
            features.dirty ? (nn::q8_8)256 : (nn::q8_8)0,
            normalize(features.size_class, 3),
            normalize(features.recent_fault_rate, 100)
        };

        nn::q8_8 hidden[16];
        nn::q8_8 output[1];
        const nn::Layer hidden_layer = {
            nn_weights::pmm_hidden_weights,
            nn_weights::pmm_hidden_bias,
            8, 16, nn::Activation::RELU
        };
        const nn::Layer output_layer = {
            nn_weights::pmm_output_weights,
            nn_weights::pmm_output_bias,
            16, 1, nn::Activation::NONE
        };

        nn::forward(hidden_layer, input, hidden);
        nn::forward(output_layer, hidden, output);
        return output[0];
    }
}
