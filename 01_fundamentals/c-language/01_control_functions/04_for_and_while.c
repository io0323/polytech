#include <stdio.h>

int main(void)
{
    int number = 1;

    printf("for ループ: ");
    for (int i = 1; i <= 5; i++) {
        printf("%d ", i);
    }

    printf("\nwhile ループ: ");
    while (number <= 5) {
        printf("%d ", number);
        number++;
    }
    printf("\n");

    return 0;
}
