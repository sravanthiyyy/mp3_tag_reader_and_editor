#include<stdio.h>
#include "view.h"
#include "edit.h"

void print_help(void);

int main(int argc, char *argv[])
{
    // Structures to hold view and edit related file information
    view view_fileinfo;
    edit edit_fileinfo;

    { 
        // -------- Argument count validation --------
        // If no operation flag is given, show help
        if(argc < 2)
        {
            print_help();
            return 1;
        }

        // For view operation (-v) exactly 3 arguments are required
        // For edit operation (-e) exactly 5 arguments are required
        if ((argc != 3 && check_operation_type(argv) == e_view) ||
            (argc != 5 && check_operation_type(argv) == e_edit))
        {
            print_help();
            return 1;
        }
    }
    
    // -------- VIEW OPERATION --------
    if(check_operation_type(argv) == e_view)
    {
        { 
            // Validate whether the given file has .mp3 extension
            if(validate_extn(argv, &view_fileinfo) == e_failure)
            {
                printf("\nError: Invalid File (Only .mp3 Files are Allowed)\n");
                return 1;
            }
        }

        {
            // Open the mp3 file for reading tag information
            if(open_file(&view_fileinfo) == e_failure)
            {
                printf("\nError: File opening failed\n");
                return 1;
            }
        }

        {
            // Read and display all ID3 tag data from the mp3 file
            if(display_data(&view_fileinfo) == e_success);
            else
            {
                printf("Error: Display file info failed");
            }
        }
    }

    // -------- EDIT OPERATION --------
    if(check_operation_type(argv) == e_edit)
    {
        {
            // Validate mp3 file and edit option arguments
            if(validate(argv, &edit_fileinfo) == e_failure)
            {
                printf("\nError: Invalid File (Only .mp3 Files are Allowed)\n");
                return 1;
            }

            {
                // Open original mp3 file and temporary edited file
                if(opening_file(&edit_fileinfo) == e_failure)
                {
                    printf("\nError: File opening failed\n");
                    return 1;
                }
            }

            {
                // Perform tag editing based on the given option
                if(edit_option(argv, &edit_fileinfo) == e_success)
                {
                    // Remove the original mp3 file
                    if(remove(argv[4]) != 0)
                    {
                        printf("\nError: Failed to remove file\n");
                        return 1;
                    }

                    // Rename edited file as the original mp3 file
                    if(rename("edited_song.mp3", argv[4]) != 0)
                    {
                        printf("\nError: Failed to rename file\n");
                        return 1;
                    }
                }
            }
        }
    }

    // -------- HELP OPERATION --------
    if(check_operation_type(argv) == e_help)
    {
        // Display help menu
        print_help();
    }

    // -------- UNSUPPORTED OPERATION --------
    if(check_operation_type(argv) == e_unsupported)
    {
        // Invalid flag provided by the user
        printf("\nInvalid operation\n");
        print_help();
        return 1;
    }
}

void print_help(void)
{
    // Prints usage instructions for the mp3 tag application
    printf("----------------------HELP MENU------------------\n");
    printf("Usage:\n");
    printf(" View mp3 tags :  -v <file.mp3>\n");
    printf(" Edit mp3 tags : -e <option> <value> <file.mp3>  \n");
    printf(" For help : -h \n\n");

    // Lists all supported edit options
    printf("Edit options are:\n");
    printf("  -t  -> edit song title\n");
    printf("  -a  -> edit artist name\n");
    printf("  -A  -> edit album name\n");
    printf("  -y  -> edit year\n");
    printf("  -m  -> edit genre/content\n");
    printf("  -c  -> edit comment\n");
    printf("-------------------------------------------------\n");
}

