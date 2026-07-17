#include "auth.h"
#include "vfs.h"
#include "terminal.h"
#include "shell.h"
#include "nn_scheduler.h"
#include "watchdog.h"

namespace auth
{
    static char user[32] = "root";
    static char pass[32] = "neural";
    static char input[64];
    static int index = 0;
    static bool password_mode = false;
    static bool logged_in = false;

    static bool equals(const char *left, const char *right)
    {
        int i = 0;
        while (left[i] && right[i])
        {
            if (left[i] != right[i]) return false;
            i++;
        }
        return left[i] == right[i];
    }

    static void prompt()
    {
        terminal::set_color(terminal::LIGHT_CYAN);
        terminal::write(password_mode ? "Password: " : "NeuralKernel login: ");
        terminal::set_color(terminal::LIGHT_GREY);
    }

    void initialize()
    {
        uint32_t size = 0;
        const char *data = (const char *)vfs::open("users", &size);
        if (data)
        {
            int source = 0;
            int target = 0;
            while (source < (int)size && data[source] != ':' && source < 31)
            {
                user[source] = data[source];
                source++;
            }
            user[source] = 0;
            if (source < (int)size && data[source] == ':') source++;
            while (source < (int)size && data[source] != '\n' && target < 31)
                pass[target++] = data[source++];
            pass[target] = 0;
        }

        logged_in = false;
        password_mode = false;
        index = 0;
    }

    bool login()
    {
        prompt();
        return logged_in;
    }

    void handle_input(char c)
    {
        if (c == '\n')
        {
            input[index] = 0;
            if (!password_mode)
            {
                if (equals(input, user))
                {
                    password_mode = true;
                    index = 0;
                    terminal::putchar('\n');
                    prompt();
                }
                else
                {
                    index = 0;
                    terminal::write("\nUnknown user\n");
                    prompt();
                }
            }
            else if (equals(input, pass))
            {
                logged_in = true;
                password_mode = false;
                index = 0;
                terminal::write("\nWelcome back, ");
                terminal::write(user);
                terminal::write(".\n");

                // Neural policy selection and watchdog actions start only
                // after the known-working boot, Ring 3, and login paths finish.
                nn_scheduler::set_enabled(true);
                watchdog::set_enabled(true);
                shell::initialize();
            }
            else
            {
                password_mode = false;
                index = 0;
                terminal::write("\nLogin incorrect\n");
                prompt();
            }
            return;
        }

        if (c == '\b')
        {
            if (index > 0)
            {
                index--;
                if (!password_mode) terminal::putchar('\b');
            }
            return;
        }

        if (index < 63)
        {
            input[index++] = c;
            if (!password_mode) terminal::putchar(c);
        }
    }

    bool authenticated() { return logged_in; }
    const char *username() { return user; }

    void logout()
    {
        logged_in = false;
        password_mode = false;
        index = 0;
        nn_scheduler::set_enabled(false);
        watchdog::set_enabled(false);
        terminal::write("\nLogged out.\n");
        prompt();
    }
}
