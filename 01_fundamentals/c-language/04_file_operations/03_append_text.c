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
