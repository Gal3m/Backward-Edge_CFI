#include<stdio.h>
#include<setjmp.h>

jmp_buf ret_adr_buf;

void longjmp_func()
{
    printf("SBU1\n");
    longjmp(ret_adr_buf, 1);
    printf("SBU2\n");
}
  
int main()
{
    if (setjmp(ret_adr_buf))
        printf("SBU3\n");
    else
    {
        printf("SBU4\n");
        longjmp_func();
    }
    return 0;
}