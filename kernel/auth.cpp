#include "auth.h"
#include "vfs.h"
#include "terminal.h"
#include "shell.h"
namespace auth {
static char user[32]="root", pass[32]="neural", input[64]; static int idx=0; static bool password=false,ok=false;
static bool eq(const char*a,const char*b){int i=0;while(a[i]&&b[i]){if(a[i]!=b[i])return false;i++;}return a[i]==b[i];}
static void prompt(){terminal::set_color(terminal::LIGHT_CYAN);terminal::write(password?"Password: ":"NeuralKernel login: ");terminal::set_color(terminal::LIGHT_GREY);}
void initialize(){uint32_t s=0;auto*d=(const char*)vfs::open("users",&s);if(d){int a=0,b=0;while(a<(int)s&&d[a]!=':'&&a<31){user[a]=d[a];a++;}user[a]=0;if(a<(int)s&&d[a]==':')a++;while(a<(int)s&&d[a]!='\n'&&b<31)pass[b++]=d[a++];pass[b]=0;}ok=false;password=false;idx=0;}
bool login(){prompt();return ok;}
void handle_input(char c){if(c=='\n'){input[idx]=0;if(!password){if(eq(input,user)){password=true;idx=0;terminal::putchar('\n');prompt();}else{idx=0;terminal::write("\nUnknown user\n");prompt();}}else{if(eq(input,pass)){ok=true;password=false;idx=0;terminal::write("\nWelcome back, ");terminal::write(user);terminal::write(".\n");shell::initialize();}else{password=false;idx=0;terminal::write("\nLogin incorrect\n");prompt();}}return;}if(c=='\b'){if(idx>0){idx--;if(!password)terminal::putchar('\b');}return;}if(idx<63){input[idx++]=c;if(!password)terminal::putchar(c);}}
bool authenticated(){return ok;} const char* username(){return user;} void logout(){ok=false;password=false;idx=0;terminal::write("\nLogged out.\n");prompt();}
}
