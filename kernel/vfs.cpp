#include "vfs.h"
#include "printk.h"
#include "terminal.h"
#include "heap.h"
namespace vfs {
static const uint8_t* base=nullptr; static uint32_t total=0; static const Header* hdr=nullptr;
struct Overlay { bool used,deleted; char name[64]; char* data; uint32_t size; };
static Overlay overlays[64];
static bool eq(const char*a,const char*b){int i=0;while(a&&b&&a[i]&&b[i]){if(a[i]!=b[i])return false;i++;}return a&&b&&a[i]==b[i];}
static void copy(char*d,const char*s,int n){int i=0;for(;i<n-1&&s&&s[i];i++)d[i]=s[i];d[i]=0;}
bool valid_nkfs_name(const char*n){if(!n)return false;int l=0;while(n[l])l++;return l>5&&n[l-5]=='.'&&n[l-4]=='n'&&n[l-3]=='k'&&n[l-2]=='f'&&n[l-1]=='s';}
void init(void*s,uint32_t z){base=(const uint8_t*)s;total=z;hdr=nullptr;for(auto&o:overlays){o.used=false;o.deleted=false;o.data=nullptr;o.size=0;}if(!s||z<sizeof(Header)){printk::log(printk::ERROR,"VFS: invalid initrd");return;}auto*c=(const Header*)s;if(c->magic[0]!='N'||c->magic[1]!='K'||c->magic[2]!='F'||c->magic[3]!='S'||c->file_count>64){printk::log(printk::ERROR,"VFS: bad initrd");return;}hdr=c;printk::log(printk::INFO,"VFS: initrd mounted");}
static Overlay* find_overlay(const char*n){for(auto&o:overlays)if(o.used&&eq(o.name,n))return &o;return nullptr;}
const void* open(const char*n,uint32_t*out){auto*o=find_overlay(n);if(o){if(o->deleted)return nullptr;if(out)*out=o->size;return o->data;}if(!hdr)return nullptr;auto*e=(const FileEntry*)(base+sizeof(Header));for(uint32_t i=0;i<hdr->file_count;i++)if(eq(e[i].name,n)){if(e[i].offset>total||e[i].size>total-e[i].offset)return nullptr;if(out)*out=e[i].size;return base+e[i].offset;}return nullptr;}
bool exists(const char*n){uint32_t s;return open(n,&s)!=nullptr;}
bool write(const char*n,const char*d,uint32_t s){if(!valid_nkfs_name(n))return false;Overlay*o=find_overlay(n);if(!o){for(auto&x:overlays)if(!x.used){o=&x;o->used=true;copy(o->name,n,64);break;}}if(!o)return false;if(o->data)heap::kfree(o->data);o->data=(char*)heap::kmalloc(s+1);if(!o->data){o->used=false;return false;}for(uint32_t i=0;i<s;i++)o->data[i]=d[i];o->data[s]=0;o->size=s;o->deleted=false;return true;}
bool remove(const char*n){if(!valid_nkfs_name(n))return false;Overlay*o=find_overlay(n);if(!o){for(auto&x:overlays)if(!x.used){o=&x;o->used=true;copy(o->name,n,64);break;}}if(!o)return false;if(o->data){heap::kfree(o->data);o->data=nullptr;}o->size=0;o->deleted=true;return true;}
void list(){if(!hdr){terminal::write("VFS not mounted\n");return;}auto*e=(const FileEntry*)(base+sizeof(Header));terminal::write("NKFS files:\n");for(uint32_t i=0;i<hdr->file_count;i++){auto*o=find_overlay(e[i].name);if(o&&o->deleted)continue;terminal::write("  ");terminal::write(e[i].name);terminal::putchar('\n');}for(auto&o:overlays){if(!o.used||o.deleted)continue;bool in=false;for(uint32_t i=0;i<hdr->file_count;i++)if(eq(e[i].name,o.name))in=true;if(!in){terminal::write("  ");terminal::write(o.name);terminal::putchar('\n');}}}
}
