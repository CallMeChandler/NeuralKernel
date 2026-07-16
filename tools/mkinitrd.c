#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct { char magic[4]; uint32_t file_count; } Header;
typedef struct { char name[64]; uint32_t offset; uint32_t size; } FileEntry;
typedef struct { char path[512]; char name[64]; uint8_t* data; uint32_t size; } Input;

static int compare(const void* a,const void* b){return strcmp(((const Input*)a)->name,((const Input*)b)->name);}
int main(int argc,char**argv){
 if(argc!=3){fprintf(stderr,"Usage: mkinitrd output.nkfs input_directory\n");return 1;}
 DIR*d=opendir(argv[2]);if(!d){perror("opendir");return 1;} Input files[64];int count=0;struct dirent*de;
 while((de=readdir(d))){if(de->d_name[0]=='.')continue;if(count>=64){fprintf(stderr,"Too many files (max 64)\n");return 1;}snprintf(files[count].path,sizeof(files[count].path),"%s/%s",argv[2],de->d_name);struct stat st;if(stat(files[count].path,&st)||!S_ISREG(st.st_mode))continue;strncpy(files[count].name,de->d_name,63);files[count].name[63]=0;count++;}closedir(d);qsort(files,count,sizeof(Input),compare);
 uint32_t cursor=sizeof(Header)+count*sizeof(FileEntry);for(int i=0;i<count;i++){FILE*f=fopen(files[i].path,"rb");if(!f){perror(files[i].path);return 1;}fseek(f,0,SEEK_END);long z=ftell(f);rewind(f);if(z<0){fclose(f);return 1;}files[i].size=(uint32_t)z;files[i].data=(uint8_t*)malloc(files[i].size?files[i].size:1);if(files[i].size&&fread(files[i].data,1,files[i].size,f)!=files[i].size){fclose(f);return 1;}fclose(f);cursor+=files[i].size;}
 FILE*out=fopen(argv[1],"wb");if(!out){perror(argv[1]);return 1;}Header h={{'N','K','F','S'},(uint32_t)count};fwrite(&h,sizeof(h),1,out);uint32_t off=sizeof(Header)+count*sizeof(FileEntry);for(int i=0;i<count;i++){FileEntry e;memset(&e,0,sizeof(e));strncpy(e.name,files[i].name,63);e.offset=off;e.size=files[i].size;fwrite(&e,sizeof(e),1,out);off+=e.size;}for(int i=0;i<count;i++){if(files[i].size)fwrite(files[i].data,1,files[i].size,out);free(files[i].data);}fclose(out);printf("NKFS: packed %d files from %s\n",count,argv[2]);return 0;
}
