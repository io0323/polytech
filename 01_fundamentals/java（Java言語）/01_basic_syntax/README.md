# 01. 基本構文 — C と似ているところ、違うところ

Java の制御文は C とほぼ同じ書き方です。一方で、ポインタがない、文字列がオブジェクト、メモリを自分で解放しない、エラーを例外で扱う、といった違いがあります。違いを中心に押さえます。

## 1. Java プログラムの動き方

```text
Hello.java ──javac──→ Hello.class（バイトコード）──java──→ JVM が実行
```

- C はコンパイルで CPU 向けの機械語を作りますが、Java は **JVM（Java 仮想マシン）** 向けの **バイトコード** を作ります。JVM があれば OS や CPU が違っても同じ `.class` が動きます（Write Once, Run Anywhere）。
- 実行中によく使う部分は JIT コンパイラが機械語に変換するため、十分に速く動きます。
- 使わなくなったオブジェクトは **ガベージコレクション（GC）** が自動で解放します。`free` は不要です。

```java
public class Hello {                          // ファイル名とクラス名を一致させる
    public static void main(String[] args) {  // ここから実行が始まる
        System.out.println("こんにちは、Java");
    }
}
```

## 2. データ型

**基本型（プリミティブ型）** は 8 種類で、サイズは環境によらず決まっています。

| 型 | サイズ | 範囲・用途 |
| --- | --- | --- |
| `byte` | 8 ビット | -128 〜 127。バイナリデータ |
| `short` | 16 ビット | -32768 〜 32767 |
| `int` | 32 ビット | 約 ±21 億。整数の標準 |
| `long` | 64 ビット | 大きな整数。リテラルは `100L` |
| `float` | 32 ビット | 単精度。リテラルは `3.3f` |
| `double` | 64 ビット | 倍精度。小数の標準 |
| `char` | 16 ビット | 1 文字（UTF-16） |
| `boolean` | — | `true` / `false`。C と違い `0`/`1` とは相互変換できない |

C との違い:

- **符号なし整数型がない**（`unsigned int` はない）。センサの生データを扱うときは `Byte.toUnsignedInt(b)` や `b & 0xFF` で変換する。
- `if (x)` のように整数を条件に使えない。`if (x != 0)` と書く。
- ローカル変数は `var` で型推論できる（Java 10 以降）。例: `var count = 0;`

それ以外（`String`、配列、クラス）は **参照型** で、変数にはオブジェクトへの参照が入ります。何も指していない状態が `null` です。

```java
int a = 10;
int b = a;      // 値がコピーされる
b = 20;         // a は 10 のまま

int[] x = {1, 2, 3};
int[] y = x;    // 参照がコピーされる（同じ配列を指す）
y[0] = 99;      // x[0] も 99 になる
```

## 3. 演算子と型変換

演算子は C とほぼ同じです。注意点だけ挙げます。

- 整数同士の割り算は切り捨て: `7 / 2` は `3`。小数が欲しければ `7 / 2.0`。
- 大きい型から小さい型へはキャストが必要: `int i = (int) 3.9;` は `3`。
- `>>>` は符号を無視した右シフト（論理シフト）。`>>` は符号を保つ（算術シフト）。
- `int` の範囲を超えると黙って桁あふれする。`Math.addExact(a, b)` を使うと例外で検出できる。

## 4. 制御文

```java
int score = 72;

if (score >= 80) {
    System.out.println("A");
} else if (score >= 60) {
    System.out.println("B");
} else {
    System.out.println("C");
}

for (int i = 0; i < 5; i++) { /* C と同じ */ }
while (score > 0) { score -= 10; }

int[] values = {3, 6, 9};
for (int v : values) {           // 拡張 for 文：全要素を順に取り出す
    System.out.println(v);
}
```

`switch` は文字列も使え、Java 14 以降は **switch 式** で値を返せます。`break` 忘れによる意図しない処理の続行（フォールスルー）が起きません。

```java
int day = 3;
String name = switch (day) {
    case 1 -> "月曜日";
    case 2 -> "火曜日";
    case 3 -> "水曜日";
    default -> "不明";
};
```

## 5. 配列

