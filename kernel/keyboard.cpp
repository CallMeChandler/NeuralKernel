#include "keyboard.h"
#include "printk.h"
#include "io.h"
#include "shell.h"
#include "telemetry.h"
#include "auth.h"
#include "editor.h"
namespace keyboard {
static const char table[]={0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' '};
static bool ctrl=false,extended=false;
static void route(char c){if(editor::active())editor::handle_char(c);else if(!auth::authenticated())auth::handle_input(c);else shell::handle_input(c);}
extern "C" void keyboard_handler(){uint8_t s=inb(0x60);telemetry::keyboard_irqs++;if(s==0xE0){extended=true;outb(0x20,0x20);return;}bool release=s&0x80;s&=0x7F;if(s==0x1D){ctrl=!release;outb(0x20,0x20);return;}if(release){extended=false;outb(0x20,0x20);return;}if(editor::active()&&extended){if(s==0x4B)editor::handle_special(3);else if(s==0x4D)editor::handle_special(4);else if(s==0x48)editor::handle_special(5);else if(s==0x50)editor::handle_special(6);extended=false;outb(0x20,0x20);return;}extended=false;if(editor::active()&&ctrl){if(s==0x1F)editor::handle_special(1);else if(s==0x10)editor::handle_special(2);outb(0x20,0x20);return;}if(s<sizeof(table)){char c=table[s];if(c){telemetry::keys_pressed++;route(c);}}outb(0x20,0x20);}
void init(){printk::log(printk::INFO,"Keyboard initialized");}
}
