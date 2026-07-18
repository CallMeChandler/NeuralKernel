#include "nlp.h"
#include "nn.h"
#include "nlp_weights.h"

namespace nlp
{
    static char lower_char(char value)
    {
        if (value >= 'A' && value <= 'Z') return value - 'A' + 'a';
        return value;
    }

    static bool equals(const char *left, const char *right)
    {
        uint16_t i = 0;
        while (left && right && left[i] && right[i])
        {
            if (lower_char(left[i]) != lower_char(right[i])) return false;
            i++;
        }
        return left && right && left[i] == right[i];
    }

    static bool contains(const char *text, const char *pattern)
    {
        if (!text || !pattern || !pattern[0]) return false;
        for (uint16_t start = 0; text[start]; start++)
        {
            uint16_t offset = 0;
            while (pattern[offset] && text[start + offset] &&
                   lower_char(text[start + offset]) == pattern[offset])
                offset++;
            if (!pattern[offset]) return true;
        }
        return false;
    }

    Result classify(const char *text)
    {
        nn::q8_8 input[nlp_weights::FEATURE_COUNT];
        nn::q8_8 hidden[nlp_weights::HIDDEN_COUNT];
        nn::q8_8 output[nlp_weights::INTENT_COUNT];

        uint16_t matched_features = 0;
        for (uint16_t i = 0; i < nlp_weights::FEATURE_COUNT; i++)
        {
            input[i] = contains(text, nlp_weights::vocabulary[i]) ? 256 : 0;
            if (input[i]) matched_features++;
        }

        const nn::Layer hidden_layer = {
            nlp_weights::hidden_weights,
            nlp_weights::hidden_biases,
            nlp_weights::FEATURE_COUNT,
            nlp_weights::HIDDEN_COUNT,
            nn::Activation::RELU
        };
        const nn::Layer output_layer = {
            nlp_weights::output_weights,
            nlp_weights::output_biases,
            nlp_weights::HIDDEN_COUNT,
            nlp_weights::INTENT_COUNT,
            nn::Activation::NONE
        };

        nn::forward(hidden_layer, input, hidden);
        nn::forward(output_layer, hidden, output);

        uint16_t best = nn::argmax(output, nlp_weights::INTENT_COUNT);
        int16_t second = -32768;
        for (uint16_t i = 0; i < nlp_weights::INTENT_COUNT; i++)
            if (i != best && output[i] > second) second = output[i];

        Result result = {
            (Intent)best,
            output[best],
            (int16_t)(output[best] - second)
        };

        // Empty or weakly matched text should never trigger a real command.
        bool accepted_short_phrase =
            equals(text, "hello") || equals(text, "hi") || equals(text, "hey") ||
            equals(text, "help") || equals(text, "what can you do") ||
            equals(text, "reboot") || equals(text, "restart") ||
            equals(text, "uptime") || equals(text, "who am i") || equals(text, "ls") ||
            equals(text, "ps") || equals(text, "exit neural");

        if (!matched_features || result.margin < 48 ||
            (matched_features < 2 && !accepted_short_phrase))
            result.intent = Intent::UNKNOWN;

        return result;
    }

    const char *intent_name(Intent intent)
    {
        switch (intent)
        {
            case Intent::GREET: return "GREET";
            case Intent::LIST_FILES: return "LIST_FILES";
            case Intent::OPEN_FILE: return "OPEN_FILE";
            case Intent::MEMORY_INFO: return "MEMORY_INFO";
            case Intent::TASK_INFO: return "TASK_INFO";
            case Intent::UPTIME_INFO: return "UPTIME_INFO";
            case Intent::WHOAMI: return "WHOAMI";
            case Intent::HELP: return "HELP";
            case Intent::REBOOT: return "REBOOT";
            case Intent::EXIT_MODE: return "EXIT_MODE";
            default: return "UNKNOWN";
        }
    }
}
