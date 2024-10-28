#include <stdio.h>
#include <stdlib.h>

int main(){
    printf("Using system function for running ps program\n");
    system("ps -ax");
    printf("ps completed\n");
    return 0;
}