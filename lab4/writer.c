# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>

int main(int argc, char *argv[]){

    char* name;

    name = argv[1];
    
    int fd = open("/dev/mydev",O_RDWR);
    if(fd<0){
        printf("%s","Can not open the file");
    }
    
    
    while (*name != '\0') {
        write(fd, name++, 1); // Write a single character
        sleep(1); // Sleep for one second
    }

    close(fd);
    return 0;

}
