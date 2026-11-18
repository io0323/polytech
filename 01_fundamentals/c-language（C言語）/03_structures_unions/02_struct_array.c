#include <stdio.h>

struct Product {
    const char *name;
    int price;
};

int main(void)
{
    struct Product products[] = {
        {"LED", 120},
        {"Switch", 80},
        {"Sensor", 450},
    };
    size_t count = sizeof(products) / sizeof(products[0]);

    for (size_t i = 0; i < count; i++) {
        printf("%s: %d 円\n", products[i].name, products[i].price);
    }

    return 0;
}
