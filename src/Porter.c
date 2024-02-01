#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

#define ERR_INIT_FILE_DESCRIPTOR 1
#define ERR_INIT_WATCH_DESCRIPTOR 2
#define ERR_ADDING_THE_PATH_TO_QUEUE 3
#define ERR_READING_THE_EVENT 4

// To sort it in different subDirectories
const char Types[3][4][10] = {{"png","jpg","jpeg","webp"},
                              {"mp4","mp3","mkv"},
                              {"pdf","txt"}};

const char Directories[4][10] = {"Images/","Media/","Docs/","Misc/"};
int Flags[4] = {0,0,0,0};

int port(char* name,char* path) {

    char* _name = strdup(name);
    char* _path = strdup(path);      // for subdirectories
    char* _Tpath = strdup(path);


    char* token = strtok(_name,".");
    char* extension;
    int err;
    while (token!=NULL){
        extension = token;
        token = strtok(NULL,".");
    }
    // got the extension.. Now start with the directory matching
    for (int i=0;i<3;i++){

        // now extension matching
        for (int j=0;j<4;j++){
            // match found
            if (strcmp(Types[i][j],extension)==0){
                fprintf(stdout,"File Created of type-%s\n",extension);

                //Flags to check whether the subDirectory is already present
                if (Flags[i]==0){
                    fprintf(stdout,"%s SubDirectory not found\nAttempting to create the subDirectory...\n",Directories[i]);
                    Flags[i]=1;
                    err = mkdir(strcat(_path,Directories[i]),00700);  // Giving read,write,execute access to current user
                    if (err==-1){
                        fprintf(stderr,"Error in creating the directory");
                        return -1;
                    }
                    fprintf(stdout,"%s SubDirectory Created\n",Directories[i]);
                }else{
                    strcat(_path,Directories[i]);
                    // if directory already exist, on making subdirectory path will be updated
                    // to path/subDirectory


                    fprintf(stdout,"%s SubDirectory Exists already\nSkipping Creation of SubDirectory..\n",Directories[i]);
                }

                // some casting as rename only work with constant types
                strcat(_Tpath,name); // actual file path
                strcat(_path,name); // new file path inside the subdirectoy
                // printf("%s\n",path);
                // printf("%s\n",_path);
                err = rename(_Tpath,_path);
                if (err==-1){
                    fprintf(stderr,"Problem in porting...\n");
                    return -1;  //not using exit to make it try for next files
                }
                
                free(_Tpath);
                return 0;
            }
    }
    }
    
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

                fprintf(stdout,"File created\n");
                name = calloc(strlen(event->name)+1,sizeof(char));
                strcpy(name,event->name);
                code = port(name,path);
                if (code==-1){
                    fprintf(stderr,"Error in porting to the subdirectory"); //not exiting to try for next events
                }
                free(name);

            }
        }

    }



}