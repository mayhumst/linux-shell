#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h> 
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h> 


#include "builtin.h" 
//mine

/*

    77 points

*/

int main() {
    while(true) {

    //
    // GET MY WORKING DIRECTORY 
    //

    char myPath[PATH_MAX];
    // reference to cwd found in stack overflow https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    if(strcmp(cwd, "/") == 0) {
        //root path
        strcpy(myPath, "/");
    }
    else {
        char * token = strtok(cwd, "/");
        char curr[PATH_MAX];
        while(token != NULL) {
            strcpy(curr, token);
            token = strtok(NULL, "/");
        }
        strcpy(myPath, curr);
    }

    //
    // PRINT PROMPT
    //

    printf("[nyush %s]$ ", myPath);
    fflush(stdout);
    fflush(stderr);

    //
    // PARSE COMMAND LINE
    //
    // used sample function provided here for getline() https://c-for-dummies.com/blog/?p=1112
    char *buffer;
    size_t bufsize = 1010;
    size_t characters;

    buffer = (char *)malloc(bufsize * sizeof(char));
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

    characters = getline(&buffer,&bufsize,stdin);
    characters++;
    // break loop on ctrl D
    if( feof(stdin) ) { 
        printf("\n");
        break ;
    }

    //create argv and argc
    //argv is list of pointers to each arg, input from command line deliminated by spaces
    //argc is number of arguments 
    int redirects[10][2];
    int pipes[10];
    pipes[0] = -1;
    // FOR REDIRECTS: 
        // pipe | 0
        // input < 1
        // out1 > 2
        // out2 >> 3
    int redir_index = 0;
    redirects[0][0] = -1;
    int numpipes = 0;
    int numin = 0;
    int numoutr = 0;
    int numouta = 0;
    
    char ** argv = malloc(500*sizeof(char*));
    char * token = strtok(buffer, " ");
    int argc = 0;
    while (token != NULL) {
        //if arg not " " or ""
        //may be unnecessary
        if(*token == ' ') {
            continue;
        }
        else if(*token == '|') {
            redirects[redir_index][0] = argc;
            redirects[redir_index][1] = 0; //pipe at argv[argc]
            pipes[numpipes] = argc;
            redir_index++;
            numpipes++;
        }
        else if(*token == '<') {
            redirects[redir_index][0] = argc;
            redirects[redir_index][1] = 1; //< at argv[argc]
            redir_index++;
            numin++;
        }
        else if(strcmp(token, ">>") == 0) {
            redirects[redir_index][0] = argc;
            redirects[redir_index][1] = 3; //>> at argv[argc]
            redir_index++;
            numouta++;
        }
        else if(*token == '>') {
            redirects[redir_index][0] = argc;
            redirects[redir_index][1] = 2; //> at argv[argc]
            redir_index++;
            numoutr++;
        }
        
        //add token string to argv
        argv[argc] = token;

        //increment token and argc
        token = strtok(NULL, " ");
        argc++; 
    }
    argv[argc] = NULL;
    //strip newline from last arg
    char* ptr = argv[argc-1];
    int k = 0;
    while(ptr[k] != '\0') {
        if(ptr[k] == '\n') {
            ptr[k] = '\0';
        }
        k++;
    }
     
    //
    // ERROR HANDLING
    //

    if(pipes[0] == -1) {

    }
    
    // first arg is special signal
    if(redirects[0][0] == 0) {
        fprintf(stderr, "Error: invalid command\n");
        continue;
    }
    if(numpipes < 1 && redir_index >0) {
        int sig = 0;

        // multiple input redirects 
        if(numin > 1) {
            fprintf(stderr, "Error: invalid command\n");
            sig++;
        }
        // multiple output redirects
        else if(numouta + numoutr > 1) {
            fprintf(stderr, "Error: invalid command\n");
            sig++;
        }
        // no file name/arg after
        else {
            int x = 0;
            while(x<redir_index) {
                int index = redirects[x][0];
                if(argv[index+1] == NULL) {
                    fprintf(stderr, "Error: invalid command\n");
                    sig++;
                    break;
                }
                x++;
            }
        }
        if(sig>0) {
            //if error thrown due to invalid command, continue loop
            continue;
        }   
    }
      

    //
    // BUILT IN COMMANDS //cd, jobs, fg, and exit.
    //

    if(strcmp(argv[0], "exit")==0) {
        if(argc>1) {
            fprintf(stderr, "Error: invalid command\n");
        }
        else {
            exit(0);
            break;
        }
    }
    else if(strcmp(argv[0], "cd") == 0) {
        //if has valid arg next && has only one param
        if(argv[1] != NULL && argc ==2) {
            int test = cd(argv[1]);
            test++;    
        }
        else {
            fprintf(stderr, "Error: invalid command\n");
        }
    }
    else if(strcmp(argv[0], "jobs") == 0) {
        continue;
    }
    else if(strcmp(argv[0], "fg") == 0) {
        continue;
    }
     
    else { 
        //
        // EXECUTE PROGRAMS
        //  

        pid_t p = fork();
        //int status = 0;
        

        if (p < 0) {
            // fork failed (this shouldn't happen)
            printf("Fork failed");
            exit(0);
        } 
        else if (p == 0) { // child (new process)
            if(numpipes == 0) {
                simpleprog(argv, argc, redir_index, redirects);
            }
            else {
                if(numpipes == 1 && strcmp(argv[0], "./producer") == 0) {
                    pipe_special(argv, argc, redir_index, redirects, pipes, numpipes);
                }
                else {
                    pipe_total(argv, argc, redir_index, redirects, pipes, numpipes);
                }
            }
        } 
        else {
            // parent
            wait(NULL);
            //waitpid(-1, &status, WNOHANG);
        }
    }
    
}
return 0;
}
