#include <stdbool.h>
#include <stdio.h>

int main(void)
{
    int values[] = {4, 8, 15, 16, 23, 42};
    int target = 16;
    size_t count = sizeof(values) / sizeof(values[0]);
    bool found = false;

    for (size_t i = 0; i < count; i++) {
        if (values[i] == target) {
            printf("%d は添字 %zu にあります。\n", target, i);
            found = true;
            break;
        }
    }

    if (!found) {
        printf("%d は見つかりませんでした。\n", target);
    }

    return 0;
}
