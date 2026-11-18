#include <stdio.h>

int add(int left, int right)
{
    return left + right;
}

int larger(int left, int right)
{
    if (left > right) {
        return left;
    }
    return right;
}

int main(void)
{
    int a = 12;
    int b = 7;

    printf("%d + %d = %d\n", a, b, add(a, b));
    printf("大きい値 = %d\n", larger(a, b));
    return 0;
}
