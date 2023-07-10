#include <stdio.h>

int main(int argc, char** argv){
    int array[12];
    array[0]='A';
    int *ptr = array;
    printf("value of ptr: 0x%x-->%c \n", ptr, *ptr);
    int temp = *(ptr+3);
    int temp2 = ptr[3];

    printf("Temp-->0x%d, temp2-->0x%d \n", temp, temp2);
    return 0;
}