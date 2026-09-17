#ifndef view_H
#define view_H

#include "types.h"
#include<stdio.h>

//structure for view
typedef struct
{
    //name of mp3 file
    char *fname;
    char title[50];
    char artist[50];
    char album[50];
    char year[5];
    char genre[30];
    char comm[50];
    //pointer to the file 
    FILE *fptr_mp3;
}view;


OperationType check_operation_type(char *argv[]);

Status validate_extn(char *argv[], view * view_fileinfo);

Status open_file(view * view_fileinfo);

Status display_data(view * view_fileinfo);

Status extract_data(FILE* fptr, char *data);

#endif