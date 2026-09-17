#include<string.h>
#include<stdlib.h>
#include "view.h"
#include "types.h"


OperationType check_operation_type(char *argv[])
{
    // check if user selected view option
    if(strcmp(argv[1],"-v") == 0)
        return e_view;

    // check if user selected edit option
    else if (strcmp(argv[1],"-e") == 0)
        return e_edit;

    // check if user requested help
    else if(strcmp(argv[1],"--help") == 0)
        return e_help;

    // unsupported command-line option
    else
        return e_unsupported;
}


Status validate_extn(char *argv[], view * view_fileinfo)
{
    // find the file extension from filename
    char *ext = strstr(argv[2],".");

    // validate whether the extension is .mp3
    if(ext != NULL && strcmp(ext,".mp3") == 0)
    {
        // store valid mp3 filename in structure
        view_fileinfo->fname = argv[2];
        return e_success;
    }
    else
        return e_failure;
}


Status open_file(view * view_fileinfo)
{
    // open the mp3 file using filename stored in structure
    view_fileinfo->fptr_mp3 = fopen(view_fileinfo->fname, "r+");

    if(view_fileinfo->fptr_mp3 != NULL)
        return e_success;
    else
        return e_failure;
}


Status extract_data(FILE *fptr, char *data)
{
    // variable to store ID3 frame size
    int tag_size=0;

    // temporary buffer to hold tag content
    char tag[50];

    // array to read 4-byte sync-safe frame size
    unsigned char size[4];

    // read 4 bytes of frame size
    if(fread(size,1,4,fptr)!=4)
        return e_failure;

    // convert sync-safe size to normal integer
    tag_size = (size[0] << 21) | (size[1] << 14) | (size[2] << 7) | size[3];

    // skip frame flags and encoding byte
    if(fseek(fptr,3,SEEK_CUR)!=0)
        return e_failure;

    // read actual frame text data
    if(fread(tag,1,tag_size-1,fptr)!=tag_size-1)
        return e_failure;

    // terminate string
    tag[tag_size-1]='\0';

    // copy extracted data into destination buffer
    strcpy(data,tag);

    return e_success;
}


Status display_data(view * view_fileinfo)
{
    // display output header
    printf("----------------------------------------------------------------------\n");
    printf("%40s\n","MP3 Tag Data");
    printf("----------------------------------------------------------------------\n");

    {
        // read and display ID3 identifier
        char buffer[4];
        if(fread(buffer,1,3,view_fileinfo->fptr_mp3)!=3)
            return e_failure;

        buffer[3]='\0';
        printf("%s ",buffer);
    }

    {
        // read and display ID3 version information
        unsigned char version[3];
        if(fread(version,1,2,view_fileinfo->fptr_mp3)!=2)
            return e_failure;

        printf("V2.%d.%d\n",version[0],version[1]);
    }

    {
        // move file pointer to first frame
        char tit2_id[5];

        if(fseek(view_fileinfo->fptr_mp3,5,SEEK_CUR)!=0)
            return e_failure;

        // read Title frame ID
        if(fread(tit2_id,1,4,view_fileinfo->fptr_mp3)!=4)
            return e_failure;

        tit2_id[4]='\0';

        // extract and display Title
        if(extract_data(view_fileinfo->fptr_mp3, view_fileinfo->title) == e_success) 
            printf("Title     :  %s\n",view_fileinfo->title);
        else
            return e_failure;
    }

    {
        // read Artist frame ID
        char artist_id[5];

        if(fread(artist_id,1,4,view_fileinfo->fptr_mp3)!=4)
            return e_failure;

        artist_id[4]='\0';

        // extract and display Artist
        if(extract_data(view_fileinfo->fptr_mp3, view_fileinfo->artist) == e_success) 
            printf("Artist    :  %s\n",view_fileinfo->artist);
        else
            return e_failure;
    }

    {
        // read Album frame ID
        char album_id[5];

        if(fread(album_id,1,4,view_fileinfo->fptr_mp3)!=4)
            return e_failure;

        album_id[4]='\0';

        // extract and display Album
        if(extract_data(view_fileinfo->fptr_mp3, view_fileinfo->album) == e_success) 
            printf("Album     :  %s\n",view_fileinfo->album);
        else
            return e_failure;
    }

    {
        // read Year frame ID
        char tyer_id[5];

        if(fread(tyer_id,1,4,view_fileinfo->fptr_mp3)!=4)
            return e_failure;

        tyer_id[4]='\0';

        // extract and display Year
        if(extract_data(view_fileinfo->fptr_mp3, view_fileinfo->year) == e_success) 
            printf("Year      :  %s\n",view_fileinfo->year);
        else
            return e_failure;
    }

    {
        // read Genre frame ID
        char tcon_id[5];

        if(fread(tcon_id,1,4,view_fileinfo->fptr_mp3)!=4)
            return e_failure;

        tcon_id[4]='\0';

        // extract and display Genre
        if(extract_data(view_fileinfo->fptr_mp3, view_fileinfo->genre) == e_success) 
            printf("Genre     :  %s\n",view_fileinfo->genre);
        else
            return e_failure;
    }

    {
        // read Comment frame ID
        char comm_id[5];

        if(fread(comm_id,1,4,view_fileinfo->fptr_mp3)!=4)
            return e_failure;

        comm_id[4]='\0';

        // extract and display Comment
        if(extract_data(view_fileinfo->fptr_mp3, view_fileinfo->comm) == e_success) 
            printf("Comments  :  %s\n",view_fileinfo->comm);
        else
            return e_failure;
    }

    
    printf("----------------------------------------------------------------------\n");

    return e_success;
}
