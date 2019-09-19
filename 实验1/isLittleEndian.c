#include <stdio.h>
#include <stdlib.h>
//1170300817 林之浩
int main()

{
    int i=1;
    char *b=(char *)&i;//char是一个字节，强制将char 类型的b指向i，则b指向的一定是i的低地址
    if(*b==1)
        printf("Little Endian\n");
    else
        printf("Big Endian\n");
    return 0;
}
