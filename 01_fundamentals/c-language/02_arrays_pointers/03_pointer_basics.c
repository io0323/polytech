#include <stdio.h>

int main(void)
{
    int temperature = 25;
    int *temperature_ptr = &temperature;

    printf("変数の値: %d\n", temperature);
    printf("変数のアドレス: %p\n", (void *)temperature_ptr);
    printf("ポインタ経由の値: %d\n", *temperature_ptr);

    *temperature_ptr = 28;
    printf("変更後の値: %d\n", temperature);
    return 0;
}
