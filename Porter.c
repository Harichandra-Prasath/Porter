#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>


#define ERR_INIT_FILE_DESCRIPTOR 1
#define ERR_INIT_WATCH_DESCRIPTOR 2
#define ERR_ADDING_THE_PATH_TO_QUEUE 3
#define ERR_READING_THE_EVENT 4

// As we are only monitoring the donwloads folder, we can hard code it for inotify

int main() {
    int fd;
    int* wd;
    char* path;

    char buf[4096];

    const struct inotify_event *event;
    ssize_t len;       // ssize_t instead of int to get error if any

    struct passwd *pw = getpwuid(getuid());    //get the user id of the caller and get the user database

    const char *_path = pw->pw_dir;
    path = calloc(strlen(_path)+10,sizeof(char));
    strcpy(path,_path);
    strcat(path,"/Downloads/");

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

    wd[0] = inotify_add_watch(fd,path,IN_CREATE| IN_ACCESS|IN_CLOSE_WRITE);
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
                fprintf(stdout,"File created\n");
            }
        }

    }



}