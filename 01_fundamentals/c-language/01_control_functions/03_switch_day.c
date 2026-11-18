#include <stdio.h>

int main(void)
{
    int day = 3;

    switch (day) {
    case 1:
        printf("月曜日\n");
        break;
    case 2:
        printf("火曜日\n");
        break;
    case 3:
        printf("水曜日\n");
        break;
    default:
        printf("1 から 3 の数字を指定してください。\n");
        break;
    }

    return 0;
}
