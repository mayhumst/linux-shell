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

int cd(char* arg);
int simpleprog(char ** argv, int argc, int redir_index, int redirects[10][2]);
char** pipe_piece(char ** argv, int argc, int redir_index, int redirects[10][2], int pipes[10], int numpipes, int begin, int end, int location); 
void pipe_total(char ** argv, int argc, int redir_index, int redirects[10][2], int pipes[10], int numpipes); 
void pipe_special(char ** argv, int argc, int redir_index, int redirects[10][2], int pipes[10], int numpipes);