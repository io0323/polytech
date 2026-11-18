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
