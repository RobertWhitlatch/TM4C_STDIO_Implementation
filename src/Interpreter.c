// Interpreter.c

#include "Master.h"

//-------------------getString--------------------//
// Constructs a string from Console input.
// Input: buffer to hold string, length of buffer
// Output: -1 if string not complete, strlen() if enter pressed or length met
//------------------------------------------------//
int getString(char* buff, int buff_length) {
    static int index = 0;
    char ch = EOF;
    while(1){
        ch = fgetc(uart);
        if(ch != EOF) {
            fputc(ch, uart);
            if(ch == '\n' || ch == '\r' || index == buff_length) {
                buff[index] = 0;
                int result = index;
                index = 0;
                return (result);
            } else if(ch == 8) {
                index--;
            } else {
                buff[index] = ch;
                index++;
            }
        }
    }
}

void Interpreter(void) {

    char in_char = EOF;
    int value;
    char str_buff[30];
    FILE* file;

    CLEAR_TERMINAL
    fprintf(uart, "Welcome to XeroOS!\n>");
    while(1) {
        in_char = fgetc(uart);
        fprintf(uart, "%c\n", in_char);
        switch(in_char) {
            case 'f':
                fprintf(uart, "Welcome to XeroFS!\nFS>");
                while(1){
                    in_char = fgetc(uart);
                    fprintf(uart, "%c\n", in_char);
                    switch(in_char) {
                        case '1':
                            eFile_Directory(uart);
                            break;
                        case '2':
                            fprintf(uart, "Specify which file to display.\nFS>");
                            getString(str_buff, 30);
                            fprintf(uart, "\n");
                            eFile_DisplayFile(uart, str_buff);
                            fprintf(uart, "\n");
                            break;
                        case '3':
                            fprintf(uart, "Specify which file to delete.\nFS>");
                            getString(str_buff, 30);
                            fprintf(uart, "\nYou are about to delete File \"%s\".\n"
                                          "Is this what you want? (Y to continue, any other escapes)\nFS>",
                                          str_buff);
                            in_char = fgetc(uart);
                            fprintf(uart, "%c\n", in_char);
                            if(in_char == 'Y'){
                                eFile_Delete(str_buff);
                                fprintf(uart, "File %s has been deleted.\n", str_buff);
                                break;
                            }
                            fprintf(uart, "No harm has come to the File or its' contents.\n");
                            break;
                        case '4':
                            fprintf(uart, "You are about to format the microSD card!\n"
                                          "This will result in complete erasure of the disk.\n"
                                          "Is this what you want? (Y to continue, any other escapes)\nFS>");
                            in_char = fgetc(uart);
                            fprintf(uart, "%c\n", in_char);
                            if(in_char == 'Y'){
                                eFile_Format();
                                fprintf(uart, "The disk is now formatted and blank.\n");
                                break;
                            }
                            fprintf(uart, "No harm has come to the disk or its' contents.\n");
                            break;
                        case '5':
                            fprintf(uart,"This operation allows data to be send over the serial interface.\n");
                            fprintf(uart,"The data will be written into a file on the uSD card.\n");
                            fprintf(uart,"This will require the full attention of the processer,\n");
                            fprintf(uart,"and as such, the OS will be disabled during this period.\n");
                            fprintf(uart,"First, please specify the total size of the file to be transferred.\nFS>");
                            getString(str_buff, 30);
                            value = atoi(str_buff);
                            fprintf(uart,"\nNext, supply a name for the file to be created (1-7 characters total)\nFS>");
                            getString(str_buff, 30);
                            fprintf(uart, "\nYou are about to create File \"%s\" of size %d.\n"
                                          "Is this what you want? (Y to continue, any other escapes to FS Menu)\nFS>",
                                          str_buff, value);
                            in_char = fgetc(uart);
                            fprintf(uart, "%c\n", in_char);
                            if(in_char != 'Y'){
                                fprintf(uart, "No File has been created.\n");
                                break;
                            }
                            eFile_Create(str_buff);
                            file = eFile_WOpen(str_buff);
                            if(file == null){
                                fprintf(uart, "Error opening file, please try again. (perhaps with a different name)\n");
                                fprintf(uart, "You will now be returned to the FS menu.\n");
                                break;
                            }
                            fprintf(uart, "You may now send the file.");
                            for(int i = 0; i < value; i++){
                                fputc(fgetc(uart), file);
                            }
                            eFile_WClose();
                            fprintf(uart, "\nTransfer Complete!\n");
                            fprintf(uart, "The resulting file will now be displayed for verification.\n");
                            eFile_DisplayFile(uart,str_buff);
                            fprintf(uart, "\nIs this correct? (Y to keep, and other deletes file)\nFS>");
                            in_char = fgetc(uart);
                            if(in_char != 'Y'){
                                eFile_Delete(str_buff);
                                fprintf(uart, "File deleted, returning to FS Menu.\n");
                                break;
                            }
                            break;
                        case 'h':
                            fprintf(uart,
                                    "1 - Print File Directory\n"
                                    "2 - Print File\n"
                                    "3 - Delete File\n"
                                    "4 - Format Disk\n"
                                    "5 - Transfer File through Terminal\n"
                                    "h - Help Menu\n"
                                    "q - Return to Main Menu\n"
                                    "~ - Clear Screen\n"
                                    );
                            break;
                        case 'q':
                            goto FileExit;
                        case '~':
                            CLEAR_TERMINAL
                            break;
                        default:
                            break;
                    }
                    fprintf(uart, "FS>");
                }
                FileExit:
                break;
            case 'h':
                fprintf(uart,
                        "f - File System Menu\n"
                        "h - Help Menu\n"
                        "i - Information Page\n"
                        "z - Shutdown Safely\n"
                        "~ - Clear Screen\n"
                        );
                break;
            case 'i':
                fprintf(uart,
                        "Core Freqency = 80MHz\n"
                        "Interrupts = %s\n",
                        (CheckInterrupts()) ? "False" : "True"
                        );
                break;
            case 'z':
                eFile_Close();
                fprintf(uart, "Now safe to remove power.\n");
                break;
            case '~':
                CLEAR_TERMINAL
                break;
            default:
                break;
        }
        fprintf(uart, ">");
    }
}
