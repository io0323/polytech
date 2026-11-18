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
