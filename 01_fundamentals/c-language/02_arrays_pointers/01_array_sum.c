#include <stdio.h>

int main(void)
{
    int values[] = {3, 6, 9, 12, 15};
    int total = 0;
    size_t count = sizeof(values) / sizeof(values[0]);

    for (size_t i = 0; i < count; i++) {
        total += values[i];
    }

    printf("要素数: %zu\n", count);
    printf("合計: %d\n", total);
    return 0;
}
