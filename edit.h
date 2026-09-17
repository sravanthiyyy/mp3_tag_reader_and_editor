#ifndef edit_H
#define edit_H

#include "types.h"
//structure for edit
typedef struct 
{
    //to store new tag 
    char *edit_content;
    //mp3 file name to be edited
    char *fname;
    //original mp3 file pointer
    FILE *fptr_edit;
    //temp edited mp3 file pointer
    FILE *fptr_new;
}edit;


Status validate(char *argv[], edit * edit_fileinfo);

Status opening_file(edit * edit_fileinfo);

Status skip_data(FILE *src, FILE *dest);

Status edit_option(char *argv[], edit *edit_fileinfo);

Status edit_data(FILE *src, FILE *dest, char *argv[]);

Status copy_remaining(FILE * src, FILE* dest);


#endif