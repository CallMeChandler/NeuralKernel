#include "splash.h"
#include "terminal.h"

namespace splash
{
    void show()
    {
        terminal::set_color(terminal::LIGHT_CYAN);
        terminal::write(
            " _   _                      _ _  __                    _ \n"
            "| \\ | | ___ _   _ _ __ __ _| | |/ /___ _ __ _ __   ___| |\n"
            "|  \\| |/ _ \\ | | | '__/ _` | | ' // _ \\ '__| '_ \\ / _ \\ |\n"
            "| |\\  |  __/ |_| | | | (_| | | . \\  __/ |  | | | |  __/ |\n"
            "|_| \\_|\\___|\\__,_|_|  \\__,_|_|_|\\_\\___|_|  |_| |_|\\___|_|\n");
        terminal::set_color(terminal::WHITE);
        terminal::write("NeuralKernel v0.1 - Small kernel. Real boundaries.\n\n");
        terminal::set_color(terminal::LIGHT_GREY);
    }
}
