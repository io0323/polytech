# polytech

ポリテク「デバイスソフトエンジニア科」の入所前学習用リポジトリです。まずは C 言語の基本を、1 ファイルずつコンパイルして動きを確認できる小さなプログラムで学びます。

## 構成

```text
polytech/
├── docs/                           # 要件定義・アーキテクチャ・回路設計・開発ログ
├── 01_fundamentals/
│   ├── c-language/
│   │   ├── 01_control_functions/   # 制御文と関数
│   │   ├── 02_arrays_pointers/     # 配列とポインタ
│   │   ├── 03_structures_unions/   # 構造体と共用体
│   │   └── 04_file_operations/     # ファイル操作
│   ├── digital-circuit/
│   └── assembler/
├── 02_microcontroller/             # GPIO / センサー / PWM / ADC / 割り込み
├── 03_embedded-linux/              # Linux 基礎 / Raspberry Pi / 組込みプログラミング
├── 04_application/                 # モニタリング
└── tests/
```

各フォルダ内の番号順に進めてください。サンプルはすべて単独でコンパイルできます。まず実行し、次に変数や条件を少し変えて結果を比べる学び方がおすすめです。

個別ファイルを開かずに全ソースをまとめて転記したい場合は、[ALL_SAMPLES.md](ALL_SAMPLES.md) を使ってください。

## 学習順とサンプル一覧

| 分野 | サンプル | 学ぶこと |
| --- | --- | --- |
| 制御文・関数 | `01_greeting_function` → `05_functions_return_values` | 関数の呼び出し、`if`、`switch`、繰り返し、戻り値 |
| 配列・ポインタ | `01_array_sum` → `05_swap_with_pointers` | 配列の走査と検索、アドレス、配列とポインタの関係、ポインタ引数 |
| 構造体・共用体 | `01_student_struct` → `04_tagged_union` | 構造体、構造体の配列、共用体のメモリ共有、安全に種別を持つ共用体 |
| ファイル操作 | `01_write_text` → `04_copy_file` | テキストの書き込み・読み込み・追記、引数、バイナリモードでのコピー |

## 必要なもの

- C コンパイラ（macOS なら Command Line Tools の `clang`、Linux なら `gcc`）
- `make`（任意。まとめてビルドするときだけ使用）

macOS でコンパイラが未導入なら、ターミナルで次を実行します。

```sh
xcode-select --install
```

Ubuntu / Debian 系では次のとおりです。

```sh
sudo apt install build-essential
```

## 1 本ずつビルド・実行する

リポジトリのルート（この `README.md` がある場所）で実行します。

```sh
mkdir -p build
gcc -std=c11 -Wall -Wextra -Wpedantic \
  01_fundamentals/c-language/01_control_functions/01_greeting_function.c \
  -o build/greeting
./build/greeting
```

macOS では `gcc` の代わりに `clang` でも構いません。

## まとめてビルドする

```sh
make
```

実行ファイルは `build/` に作られます。たとえば次のように起動できます。

```sh
./build/01_control_functions_04_for_and_while
./build/02_arrays_pointers_05_swap_with_pointers
```

ファイル操作のサンプルは、引数で対象ファイルを指定できます。

```sh
./build/04_file_operations_01_write_text practice_notes.txt
./build/04_file_operations_02_read_text practice_notes.txt
./build/04_file_operations_03_append_text practice_notes.txt
./build/04_file_operations_04_copy_file practice_notes.txt practice_notes_copy.txt
```

生成物を消すには `make clean` を使います。
