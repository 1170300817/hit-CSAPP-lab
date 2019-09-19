#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len)
{
    size_t i=0;
    printf("µØÖ·£º%p      ", &start[i]);
    printf("ÄÚÈİ£º");
    for (i = 0; i < len; i++)
        printf("%.2x ",start[i]);
    printf("\n");
}
int main()
{
    short short1 = 12;
    printf("short_1=%d\n",short1);
    show_bytes((byte_pointer) &short1, sizeof(short));

    int int_1 =300817;
    printf("int_1=%d\n",int_1);
    show_bytes((byte_pointer) &int_1, sizeof(int));

    long long_1 =702011;
    printf("long_1=%d\n",long_1);
    show_bytes((byte_pointer) &long_1, sizeof(long));

    float float_1 =702011.11;
    printf("float_1=%f\n",float_1);
    show_bytes((byte_pointer) &float_1, sizeof(float));

    double double_1 =9702011.11;
    printf("double_1=%f\n",double_1);
    show_bytes((byte_pointer) &double_1, sizeof(double));

    char char_1 ='b';
    printf("char_1=%c\n",char_1);
    show_bytes((byte_pointer) &char_1, sizeof(char));

    char *h="abc";
    printf("*h=%p\n",&h);
    show_bytes((byte_pointer) h, strlen(h));

    printf("\n");
    printf("\n");

    struct STU
    {
        char name[3];
        int age;
        float height;
    };
    struct STU lin = {"lzh",20,182.51};
    printf("struct:\n");
    printf("name=%s\n",lin.name);
    show_bytes((byte_pointer) &lin.name, strlen(lin.name));

    printf("age=%d\n",lin.age);
    show_bytes((byte_pointer) &lin.age, sizeof(int));

    printf("height=%f\n",lin.height);
    show_bytes((byte_pointer) &lin.height, sizeof(float));

    printf("\n");
    printf("\n");

    printf("array:\n");

    printf("array=123456\n");
    int m[6]= {1,2,3,4};
    show_bytes((byte_pointer) m, sizeof(m));
    printf("\n");
    printf("\n");

    typedef union
    {
        char c;
        int a;
        int b;
    } Demo;

    printf("union:\n");
    Demo d;
    printf("union_1.a=2\n");

    d.b = 2;
    show_bytes((byte_pointer) &d, sizeof(d));
    printf("union_1.c='A'\n");
    d.c = 'A';
    show_bytes((byte_pointer) &d, sizeof(d));
    printf("\n");
    printf("\n");

    enum Season {spring, summer, autumn, winter} s;
    s = spring;
    printf("enum:\n");
    printf("enum_1=spring, summer, autumn, winter\n");
    printf("s=spring\n");
    show_bytes((byte_pointer) &s, sizeof(s));
    s = winter;
    printf("s=winter\n");
    show_bytes((byte_pointer) &s, sizeof(s));
    printf("\n");
    printf("\n");

    printf("main:\n");
    show_bytes((byte_pointer) main, sizeof(main));
    printf("\n");

    printf("printf:\n");
    show_bytes((byte_pointer) printf, sizeof(printf));




}













