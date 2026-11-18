# C 言語サンプル集（転記用・1ファイル）

このファイルだけを開けば、全 18 本のサンプルを番号順に転記できます。各コードブロックは **別々の `.c` ファイル** として保存してコンパイルしてください。

ソースの実体は [01_fundamentals/c-language/](01_fundamentals/c-language/) にあります。

## 01. 制御文・関数

### 01_greeting_function.c

```c
#include <stdio.h>

void print_greeting(const char *name)
{
    printf("こんにちは、%sさん！\n", name);
}

int main(void)
{
    print_greeting("Polytech");
    return 0;
}
```

### 02_if_else_grade.c

```c
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
```

### 03_switch_day.c

```c
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
```

### 04_for_and_while.c

```c
#include <stdio.h>

int main(void)
{
    int number = 1;

    printf("for ループ: ");
    for (int i = 1; i <= 5; i++) {
        printf("%d ", i);
    }

    printf("\nwhile ループ: ");
    while (number <= 5) {
        printf("%d ", number);
        number++;
    }
    printf("\n");

    return 0;
}
```

### 05_functions_return_values.c

```c
#include <stdio.h>

int add(int left, int right)
{
    return left + right;
}

int larger(int left, int right)
{
    if (left > right) {
        return left;
    }
    return right;
}

int main(void)
{
    int a = 12;
    int b = 7;

    printf("%d + %d = %d\n", a, b, add(a, b));
    printf("大きい値 = %d\n", larger(a, b));
    return 0;
}
```

## 02. 配列・ポインタ

### 01_array_sum.c

```c
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
```

### 02_array_search.c

```c
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
```

### 03_pointer_basics.c

```c
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
```

### 04_array_with_pointer.c

```c
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
```

### 05_swap_with_pointers.c

```c
#include <stdio.h>

void swap(int *left, int *right)
{
    int temporary = *left;

    *left = *right;
    *right = temporary;
}

int main(void)
{
    int first = 10;
    int second = 20;

    printf("交換前: first = %d, second = %d\n", first, second);
    swap(&first, &second);
    printf("交換後: first = %d, second = %d\n", first, second);
    return 0;
}
```

## 03. 構造体・共用体

### 01_student_struct.c

```c
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
```

### 02_struct_array.c

```c
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
```

### 03_union_basics.c

```c
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
```

### 04_tagged_union.c

```c
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
```

## 04. ファイル操作

### 01_write_text.c

```c
#include <stdio.h>

int main(int argc, char *argv[])
{
    const char *filename = argc > 1 ? argv[1] : "practice_notes.txt";
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        perror("ファイルを開けません");
        return 1;
    }

    fprintf(file, "C 言語のファイル操作を練習中です。\n");
    fprintf(file, "次は読み込みと追記を試します。\n");

    if (fclose(file) == EOF) {
        perror("ファイルを閉じられません");
        return 1;
    }

    printf("%s に書き込みました。\n", filename);
    return 0;
}
```

### 02_read_text.c

```c
#include <stdio.h>

int main(int argc, char *argv[])
{
    const char *filename = argc > 1 ? argv[1] : "practice_notes.txt";
    FILE *file = fopen(filename, "r");
    int character;

    if (file == NULL) {
        perror("ファイルを開けません");
        return 1;
    }

    while ((character = fgetc(file)) != EOF) {
        putchar(character);
    }

    if (ferror(file)) {
        perror("ファイルを読み込めません");
        fclose(file);
        return 1;
    }

    if (fclose(file) == EOF) {
        perror("ファイルを閉じられません");
        return 1;
    }

    return 0;
}
```

### 03_append_text.c

```c
#include <stdio.h>

int main(int argc, char *argv[])
{
    const char *filename = argc > 1 ? argv[1] : "practice_notes.txt";
    FILE *file = fopen(filename, "a");

    if (file == NULL) {
        perror("ファイルを開けません");
        return 1;
    }

    fprintf(file, "この行は追記モードで追加されました。\n");

    if (fclose(file) == EOF) {
        perror("ファイルを閉じられません");
        return 1;
    }

    printf("%s に追記しました。\n", filename);
    return 0;
}
```

### 04_copy_file.c

```c
#include <stdio.h>

int main(int argc, char *argv[])
{
    FILE *source;
    FILE *destination;
    int character;

    if (argc != 3) {
        fprintf(stderr, "使い方: %s <コピー元> <コピー先>\n", argv[0]);
        return 1;
    }

    source = fopen(argv[1], "rb");
    if (source == NULL) {
        perror("コピー元を開けません");
        return 1;
    }

    destination = fopen(argv[2], "wb");
    if (destination == NULL) {
        perror("コピー先を開けません");
        fclose(source);
        return 1;
    }

    while ((character = fgetc(source)) != EOF) {
        if (fputc(character, destination) == EOF) {
            perror("コピー先に書き込めません");
            fclose(source);
            fclose(destination);
            return 1;
        }
    }

    if (ferror(source)) {
        perror("コピー元を読み込めません");
        fclose(source);
        fclose(destination);
        return 1;
    }

    if (fclose(source) == EOF || fclose(destination) == EOF) {
        perror("ファイルを閉じられません");
        return 1;
    }

    printf("%s を %s にコピーしました。\n", argv[1], argv[2]);
    return 0;
}
```

## コンパイルの書式

各サンプルは、たとえば次の書式でコンパイルできます。

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic ファイル名.c -o 実行ファイル名
```
