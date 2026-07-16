#include "editor.h"
#include "terminal.h"
#include "vfs.h"
#include "shell.h"
namespace editor {
static bool opened=false; static char name[64]; static char text[1840]; static int len=0,pos=0;
static void copy(char*d,const char*s,int n){int i=0;for(;i<n-1&&s&&s[i];i++)d[i]=s[i];d[i]=0;}
static void render(){terminal::clear();terminal::set_color(terminal::LIGHT_CYAN);terminal::write("+- Editing: ");terminal::write(name);terminal::write(" ------------------------------------------------+\n");terminal::set_color(terminal::WHITE);for(int i=0;i<len;i++)terminal::putchar(text[i]);terminal::set_color(terminal::LIGHT_CYAN);terminal::set_cursor(23,0);terminal::write("Ctrl+S: Save  Ctrl+Q: Quit");int r=1,c=0;for(int i=0;i<pos;i++){if(text[i]=='\n'){r++;c=0;}else if(++c>=80){r++;c=0;}}terminal::set_cursor(r<23?r:22,c);terminal::set_color(terminal::WHITE);}
bool open(const char*f){if(!vfs::valid_nkfs_name(f)){terminal::write("Only .nkfs files can be edited.\n");return false;}copy(name,f,64);uint32_t s=0;auto*d=(const char*)vfs::open(f,&s);len=0;if(d)for(;len<(int)s&&len<1839;len++)text[len]=d[len];text[len]=0;pos=len;opened=true;render();return true;}
bool active(){return opened;}
void handle_char(char c){if(!opened)return;if(c=='\b'){if(pos>0){for(int i=pos-1;i<len;i++)text[i]=text[i+1];pos--;len--;}}else if(c=='\n'||(c>=32&&c<=126)){if(len<1839){for(int i=len;i>pos;i--)text[i]=text[i-1];text[pos++]=c;text[++len]=0;}}render();}
void handle_special(uint8_t k){if(!opened)return;if(k==1){vfs::write(name,text,len);render();}else if(k==2){opened=false;terminal::clear();terminal::write("Editor closed.\n");shell::initialize();return;}else if(k==3&&pos>0)pos--;else if(k==4&&pos<len)pos++;else if(k==5){int start=pos;while(start>0&&text[start-1]!='\n')start--;int column=pos-start;if(start>0){int previous_end=start-1;int previous_start=previous_end;while(previous_start>0&&text[previous_start-1]!='\n')previous_start--;int previous_length=previous_end-previous_start;pos=previous_start+(column<previous_length?column:previous_length);}}else if(k==6){int start=pos;while(start>0&&text[start-1]!='\n')start--;int column=pos-start;int end=pos;while(end<len&&text[end]!='\n')end++;if(end<len){int next_start=end+1;int next_end=next_start;while(next_end<len&&text[next_end]!='\n')next_end++;int next_length=next_end-next_start;pos=next_start+(column<next_length?column:next_length);}}if(opened)render();}
}
