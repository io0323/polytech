#include <stdio.h>

int main(void)
{
    int readings[] = {18, 21, 20, 23};
    int *current = readings;
    size_t count = sizeof(readings) / sizeof(readings[0]);

    for (size_t i = 0; i < count; i++) {
        printf("readings[%zu] = %d\n", i, *(current + i));
    }

    return 0;
}
