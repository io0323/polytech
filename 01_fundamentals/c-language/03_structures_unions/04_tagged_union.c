#include <stdio.h>

enum ValueType {
    VALUE_INTEGER,
    VALUE_DECIMAL
};

union Value {
    int integer_value;
    float decimal_value;
};

struct Measurement {
    enum ValueType type;
    union Value value;
};

void print_measurement(struct Measurement measurement)
{
    if (measurement.type == VALUE_INTEGER) {
        printf("整数の測定値: %d\n", measurement.value.integer_value);
    } else {
        printf("小数の測定値: %.2f\n", measurement.value.decimal_value);
    }
}

int main(void)
{
    struct Measurement count = {VALUE_INTEGER, {.integer_value = 120}};
    struct Measurement voltage = {VALUE_DECIMAL, {.decimal_value = 3.30f}};

    print_measurement(count);
    print_measurement(voltage);
    return 0;
}
