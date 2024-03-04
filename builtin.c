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


int cd(char* arg) {
/*
    //check validity of the arg
    //split string by "/"
*/
    //open dir
    struct dirent *de;  // Pointer for directory entry
    DIR *dr = opendir("."); 
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
    de = readdir(dr); 
    char* dename = de->d_name;

    //find arg in the directory
    //first part of path arg
    char * token = strtok(arg, "/");
    int i = 0;
    //for each token in the path arg, delim by /, no extra spaces
    while(token != NULL && *token != '\n') {
        //for each file/folder in directory
        while (de != NULL) {
            strcpy(dename, de->d_name);
            //if token and directory match, success - change directory and if necessary proceed tokens 
            if (strcmp(dename, token) == 0 || strcmp(strcat(dename, "\n"), token) == 0) {
                //change dir
                chdir(de->d_name);
                    
                //update de, dir
                closedir(dr);   
                dr = opendir("."); 
                if (dr == NULL)  // opendir returns NULL if couldn't open directory 
                { 
                    printf("Could not open current directory" ); 
                    return 0; 
                } 
                de = readdir(dr); 
                dename = de->d_name;
                i = 1;
                break;
            }
            //if they dont match, check next directory file
            de = readdir(dr);
        }
        //if a match was found, update the token to the next piece
        if(i == 1) {
            i = 0;
            token = strtok(NULL, "/");            
        }
        //no match was found, invalid cd call
        else {
            fprintf(stderr, "Error: invalid directory\n");
            closedir(dr);  
            break;
        }            
    }
        
    return 0; 
}


int simpleprog(char ** argv, int argc, int redir_index, int redirects[10][2]) {
    int type = 0; 
    char firstcommand[100]; 
    char** firstargs = malloc((100) * sizeof(char*));
    firstargs[0] = firstcommand;
    if (argc == 1) {
        //
    }

    //THREE CASES ? (FOUR?)
    // absolute path always starts w slash, use as is
    if(argv[0][0] == '/') { 
        strcpy(firstcommand, argv[0]);
    }
    // starts with ./
    else if(argv[0][0] == '.' && argv[0][1] == '/') {
        strcpy(firstcommand, argv[0]);
    }
    else { //ITERATE AND CHECK FOR SLASHES
        int a = 0;
        int contains = 0;
        while (argv[0][a] != '\0') {
            if(argv[0][a] == '/') {
                contains = 1;
                break;
            }
            a++;
        }
        //contains slash anywhere, relative to our directory
        if (contains ==1) {
            char newarg[100] = "./";
            strcat(newarg, argv[0]);
            strcpy(firstcommand, newarg);
        }
        //no slashes, only the name of the program specified.. append 
        else {
            if(strcmp(argv[0], "time") == 0) {
                strcpy(firstcommand, argv[0]);
                type = 1;
            }
            else {
                char newarg[100] = "/usr/bin/";
                strcat(newarg, argv[0]);
                strcpy(firstcommand, newarg);
            }
            
        }
    }
    
    
    //copy only args, no redirects or pipes
    int c = 1; 
    if(redir_index == 0) {
        //no redirects
        while(argv[c] != NULL) {
            //for each arg
            firstargs[c] = malloc(100);
            strcpy(firstargs[c], argv[c]);
            c++;
        }
        firstargs[c]= NULL;
    }
    else {
        //redirects exist
        while( c < *redirects[0] ) {
            firstargs[c] = malloc(100);
            strcpy(firstargs[c], argv[c]);
            c++;
        }
        firstargs[c]= NULL;
    }

    // freopen reference from https://www.tutorialspoint.com/c_standard_library/c_function_freopen.htm
    FILE *fp1 = NULL;
    FILE *fp2 = NULL;
    FILE *fp3 = NULL;
    int b = 1; // num of special chars
    while(b <= redir_index) {
        int argnum = redirects[b-1][0];
        //redirects[0][1] = character
        if(redirects[b-1][1] == 0) {
            // pipe
        }
        else if(redirects[b-1][1] == 1) {
            // input
            if(open(argv[argnum+1], O_RDONLY) < 0) {
                fprintf(stderr, "Error: invalid file\n");
                exit(-1);
            }
            else {
                fp1 = freopen(argv[argnum+1], "r", stdin);
            }
        }
        else if(redirects[b-1][1] == 2) {
            // output overwrite
            fp2 = freopen(argv[argnum+1], "w+", stdout);

        }
        else {
            // output append
            fp3 = freopen(argv[argnum+1], "a+", stdout);
        }
        b++;
    }

    if(type == 1) {
        execvp( firstcommand, firstargs );
        exit(errno);
    }
    else {
        execv( firstcommand, firstargs );
    }
    
    
    if(fp1 != NULL) {
        fclose(fp1);
    }
    if(fp2 != NULL) {
        fclose(fp2);
    }
    if(fp3 != NULL) {
        fclose(fp3);
    }

    fprintf(stderr, "Error: invalid program\n");
    exit(errno);

}


