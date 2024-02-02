#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ERR_INIT_FILE_DESCRIPTOR 1
#define ERR_INIT_WATCH_DESCRIPTOR 2
#define ERR_ADDING_THE_PATH_TO_QUEUE 3
#define ERR_READING_THE_EVENT 4

// To sort it in different subDirectories
const char Types[3][4][10] = {{"png","jpg","jpeg","webp"},
                              {"mp4","mp3","mkv"},
                              {"pdf","txt"}};

const char* Directories[] = {"Images","Media","Docs","Misc"};
int Flags[4] = {0,0,0,0};


int handleDir(int _dir,char* _path){
    int err;
    if (Flags[_dir]==0){
        fprintf(stdout,"%s SubDirectory not found\nAttempting to create the subDirectory...\n",Directories[_dir]);
        Flags[_dir]=1;
        err = mkdir(strcat(strcat(_path,Directories[_dir]),"/"),00700);  // Giving read,write,execute access to current user
        if (err==-1){
            fprintf(stderr,"Error in creating the directory\n");
            return -1;
        }
        fprintf(stdout,"%s SubDirectory Created\n",Directories[_dir]);
    }else{
        strcat(strcat(_path,Directories[_dir]),"/");
        // if directory already exist, on making subdirectory path will be updated
        // to path/subDirectory
        fprintf(stdout,"%s SubDirectory Exists already\nSkipping Creation of SubDirectory..\n",Directories[_dir]);
    }
    return 0;
}

int Is_Dir(char* file){
    struct stat sb;
    int err =stat(file, &sb);
    if (err==-1){
        fprintf(stderr,"Error in getting the stat\n");  
    }
    return S_ISDIR(sb.st_mode);
}

int port(char* name,char* path) {
    printf("%s\n",name);
    char* _Tpath = calloc(1024,sizeof(char));  //copy of the  downloads path
    strcpy(_Tpath,path);
    strcat(_Tpath,name); // actual file path

    
    int ret = Is_Dir(_Tpath);
    if (ret==1){
        fprintf(stdout,"Ignoring the folder creation...\n");
        free(_Tpath);
        return 1;
    }

    char* _name = strdup(name);
    char* _path = calloc(1024,sizeof(char));      // for subdirectories
    
    strcpy(_path,path);
   

    char* token = strtok(_name,".");
    char* extension;
    int err;
    while (token!=NULL){
        extension = token;
        token = strtok(NULL,".");
    }

    

    int i;
    // got the extension.. Now start with the directory matching
    for (i=0;i<3;i++){

        // now extension matching
        for (int j=0;j<4;j++){
            // match found
            if (strcmp(Types[i][j],extension)==0){
                
                //handle Dir function to check, create the folders
                err = handleDir(i,_path);
                if (err==-1){
                    return -1;
                }

                strcat(_path,name); // new file path inside the subdirectoy
                err = rename(_Tpath,_path);
                if (err==-1){
                    fprintf(stderr,"Problem in porting...\n");
                    return -1;  //not using exit to make it try for next files
                }
                fprintf(stdout,"File Ported to %s\n",_path);
                free(_path);
                free(_Tpath);
                
                return 0;
            }
    }
    }
    // loop exited means no match found ... put it in misc
    handleDir(i,_path);
    
    strcat(_path,name);
    err = rename(_Tpath,_path);
    if (err==-1){
        fprintf(stderr,"Error in porting file...\n");
    }

    fprintf(stdout,"File Ported to %s\n",_path);
    free(_path);
    free(_Tpath);
    return 0;
    
}

// As we are only monitoring the donwloads folder, we can hard code it for inotify

int main() {

    int fd;
    int* wd;
    char buf[4096];
    char* path;
    const struct inotify_event *event;
    ssize_t len;       // ssize_t instead of int to get error if any

    struct passwd *pw = getpwuid(getuid());    //get the user id of the caller and get the user database

    path = pw->pw_dir;
    strcat(path,"/Downloads/");  //hardcode it from $HOME


    // Initialise the file descriptor
    fd = inotify_init();
    if (fd==-1){
        fprintf(stderr,"Error Initialising the file descriptor\n");
        exit(ERR_INIT_FILE_DESCRIPTOR);
    }

    //Allocate the memory for the watch descriptor
    wd = calloc(1,sizeof(int));
    if (wd==NULL){
        fprintf(stderr,"Error allocating memory to Watch descriptor\n");
        exit(ERR_INIT_WATCH_DESCRIPTOR);
    }

    wd[0] = inotify_add_watch(fd,path,IN_CREATE);
    if (wd[0]==-1){
        fprintf(stderr,"Error adding the path to the Eventqueue\n");
        exit(ERR_ADDING_THE_PATH_TO_QUEUE);
    }else{
        fprintf(stdout,"Success adding the path to the Eventqueue\n");
    }




    // loop to run forever...basically monitoring all the time
    fprintf(stdout,"Monitoring the directory\n");
    while(1) {
        
        len = read(fd,buf,sizeof(buf));
        if (len==-1){
            fprintf(stderr,"Error in reading the event\n");
            exit(ERR_READING_THE_EVENT);
        }

        for (char *bufptr = buf;bufptr<buf+len;bufptr+=sizeof(struct inotify_event)+event->len){
            event = (const struct inotify_event *) bufptr;
            if (event->mask & IN_CREATE){
                int code;
                char* name;

                name = calloc(strlen(event->name)+1,sizeof(char));
                strcpy(name,event->name);
                code = port(name,path);
                if (code==-1){
                    fprintf(stderr,"Error in porting to the subdirectory\n"); //not exiting to try for next events
                }
                free(name);

            }
        }

    }



}