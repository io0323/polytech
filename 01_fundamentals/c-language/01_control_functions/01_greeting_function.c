#include <stdio.h>

void print_greeting(const char *name)
{
    printf("こんにちは、%sさん！\n", name);
}

int main(void)
{
    print_greeting("Polytech");
    return 0;
}