char** pipe_piece(char ** argv, int argc, int redir_index, int redirects[10][2], int pipes[10], int numpipes, int begin, int end, int location) {
    
    /*
        This function parses argv between 2 given indexes and returns a new argument subarray to be executed. 
    */
    //LOCATION: 1 = left, 2 = middle, 3 = right


    // UNUSED
    if(argc || redir_index || redirects[0] || pipes[0] || numpipes || location) {
        //
    }
    
    char firstcommand[100]; 
    char** firstargs = malloc((100) * sizeof(char*));
    firstargs[0] = firstcommand;

    strcpy(firstcommand, argv[ begin ]);

    int c = begin + 1;
    int i = 1; 
    while( c < end ) {
        firstargs[i] = malloc(100);                        
        strcpy(firstargs[i], argv[c]);
        c++;
        i++; 
    }
    firstargs[i] = NULL;
    return firstargs; 
}


void pipe_total(char ** argv, int argc, int redir_index, int redirects[10][2], int pipes[10], int numpipes) {

    int index = 0;
    int err = 0; 

    //make new pipes
    int mypipe[2*numpipes];

    for (int in = 0; in < numpipes; in++) {
        pipe(mypipe + (in*2));
    }
    
    int my_in = dup(0);
    int my_out = dup(1);

    int fp_in;
    int fp_out;
    

    while(index < numpipes) { // number of loop iterations equals number of pipes
        pid_t newPID = fork(); 
        //child
        if(newPID == 0) {
            
            if(index == 0) { 
                //leftmost command
                //fprintf(stderr, "happens first\n");
                char ** myargs; 
                //determine input from
                if(redirects[0][1] != 0) { //first redirect is not the pipe
                    if(redirects[0][1] == 1) {//input redirect
                        if(open(argv[ redirects[0][0] + 1 ], O_RDONLY) < 0) {
                            fprintf(stderr, "Error: invalid file\n");
                            err++;
                            exit(1); 
                        }
                        else {
                            fp_in = open(argv[ redirects[0][0] + 1 ], O_RDONLY);
                            dup2(fp_in, 0);
                            close(fp_in);
                            myargs = pipe_piece(argv, argc, redir_index, redirects, pipes, numpipes, 0, redirects[0][0], 1 ); 
                        }
                    }
                    else { 
                        //bad redirect args
                        fprintf(stderr, "Error: invalid command\n");
                        err++; 
                        exit(1); 
                    }
                }
                else {
                    // 0 to first pipe
                    myargs = pipe_piece(argv, argc, redir_index, redirects, pipes, numpipes, 0, pipes[0], 1 ); 
                }

                // do first command and write to pipe1
                dup2(mypipe[1], 1);
                close(mypipe[1]); 
                close(mypipe[0]);
                
                execvp(myargs[0], myargs); 
                exit(0); 
            }
            else {
                // FIRST: figure out arg array bounds
                // from previous pipe (+1) to this pipe                 
                char ** myargs = pipe_piece(argv, argc, redir_index, redirects, pipes, numpipes, pipes[index -1] + 1, pipes[index], 2 ); 
                
                
                //fprintf(stderr, "happens third\n");
               
                // take from pipe2, do first command, write to pipe1
                // take from pipe2 // write to pipe1

                //write to next pipe
                dup2(mypipe[(index*2) + 1], 1);
                close(mypipe[(index*2) + 1]);
                close(mypipe[(index*2)]);

                
                execvp(myargs[0], myargs); 
                //fprintf(stderr, "Error in middle\n");
                exit(0); 
            }
            exit(0); //just in case?
        }
        //parent 
        else {

            if (err > 0) {
                exit(1); 
            }

            if(index == numpipes-1) {
                //last iteration
                close(mypipe[(index*2) + 1]);
                wait(NULL);

                pid_t lastPID = fork(); 

                if(lastPID == 0) {
                    //fprintf(stderr, "happens fourth\n");
                    char** myargs;

                    if(redirects[redir_index-1][1] != 0) { //final special char NOT a pipe
                        // OUTPUT REDIRECT TO FILE
                        if(redirects[redir_index-1][1] == 2) {
                            // overwrite
                            fp_out = open(argv[ redirects[redir_index-1][0] + 1 ], O_WRONLY | O_CREAT);
                            dup2(fp_out, 1);
                            close(fp_out);
                            myargs = pipe_piece(argv, argc, redir_index, redirects, pipes, numpipes, pipes[index] + 1, argc -2, 3 ); 
                        }
                        else if(redirects[redir_index-1][1] == 3) {
                            //append
                            fp_out = open(argv[ redirects[redir_index-1][0] + 1 ], O_WRONLY | O_APPEND);
                            dup2(fp_out, 1);
                            close(fp_out);
                            myargs = pipe_piece(argv, argc, redir_index, redirects, pipes, numpipes, pipes[index] + 1, argc -2, 3 ); 
                        }
                        else {
                            //incorrect final symbol
                            fprintf(stderr, "Error: invalid command\n");
                            err++; 
                            exit(1); 
                        }
                    }
                    else{
                        //REGULAR OUTPUT DIRECT TO STDOUT
                        dup2(my_out, 1);
                        close(my_out);
                        //from last pipe to end 
                        myargs = pipe_piece(argv, argc, redir_index, redirects, pipes, numpipes, pipes[index] + 1, argc, 3 ); 
                    }

                    //read from last pipe
                    dup2(mypipe[(index*2)], 0);
                    close(mypipe[(index*2)]);
                    close(mypipe[(index*2) + 1]);

                    if(execvp(myargs[0], myargs) < 0){
                        fprintf(stderr, "Error: invalid command\n");
                        exit(EXIT_FAILURE);
                    }

                    execvp(myargs[0], myargs); 
                    exit(0); 
                               
                }
                else {
                    
                    //fprintf(stderr, "happens fifth\n");
                    close(mypipe[(index*2)]);
                    close(mypipe[(index*2) + 1]);
                    close(my_out);
                    
                    wait(NULL);

                    //fprintf(stderr, "happens sixth\n");
                    
                    index++; 
                    
                    dup2(my_in, 0);
                    close(my_in);

                }
            }
            else {
                close(mypipe[((index)*2) + 1]);
                wait(NULL);

                // read from previous pipe
                dup2(mypipe[((index)*2)], 0);
                close(mypipe[((index)*2)]);

                index++; 
                //fprintf(stderr, "happens second\n");
            }
                       
        }        
    }  

    exit(0);       
}

