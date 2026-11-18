#include <stdio.h>

int main(void)
{
    int score = 72;

    if (score >= 80) {
        printf("評価: A\n");
    } else if (score >= 60) {
        printf("評価: B\n");
    } else {
        printf("評価: C\n");
    }

    return 0;
}