```java
int[] readings = new int[4];          // 要素は 0 で初期化される
int[] values = {4, 8, 15, 16};
System.out.println(values.length);   // 4（sizeof の計算は不要）

int[][] grid = new int[3][4];        // 2 次元配列
```

範囲外にアクセスすると、C のように黙って壊れるのではなく `ArrayIndexOutOfBoundsException` が発生します。サイズを後から変えたい場合は `ArrayList`（[02 オブジェクト指向](../02_object_oriented/README.md)）を使います。

## 6. 文字列

`String` はオブジェクトで、**一度作ると変更できない（イミュータブル）** のが特徴です。

```java
String a = "Poly";
String b = a + "tech";            // 新しい文字列が作られる
System.out.println(b.length());   // 8
System.out.println(b.substring(0, 4));  // "Poly"
System.out.println(b.equals("Polytech")); // true
```

- **文字列の比較は `equals`** を使う。`==` は「同じオブジェクトか」を比べるため、内容が同じでも `false` になることがある。
- ループの中で何度も連結するときは `StringBuilder` を使う。
- 書式付き出力は `System.out.printf("%d 件%n", n);` や `String.format(...)`。改行は `%n`。

## 7. メソッド

C の関数にあたるものを **メソッド** と呼び、必ずクラスの中に書きます。

```java
public class Calc {
    static int add(int left, int right) {
        return left + right;
    }

    static double add(double left, double right) {  // オーバーロード
        return left + right;
    }

    public static void main(String[] args) {
        System.out.println(add(12, 7));      // int 版が呼ばれる
        System.out.println(add(1.5, 2.0));   // double 版が呼ばれる
    }
}
```

- 同じ名前で引数の型や数が違うメソッドを定義できる（**オーバーロード**）。
- 引数は常に **値渡し**。ただし参照型は「参照の値」が渡るため、呼び出し先で配列の中身を変えると呼び出し元にも反映される。ポインタを使った `swap` のような関数は書けない。

## 8. 例外処理

C では戻り値でエラーを返していましたが、Java は **例外** を投げて処理を中断し、`catch` で受け取ります。

```java
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public class ReadNotes {
    public static void main(String[] args) {
        try {
            String text = Files.readString(Path.of("practice_notes.txt"));
            System.out.println(text);
        } catch (IOException e) {
            System.err.println("ファイルを読み込めません: " + e.getMessage());
        }
    }
}
```

| 種類 | 例 | 扱い |
| --- | --- | --- |
| チェック例外 | `IOException` | `catch` するか、メソッドに `throws` を書くことがコンパイラに強制される |
| 非チェック例外 | `NullPointerException`、`IllegalArgumentException` | 主にプログラムの誤り。強制はされない |
| エラー | `OutOfMemoryError` | 通常は回復できない |

ファイルやソケットのように閉じる必要があるものは **try-with-resources** を使うと、例外が起きても自動で閉じられます（C の `fclose` 忘れを防げる）。

```java
try (var reader = Files.newBufferedReader(Path.of("data.csv"))) {
    String line;
    while ((line = reader.readLine()) != null) {
        System.out.println(line);
    }
} // ここで reader.close() が自動で呼ばれる
```

## セルフチェック

1. Java のソースはコンパイルすると何になり、何の上で動くか。
2. `int[] y = x;` のあと `y[0] = 99;` とすると `x[0]` はどうなるか。
3. 文字列の内容が等しいかを比べるのに `==` ではなく何を使うか。
4. Java に符号なし整数型はあるか。`byte` の値 `0xF0` を 0〜255 の `int` として扱うにはどうするか。
5. `IOException` を投げるメソッドを呼ぶとき、コンパイラは何を求めるか。

<details>
<summary>答え</summary>

1. バイトコード（`.class`）になり、JVM の上で動く。  
2. 同じ配列を指しているので `x[0]` も `99` になる。  
3. `equals` メソッド。  
4. ない。`Byte.toUnsignedInt(b)` または `b & 0xFF` で変換する。  
5. `try-catch` で捕まえるか、呼び出し元のメソッドに `throws IOException` を書くこと（チェック例外）。
</details>
