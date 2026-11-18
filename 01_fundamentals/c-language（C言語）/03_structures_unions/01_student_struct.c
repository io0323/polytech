#include <stdio.h>

struct Student {
    const char *name;
    int age;
    int score;
};

int main(void)
{
    struct Student student = {"Aki", 20, 88};

    printf("名前: %s\n", student.name);
    printf("年齢: %d\n", student.age);
    printf("点数: %d\n", student.score);
    return 0;
}
