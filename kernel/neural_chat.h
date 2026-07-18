#pragma once

namespace neural_chat
{
    void enter();
    void leave();
    bool active();
    void handle_line(const char *line);
}
