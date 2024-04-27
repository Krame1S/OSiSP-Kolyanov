#include <stdio.h>
#include <string.h>

#include "funcs.h"

int main(int argc, char** argv) {
    char cmd[3];
    
    read_user_command(cmd);

    while(process_command(cmd)) {
        read_user_command(cmd);
    }
    
    return 0;
}