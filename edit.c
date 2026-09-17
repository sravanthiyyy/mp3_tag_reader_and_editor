#include<string.h>
#include<stdio.h>
#include "edit.h"
#include "types.h"

/*
 * Global variables
 * ---------------
 * count : used while copying flag bytes / skipping data
 * flag  : used to detect whether a valid edit option was given
 * ch    : single-byte buffer used for file copy
 */
int count=0,flag=1;
char ch;

/*
 * Function: validate
 * ------------------
 * Validates command-line arguments.
 * - Checks whether the 4th argument has ".mp3" extension
 * - Stores filename and edit content in structure
 */
Status validate(char *argv[], edit * edit_fileinfo)
{
    // Find extension in filename
    char *ext = strstr(argv[4],".");

    // Check if extension exists and is ".mp3"
    if(ext != NULL && strcmp(ext,".mp3") == 0)
    {
        // store mp3 filename
        edit_fileinfo->fname = argv[4];
        // store new tag text        
        edit_fileinfo->edit_content = argv[3]; 
        return e_success;
    }
    else
        return e_failure;
}

/*
 * Function: opening_file
 * ---------------------
 * Opens original MP3 file in read+write mode
 * Creates a new temporary file to store edited output
 */
Status opening_file(edit * edit_fileinfo)
{
    // Open original mp3 file
    edit_fileinfo->fptr_edit = fopen(edit_fileinfo->fname, "r+");

    if(edit_fileinfo->fptr_edit == NULL)
        return e_failure;

    // Open new output file
    edit_fileinfo->fptr_new = fopen("edited_song.mp3", "w+");

    if(edit_fileinfo->fptr_new != NULL)
        return e_success;
    else
        return e_failure;
}

/*
 * Function: skip_data
 * ------------------
 * Copies one complete ID3 frame as-is from source to destination
 * Used when the current frame is NOT the one to be edited
 */
Status skip_data(FILE *src, FILE *dest)
{
    long cur = ftell(src);    // save current position
    int tag_size=0,count=0;
    char tag[50],tit2_id[4];

    unsigned char size[4];

    // Read frame ID (e.g., TIT2, TPE1, etc.)
    if(fread(tit2_id,1,4,src)!=4)
        return e_failure;

    // Read frame size (sync-safe integer)
    if(fread(size,1,4,src)!=4)
        return e_failure;

    // Convert sync-safe size to normal integer
    tag_size = (size[0] << 21) | (size[1] << 14) | (size[2] << 7) | size[3];

    // Skip frame flags (2 bytes + encoding byte assumed)
    if(fseek(src,3,SEEK_CUR)!=0)
        return e_failure;

    // Safety check to avoid buffer overflow
    if (tag_size - 1 > sizeof(tag))
        return e_failure;

    // Read frame content
    if(fread(tag,1,tag_size-1,src)!=tag_size-1)
        return e_failure;

    long end = ftell(src);   // end position after reading frame

    // Go back to start of frame
    if(fseek(src,cur,SEEK_SET)!=0)
            return e_failure;

    long bytes = (end-cur);  // total bytes to copy

    // Copy entire frame byte-by-byte
    while (bytes-- > 0)
    {
        fread(&ch, 1, 1,src );
        fwrite(&ch, 1, 1, dest);
    }
    
    return e_success;
}

/*
 * Function: edit_data
 * ------------------
 * Edits a specific ID3 frame by replacing its content
 * Writes updated frame size and new text
 */
Status edit_data(FILE *src, FILE *dest, char *argv[])
{
    char tit2_id[4];
    char tsize[4];

    // New data size (+1 for encoding byte)
    int size = strlen(argv[3])+1;

    // Read frame ID
    if(fread(tit2_id,1,4,src)!=4)
        return e_failure;

    // Write same frame ID to destination
    if(fwrite(tit2_id,1,4,dest)!=4)
        return e_failure;

    // Read old frame size
    if(fread(tsize,1,4,src)!=4)
        return e_failure;

    // Convert old size (sync-safe)
    int tag_size = (tsize[0] << 21) | (tsize[1] << 14) | (tsize[2] << 7) | tsize[3];

    // Move back to overwrite frame size
    fseek(src,-4,SEEK_SET);

    {
        char frame_size[4];

        // Write new frame size (normal integer, NOT sync-safe)
        frame_size[0] = (size >> 24) & 0xFF;
        frame_size[1] = (size >> 16) & 0xFF;
        frame_size[2] = (size >> 8) & 0xFF;
        frame_size[3] =  size & 0xFF;

        if(fwrite(frame_size,1,4,dest)!=4)
            return e_failure;

        // Copy frame flags / encoding bytes
        while (count++ < 3)
        {
            fread(&ch, 1, 1, src);
            fwrite(&ch, 1, 1, dest);
        }

        // Write new tag text
        if(fwrite(argv[3],1,strlen(argv[3]),dest)!=strlen(argv[3]))
            return e_failure;
    }
    
    // Skip remaining old frame data
    if(fseek(src,tag_size-1,SEEK_CUR)!=0)
        return e_failure;

    printf("----------------------------------------------------------------------\n");
    printf("%51s\n","MP3 Tag Data Edited Successfully");
    printf("----------------------------------------------------------------------\n");

    return e_success;
}

