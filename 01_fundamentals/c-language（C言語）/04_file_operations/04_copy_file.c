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
