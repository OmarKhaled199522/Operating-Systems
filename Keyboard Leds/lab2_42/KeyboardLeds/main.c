#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void set_led_state(char led[], int state){

    
    char file_path[100] = "/sys/kernel/keyboard/";
    strcat(file_path, led);
    char instruction[100] = "echo \"";

    if(state) strcat(instruction, "1\" >> ");
    else strcat(instruction, "0\" >> ");

    char permInstruction[100] = "sudo chmod 777 ";
    strcat(instruction, file_path);
    strcat(permInstruction, file_path);
    system(permInstruction);
    system(instruction);

}


char get_led_state(char led[]){
    
    char file_path[100] = "/sys/kernel/keyboard/";
    strcat(file_path, led);

    char state;
    FILE *keyboard_led_file;
    keyboard_led_file = fopen(file_path, "r");

    if (keyboard_led_file) {

        state = getc(keyboard_led_file);
        fclose(keyboard_led_file);
    }

    return state;
}


// 1   2   3
//set caps on
int main(int argc, char *argv[])
{

   if(argc < 3) return -1;

    char *process = argv[1];
    char *led = argv[2];
    char *state = argv[3];

    if(strcmp(process, "set") == 0 && argc > 3){

        if(strcmp(state, "on") == 0) set_led_state(led, 1);
        else set_led_state(led, 0);

    } else if(strcmp(process, "get") == 0){

        printf("%sLock is : %c\n", led, get_led_state(led));

    } else return -1;

    return 0;
}