void pipe_special(char ** argv, int argc, int redir_index, int redirects[10][2], int pipes[10], int numpipes) {
    // UNUSED
    if(redir_index < 1) {
        if(redirects[0][0]) {
            if(numpipes) {
                
            }
        }
    }

    //make new pipe
    int mypipe[2];
    pipe(mypipe);
    pid_t newPID = fork(); 

    //child
    if(newPID == 0) {
        
        char ** myargs = pipe_piece(argv, argc, redir_index, redirects, pipes, numpipes, 0, pipes[0], 1 ); 
        //LOCATION: 1 = left, 2 = middle, 3 = right

        dup2(mypipe[1], STDOUT_FILENO);
        close(mypipe[1]);
        close(mypipe[0]); 
        execvp(myargs[0], myargs); 
        fprintf(stderr, "Error in first\n");
    }

    //parent 
    else {
        
       char ** myargs = pipe_piece(argv, argc, redir_index, redirects, pipes, numpipes, pipes[0] + 1, argc, 3 ); 
        //LOCATION: 1 = left, 2 = middle, 3 = right

        dup2(mypipe[0], STDIN_FILENO);
        close(mypipe[1]);
        close(mypipe[0]); 
        execvp(myargs[0], myargs); 
        fprintf(stderr, "Error in second\n");
    }        
}