/*
 * Function: edit_option
 * --------------------
 * Controls which ID3 frame to edit based on command-line option
 * Copies header, edits selected frame, skips others
 */
Status edit_option(char *argv[], edit *edit_fileinfo)
{
    // Copy first 10 bytes (ID3 header)
    while (ftell(edit_fileinfo->fptr_edit)!=10)
    {
        fread(&ch, 1, 1, edit_fileinfo->fptr_edit);
        fwrite(&ch, 1, 1, edit_fileinfo->fptr_new);
    }

    // Title
    if(strcmp(argv[2],"-t") == 0)
    {
        if (edit_data(edit_fileinfo->fptr_edit, edit_fileinfo->fptr_new, argv) == e_failure)
            return e_failure;
        printf("Title     :  %s\n",argv[3]);
        flag=0;
    }
    else
        skip_data(edit_fileinfo->fptr_edit,edit_fileinfo->fptr_new);

    // Artist
    if(strcmp(argv[2],"-a") == 0)
    {
        if (edit_data(edit_fileinfo->fptr_edit, edit_fileinfo->fptr_new, argv) == e_failure)
            return e_failure;
        printf("Artist     :  %s\n",argv[3]);
        flag=0;
    }
    else
        skip_data(edit_fileinfo->fptr_edit,edit_fileinfo->fptr_new);

    // Album
    if(strcmp(argv[2],"-A") == 0)
    {
        if (edit_data(edit_fileinfo->fptr_edit, edit_fileinfo->fptr_new, argv) == e_failure)
            return e_failure;
        printf("Album     :  %s\n",argv[3]);
        flag=0;
    }
    else
        skip_data(edit_fileinfo->fptr_edit,edit_fileinfo->fptr_new);

    // Year
    if(strcmp(argv[2],"-y") == 0)
    {
        if (edit_data(edit_fileinfo->fptr_edit, edit_fileinfo->fptr_new, argv) == e_failure)
            return e_failure;
        printf("Year     :  %s\n",argv[3]);
        flag=0;
    }
    else
        skip_data(edit_fileinfo->fptr_edit,edit_fileinfo->fptr_new);

    // Genre
    if(strcmp(argv[2],"-m") == 0)
    {
        if (edit_data(edit_fileinfo->fptr_edit, edit_fileinfo->fptr_new, argv) == e_failure)
            return e_failure;
        printf("Genre     :  %s\n",argv[3]);
        flag=0;
    }
    else
        skip_data(edit_fileinfo->fptr_edit,edit_fileinfo->fptr_new);

    // Comment
    if(strcmp(argv[2],"-c") == 0)
    {
        if (edit_data(edit_fileinfo->fptr_edit, edit_fileinfo->fptr_new, argv) == e_failure)
            return e_failure;
        printf("Comments     :  %s\n",argv[3]);
        flag=0;
    }

    // Invalid option handling
    if(flag)
    {
        printf("\nError: Enter a valid edit option\n");
        printf("USAGE:\n"
            "To edit please pass like: ./a.out -t/-a/-A/-m/-y/-c <changing text> <mp3_filename>\n");
        return e_failure;
    }

    // Copy remaining audio data
    if(copy_remaining(edit_fileinfo->fptr_edit, edit_fileinfo->fptr_new) == e_failure)
        return e_failure;

    flag=1;
    return e_success;
}

/*
 * Function: copy_remaining
 * -----------------------
 * Copies remaining audio data (after ID3 tags)
 * Ensures audio is not lost
 */
Status copy_remaining(FILE * src, FILE* dest)
{
    while (fread(&ch, 1, 1,src ))
        fwrite(&ch, 1, 1, dest);

    printf("----------------------------------------------------------------------\n");

    return e_success;
}
