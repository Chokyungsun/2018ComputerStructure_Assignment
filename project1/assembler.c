#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*******************************************************
 * Function Declaration
 *
 *******************************************************/
char *change_file_ext(char *str);
void parse_command(char* command, int* argc, char*** argv);
void save_data(int count, char** argv);
void save_text(int count, int label_count, char**argv);
void labelling(int count, int instruction_count, char** argv);
void release_argv(int argc, char*** argv);
void decimal_to_binary(int n, int size, char* result);
void hex_to_binary(char* hex, int length, int size, char* result);

/*******************************************************
*Structures
*
*******************************************************/
struct data_section{
    char data_name[50];
    unsigned int data_address;
    char data_type[50];
    char data_value[50];
};

struct label{
    unsigned int label_address;
    char label_name[50];
};

struct instruction{
    unsigned int text_address;
    char text_type;
    char text_command[50];
    char text_label[50];
};

struct R_type{
    int opcode;
    int rs;
    int rt;
    int rd;
    int sa;
    int funct;
};

struct I_type{
    int opcode;
    int rs;
    int rt;
    int immediate;
};

struct J_type{
    int opcode;
    int jump_target;
};


int data_count = 0;
int instruction_count = 0;
int label_count = 0;
struct data_section* data[16] = { NULL, };
struct instruction* instructions[128] = { NULL, };
struct label* labels[16] = { NULL, };

char* Rtype_commands[] = {"and", "or", "sll", "srl", "addu", "nor", "subu", "jr", "sltu"};
char* Itype_commands[] = {"lui", "ori", "addiu", "bne", "beq", "ori", "andi", "lw", "sw", "sltiu"};
char* Jtype_commands[] = {"j", "jal"};

/*******************************************************
 * Function: main
 *
 * Parameters:
 *  int
 *      argc: the number of argument
 *  char*
 *      argv[]: array of a sting argument
 *
 * Return:
 *  return success exit value
 *
 * Info:
 *  The typical main function in C language.
 *  It reads system arguments from terminal (or commands)
 *  and parse an assembly file(*.s).
 *  Then, it converts a certain instruction into
 *  object code which is basically binary code.
 *
 *******************************************************/
