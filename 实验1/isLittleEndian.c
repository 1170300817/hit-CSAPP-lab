#include <stdio.h>
#include <stdlib.h>
//1170300817 ��֮��
int main()

{
    int i=1;
    char *b=(char *)&i;//char��һ���ֽڣ�ǿ�ƽ�char ���͵�bָ��i����bָ���һ����i�ĵ͵�ַ
    if(*b==1)
        printf("Little Endian\n");
    else
        printf("Big Endian\n");
    return 0;
}
