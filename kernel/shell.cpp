#include "shell.h"
#include "terminal.h"
#include "telemetry.h"
#include "pmm.h"
#include "heap.h"
#include "task.h"
#include "scheduler.h"
#include "vfs.h"
#include "auth.h"
#include "editor.h"
#include "io.h"
namespace shell {
static char buf[256]; static int n=0;
static bool eq(const char*a,const char*b){int i=0;while(a[i]&&b[i]){if(a[i]!=b[i])return false;i++;}return a[i]==b[i];}
static bool starts(const char*a,const char*b){int i=0;while(b[i]){if(a[i]!=b[i])return false;i++;}return true;}
static void num(uint32_t x){if(!x){terminal::putchar('0');return;}char b[16];int i=0;while(x){b[i++]='0'+x%10;x/=10;}while(i--)terminal::putchar(b[i]);}
static void prompt(){terminal::set_color(terminal::LIGHT_CYAN);terminal::write("NeuralKernel> ");terminal::set_color(terminal::LIGHT_GREY);}
void initialize(){n=0;buf[0]=0;prompt();}
static void command(){telemetry::commands_executed++;terminal::putchar('\n');
if(eq(buf,"help"))terminal::write("help clear about telemetry pmm heap ps ls whoami uptime meminfo uname echo cat write rm edit exit reboot\n");
else if(eq(buf,"clear"))terminal::clear(); else if(eq(buf,"about"))terminal::write("NeuralKernel Experimental AI OS\n");
else if(eq(buf,"telemetry"))telemetry::print(); else if(eq(buf,"pmm"))pmm::print_stats(); else if(eq(buf,"heap"))heap::print_stats();
else if(eq(buf,"ls"))vfs::list(); else if(eq(buf,"whoami")){terminal::write(auth::username());terminal::putchar('\n');}
else if(eq(buf,"uptime")){uint32_t t=telemetry::timer_ticks;num(t/100);terminal::write(" seconds (ticks: ");num(t);terminal::write(")\n");}
else if(eq(buf,"meminfo")){terminal::write("Memory information\n  PMM used pages: ");num(pmm::used_pages_count());terminal::write("\n  PMM free pages: ");num(pmm::free_pages_count());terminal::write("\n  Heap allocated: ");num(heap::allocated_bytes());terminal::write(" bytes\n  Heap free: ");num(heap::free_bytes());terminal::write(" bytes\n");}
else if(eq(buf,"uname"))terminal::write("NeuralKernel v0.1 i386\n"); else if(starts(buf,"echo ")){terminal::write(buf+5);terminal::putchar('\n');}
else if(starts(buf,"cat ")){const char*f=buf+4;if(!vfs::valid_nkfs_name(f)){terminal::write("Warning: cat accepts .nkfs files only.\n");}else{uint32_t s;auto*d=(const char*)vfs::open(f,&s);if(!d)terminal::write("File not found.\n");else{for(uint32_t i=0;i<s;i++)terminal::putchar(d[i]);if(!s||d[s-1]!='\n')terminal::putchar('\n');}}}
else if(starts(buf,"write ")){char*f=buf+6;char*sp=f;while(*sp&&*sp!=' ')sp++;if(!*sp)terminal::write("Usage: write file.nkfs text\n");else{*sp++=0;if(*sp=='\"'){sp++;int l=0;while(sp[l]&&sp[l]!='\"')l++;sp[l]=0;}int l=0;while(sp[l])l++;terminal::write(vfs::write(f,sp,l)?"Written.\n":"Write failed. Use a .nkfs filename.\n");}}
else if(starts(buf,"rm "))terminal::write(vfs::remove(buf+3)?"Removed.\n":"Remove failed. Use a .nkfs filename.\n");
else if(starts(buf,"edit ")){if(editor::open(buf+5)){n=0;return;}}
else if(eq(buf,"exit")){n=0;auth::logout();return;} else if(eq(buf,"reboot")){terminal::write("Rebooting...\n");outb(0x64,0xFE);for(;;)asm volatile("hlt");}
else if(eq(buf,"ps")){terminal::write("ID STATE NAME\n");for(int i=0;i<task::get_task_count();i++){auto*t=task::get_task(i);if(!t||!t->active)continue;num(t->id);terminal::write(" ");terminal::write(task::state_string(t->state));terminal::write(" ");terminal::write(t->name);terminal::putchar('\n');}}
else if(n) {terminal::write("Unknown command: ");terminal::write(buf);terminal::putchar('\n');}
n=0;buf[0]=0;prompt();}
void handle_input(char c){if(c=='\n'){buf[n]=0;command();return;}if(c=='\b'){if(n){n--;terminal::putchar('\b');}return;}if(n<255){buf[n++]=c;terminal::putchar(c);}}
}