int main(int argc, char *argv[])
{
    FILE *input, *output;
    char *filename;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <*.s>\n", argv[0]);
        fprintf(stderr, "Example: %s sample_input/example?.s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    filename = strdup(argv[1]); // strdup() is not a standard C library but fairy used a lot.
    if(change_file_ext(filename) == NULL) {
        fprintf(stderr, "'%s' file is not an assembly file.\n", filename);
        exit(EXIT_FAILURE);
    }

    output = fopen(filename, "w");
    if (output == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    // process();

    //Read file line by line
    if(input!= NULL){
        char strTemp[255];
        char* sPtr;
        int argc;
        char** argv;

        sPtr = fgets(strTemp, sizeof(strTemp), input);
            //printf("%s", sPtr);

        parse_command(sPtr, &argc, &argv);

            //Seperate data section and text section
        if(strcmp(argv[0], ".data") == 0) printf("********it's data section.***********\n");

        while(strcmp(argv[0], ".text") != 0){
            release_argv(argc, &argv);

            sPtr = fgets(strTemp, sizeof(strTemp), input);
            printf("%s", sPtr);

            parse_command(sPtr, &argc, &argv);
            if(strcmp(argv[0], ".text") == 0) break;

            save_data(data_count, argv);
            data_count++;

            //printf("%s", argv[0]);
        }

            //checking
            /*for(int i=0; i<data_count;i++){  
                printf("data name = %s, data_address = %d, data_value = %s, data_type = %s\n", data[i]->data_name, data[i]->data_address, data[i]->data_value, data[i]->data_type);
            }
            printf("command num = %d\n", data_count);*/


        if(strcmp(argv[0], ".text") == 0) printf("********it's text section.***********\n");
        while(!feof(input)){
            release_argv(argc, &argv);

            sPtr = fgets(strTemp, sizeof(strTemp), input);
            printf("%s", sPtr);

            parse_command(sPtr, &argc, &argv);
            //if(argv[0] == NULL) break; 

            char label_dect[10]="";
            int i;

            if (argv[0][strlen(argv[0])-1] ==  ':'){

                i = strlen(argv[0])-1;
                strncpy(label_dect, argv[0], i);
                strcpy(argv[0], label_dect);

                labelling(label_count, instruction_count, argv);
                label_count++;
            }
            else if (argv[0][strlen(argv[0])-1] !=  ':'){
                if(strcmp(argv[0], "") == 0) break;

                if(strcmp(argv[0], "la") == 0){
                    for(int i =0; i < data_count ; i++){
                        if(strcmp(data[i]->data_name, argv[2]) == 0){
                            char add[16];
                            int t;
                            char temp[50];

                            //in case that the lower 16 bit address is 0x0000
                            if((data[i]->data_address % 16 == 0)&&(data[i]->data_address % (16*16) ==0 )&&(data[i]->data_address % (16*16*16)==0)&&(data[i]->data_address % (16*16*16*16)==0)){
                                
                                t = (data[i]->data_address >> 16);
                                sprintf(add,"%d\n",t);
                                strcpy(temp, "lui ");
                                strcat(temp, argv[1]);
                                strcat(temp, add);
                                strcpy(sPtr, temp);

                                parse_command(sPtr, &argc, &argv);
                                save_text(instruction_count, label_count, argv);
                                instruction_count++;
                                break;
                            }

                            //in case that the lower 16 bit address is not 0x0000
                            else{
                                t = (data[i]->data_address >> 16);
                                sprintf(add,"%d\n",t);
                                strcpy(temp, "lui ");
                                strcat(temp, argv[1]);
                                strcat(temp, add);
                                strcpy(sPtr, temp);

                                parse_command(sPtr, &argc, &argv);
                                save_text(instruction_count, label_count, argv);
                                instruction_count++;

                                t = (data[i]->data_address << 16);
                                sprintf(add,"%d\n",t);
                                strcpy(temp, "ori ");
                                strcat(temp, argv[1]);
                                strcat(temp, argv[1]);
                                strcat(temp, add);
                                strcpy(sPtr, temp);

                                parse_command(sPtr, &argc, &argv);
                                save_text(instruction_count, label_count, argv);
                                instruction_count++;
                                break;
                            }
                        }
                    }
                }

                else{
                    save_text(instruction_count, label_count, argv);
                    instruction_count++;
                }
            }

            //printf("%s", argv[0]);
        }

    }

    //printf("data count: %d\n",data_count);
    //printf("instruction count: %d\n",instruction_count);

//write object file
/************format***************
<text section size>
<data section size>
<instruction 1>
.
.
<instruction n>
<data value 1>
.
.
<data value n>
************************************/

// text and data section write
char* output_text_size;
decimal_to_binary(instruction_count, 30, output_text_size);
fprintf(output, output_text_size);
char* output_data_size;
decimal_to_binary(data_count, -1, output_data_size);
fprintf(output, output_data_size);

/*instruction output
*
*
*/

// data value write
/*char* output_data_value;
for(int i = 0; i< data_count; i++){
    //hex value
    if((data[i]->data_value)[1] =='x'){
        printf("%d\n", i);
        int len = strlen(data[i]->data_value) - 2;
        hex_to_binary(data[i]->data_value, len, -1, output_data_value);
        printf("%s\n", output_data_value);
        fprintf(output, output_data_value);
    }
    //decimal value
    else{
        decimal_to_binary(data[i]->data_value, -1, output_data_value);
        printf("%s\n", output_data_value);
        fprintf(output, output_data_value);
    }
}
*/

//free memory
for (int i = 0; i < data_count; ++i) {
    if (data[i]) {
        free(data[i]);
        data[i] = NULL;
    }
}
for (int i = 0; i < instruction_count; ++i) {
    if (instructions[i]) {
        free(instructions[i]);
        instructions[i] = NULL;
    }
}
for (int i = 0; i < label_count; ++i) {
    if (labels[i]) {
        free(labels[i]);
        labels[i] = NULL;
    }
}

fclose(input);
fclose(output);
exit(EXIT_SUCCESS);
}


/*******************************************************
 * Function: change_file_ext
 *
 * Parameters:
 *  char
 *      *str: a raw filename (without path)
 *
 * Return:
 *  return NULL if a file is not an assembly file.
 *  return empty string
 *
 * Info:
 *  This function reads filename and converst it into
 *  object extention name, *.o
 *
 *******************************************************/
char *change_file_ext(char *str)
{
    char *dot = strrchr(str, '.');

    if (!dot || dot == str || (strcmp(dot, ".s") != 0)) {
        return NULL;
    }

    str[strlen(str) - 1] = 'o';
    return "";
}

void parse_command(char* command, int* argc, char*** argv){

//memory allocation
    *argc = 0;
    (*argv) = (char**)malloc(sizeof(char*)*40);

//separate command  using strtok function
//delimiter is ' \n\t,$'

    char* temp_c = strtok(command, " \n\t,$");
    //exception when you write nothing
    if(temp_c == NULL){
        *argc = 1;
        (*argv)[0] = "";
        return;
    }

    //set argc and argv
    while(temp_c != NULL){
        (*argv)[*argc] = (char*)malloc(sizeof(char)*100);
        strcpy((*argv)[*argc], temp_c);
        temp_c = strtok(NULL, " \n\t,$");
        (*argc)++;
    }
    
    //checking
    
    /*printf("%d index\n",*argc);
    for(int i=0; i<(*argc) ; i++){
        printf("argv[%d] = %s\n", i, (*argv)[i]);
    }*/
    

    return;
}
void release_argv(int argc, char*** argv) {
  for (int i = 0; i < argc; ++i) {
    free((*argv)[i]);
}
free(*argv);
*argv = NULL;
}

void save_data(int count, char** argv){
    data[count] = malloc(sizeof(struct data_section));

    int length = strlen(argv[0])-1;
    strncpy(data[count]->data_name, argv[0], length);
    if(count ==0) data[count]->data_address = 0x10000000;
    else data[count]->data_address = 0x10000000 + (count)*4;

    strcpy(data[count]->data_type, argv[1]);
    strcpy(data[count]->data_value, argv[2]);

    //checking
    //printf("%d\n", count);
    printf("data name = %s, data_address = %d, data_value = %s, data_type = %s\n", data[count]->data_name, data[count]->data_address, data[count]->data_value, data[count]->data_type);

    return;
}

void labelling(int count, int instruction_count, char** argv){
    labels[count] = malloc(sizeof(struct label));

    strcpy(labels[count]->label_name , argv[0]);
    if(instruction_count ==0) labels[count]->label_address = 0x00400000;
    else labels[count]->label_address = 0x00400000 + instruction_count *4;

    //checking
    /*printf("count: %d, instruction_count: %d\n", count, instruction_count);
    printf("label name = %s, label_address = %d\n", labels[count]->label_name, labels[count]->label_address);
    */

    return;
}

void save_text(int count, int l_count, char**argv){
    instructions[count] = malloc(sizeof(struct instruction));

    strcpy(instructions[count]->text_command, argv[0]);
    if(count == 0) instructions[count]->text_address = 0x00400000;
    else instructions[count]->text_address = 0x00400000 + (count)*4;

    int label = l_count-1;
    strcpy(instructions[count]->text_label, labels[label]->label_name);

    for(int i=0; i< 9; i++){
        if(strcmp(Rtype_commands[i], instructions[count]->text_command) == 0){
            instructions[count]->text_type = 'R';
            break;
        }
    }

    for(int i=0; i< 10; i++){
        if(strcmp(Itype_commands[i], instructions[count]->text_command) == 0){
            instructions[count]->text_type = 'I';
            break;
        }
    }

    for(int i=0; i< 2; i++){
        if(strcmp(Jtype_commands[i], instructions[count]->text_command) == 0){
            instructions[count]->text_type = 'J';
            break;
        }
    }

    //checking
    printf("instruction num: %d\n", count);
    printf("label num = %d\n", label);

    printf("instruction command = %s, text_address = %d, text_label = %s, text_type = %c\n", 
        instructions[count]->text_command, instructions[count]->text_address, instructions[count]->text_label, 
        instructions[count]->text_type);

}

void decimal_to_binary(int n, int size, char* result){
    int c, d;
    int count = 0;
    char* str;

    str = (char*)malloc(32+1);
    if(str == NULL)
        exit(EXIT_FAILURE);

    for(c = 31; c >= 0; c--){
        d = n >> c;
        if(d & 1) *(str + count) = 1 +'0';
        else *(str + count) = 0 +'0';

        count++;
    }
    *(str + count) = '\0';

    //cut the string 
    int binary_len = strlen(str);
    int shifted_size;
    int t, j;
    if(size == 0){
        printf("Invalid size \n");
        exit(EXIT_FAILURE);
    }
    else if(size == -1 || size == binary_len){
        strcpy(result, str);
    }
    else if(size > binary_len){
        shifted_size =  size - binary_len;

        for (j = 0; j < shifted_size; j++){
            result[j] = '0';
        }
        strcat(result, str);

    }
    else{
        shifted_size =  binary_len- size;

        t = 0;
        for (j = shifted_size; j < binary_len; j++){
            result[t] = str[j];
            t++;
        }
    }
    return;
}

void hex_to_binary(char* hex, int length, int size, char* result){
    char* binary = (char*)malloc(length*4 +1);
    int i = 2;

    if(binary == NULL)
        exit(EXIT_FAILURE);

    while(hex[i] != '\0'){
        switch(hex[i])
        {
            case '0':
                strcat(binary, "0000");
                break;
            case '1':
                strcat(binary, "0001");
                break;
            case '2':
                strcat(binary, "0010");
                break;
            case '3':
                strcat(binary, "0011");
                break;
            case '4':
                strcat(binary, "0100");
                break;
            case '5':
                strcat(binary, "0101");
                break;
            case '6':
                strcat(binary, "0110");
                break;
            case '7':
                strcat(binary, "0111");
                break;
            case '8':
                strcat(binary, "1000");
                break;
            case '9':
                strcat(binary, "1001");
                break;
            case 'a':
            case 'A':
                strcat(binary, "1010");
                break;
            case 'b':
            case 'B':
                strcat(binary, "1011");
                break;
            case 'c':
            case 'C':
                strcat(binary, "1100");
                break;
            case 'd':
            case 'D':
                strcat(binary, "1101");
                break;
            case 'e':
            case 'E':
                strcat(binary, "1110");
                break;
            case 'f':
            case 'F':
                strcat(binary, "1111");
                break;
            default:
                printf("Invalide hex num.");

        }
        i++;
    }
    printf("%s\n", binary);

    //cut the string 
    int binary_len = strlen(binary);
    int shifted_size;
    int t, j;

    if(size == 0){
        printf("Invalid size \n");
        exit(EXIT_FAILURE);
    }
    else if(size == -1 || size == binary_len){
        strcpy(result, binary);
    }
    else if(size > binary_len){
        shifted_size =  size - binary_len;

        for (j = 0; j < shifted_size; j++){
            result[j] = '0';
        }
        strcat(result, binary);

    }
    else{
        shifted_size =  binary_len- size;
        t = 0;
        for (j = shifted_size; j < binary_len; j++){
            result[t] = binary[j];
            t++;
        }
    }
    return;
}