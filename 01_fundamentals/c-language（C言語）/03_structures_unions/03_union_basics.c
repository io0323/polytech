#include <stdio.h>

union SensorValue {
    int integer_value;
    float decimal_value;
};

int main(void)
{
    union SensorValue value;

    value.integer_value = 42;
    printf("整数値: %d\n", value.integer_value);

    value.decimal_value = 23.5f;
    printf("小数値: %.1f\n", value.decimal_value);
    printf("union のサイズ: %zu バイト\n", sizeof(value));

    /* union はメンバーが同じメモリ領域を共有するため、最後に代入した値を使います。 */
    return 0;
}
