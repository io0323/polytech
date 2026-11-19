# 02. オブジェクト指向プログラミング — データと処理をまとめて設計する

C の構造体はデータだけをまとめましたが、Java の **クラス** はデータ（フィールド）とそれを扱う処理（メソッド）をまとめます。大きなプログラムを部品に分け、変更に強くするための考え方です。

## 1. クラスとオブジェクト

- **クラス**: 設計図。どんなデータと操作を持つかを定義する。
- **オブジェクト（インスタンス）**: 設計図から `new` で作った実体。

```java
public class Sensor {
    private final String name;   // フィールド（final は再代入不可）
    private double value;

    public Sensor(String name) { // コンストラクタ：new したときに呼ばれる
        this.name = name;
    }

    public void update(double value) {   // メソッド
        this.value = value;
    }

    public double getValue() {
        return value;
    }

    public String getName() {
        return name;
    }
}
```

```java
Sensor temp = new Sensor("温度");
temp.update(23.5);
System.out.println(temp.getName() + ": " + temp.getValue());
```

`static` を付けたフィールド・メソッドは、オブジェクトごとではなくクラスに 1 つだけ存在します（例: `Math.max`）。

## 2. オブジェクト指向の 3 大要素

### 2.1 カプセル化

フィールドを `private` にして外から直接書き換えられないようにし、メソッド経由でだけ操作させます。値のチェックを 1 か所にまとめられます。

```java
public void setThreshold(double threshold) {
    if (threshold < 0) {
        throw new IllegalArgumentException("しきい値は 0 以上");
    }
    this.threshold = threshold;
}
```

| アクセス修飾子 | 見える範囲 |
| --- | --- |
| `public` | どこからでも |
| `protected` | 同じパッケージ ＋ サブクラス |
| （なし） | 同じパッケージ |
| `private` | そのクラスの中だけ |

### 2.2 継承

既存のクラス（スーパークラス）を引き継いで、新しいクラス（サブクラス）を作ります。Java の継承は **1 つのクラスからだけ**（単一継承）です。

```java
public class Device {
    protected final String id;

    public Device(String id) {
        this.id = id;
    }

    public String describe() {
        return "Device " + id;
    }
}

public class Camera extends Device {
    public Camera(String id) {
        super(id);                  // スーパークラスのコンストラクタを呼ぶ
    }

    @Override                       // 上書きしていることをコンパイラに確認させる
    public String describe() {
        return "Camera " + id;
    }
}
```

「A は B の一種である（is-a）」が成り立つときだけ継承を使います。単に機能を借りたいだけなら、フィールドとして持つ **コンポジション（has-a）** の方が変更に強くなります。

### 2.3 ポリモーフィズム（多態性）

同じ型の変数・同じメソッド呼び出しでも、実際のオブジェクトによって異なる処理が動きます。

```java
Device[] devices = { new Device("D1"), new Camera("C1") };
for (Device d : devices) {
    System.out.println(d.describe());   // "Device D1"、"Camera C1"
}
```

呼び出し側は具体的なクラスを知らなくてよいため、新しい機器の種類を追加しても呼び出し側を変えずに済みます。

## 3. 抽象クラスとインタフェース

```java
public interface Measurable {           // 「何ができるか」の約束
    double measure();
}

public abstract class BaseSensor implements Measurable {   // 共通処理を持つ未完成のクラス
    private int readCount;

    public final double read() {
        readCount++;
        return measure();               // 具体的な測定はサブクラスに任せる
    }

    public int getReadCount() {
        return readCount;
    }
}

public class DummyTemperatureSensor extends BaseSensor {
    @Override
    public double measure() {
        return 23.5;
    }
}
```

| 項目 | インタフェース | 抽象クラス |
| --- | --- | --- |
| 目的 | 型が持つ能力の約束 | 共通の実装を持つ土台 |
| 複数持てるか | `implements A, B` と複数実装できる | 継承は 1 つだけ |
| フィールド | 定数のみ | 通常のフィールドを持てる |
| メソッドの実装 | `default` メソッドなら持てる | 持てる |

迷ったらまずインタフェースで設計します。テストで本物のセンサの代わりに偽物（モック）を差し込みやすくなります（[04 テスト技法](../04_testing/README.md)）。

## 4. よく使う標準クラス

### 4.1 コレクション

| インタフェース | 代表的な実装 | 特徴 |
| --- | --- | --- |
| `List` | `ArrayList` | 順序あり、重複可。可変長の配列 |
| `Set` | `HashSet`、`TreeSet` | 重複なし |
| `Map` | `HashMap`、`TreeMap` | キーと値の組 |

```java
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

List<Double> readings = new ArrayList<>();   // <> はジェネリクス（要素の型を指定）
readings.add(18.0);
readings.add(21.5);

Map<String, Double> latest = new HashMap<>();
latest.put("temp", 23.5);
System.out.println(latest.get("temp"));
```

ジェネリクスには基本型を書けないため、`int` は `Integer`、`double` は `Double` などの **ラッパークラス** を使います（自動で変換される）。

### 4.2 `equals` と `hashCode`

自作クラスを `HashMap` のキーや `HashSet` の要素にするなら、`equals` と `hashCode` を両方オーバーライドします。値を入れるだけのクラスなら **record**（Java 16 以降）を使うと、これらが自動で作られます。

```java
public record Reading(String sensorId, double value) { }

Reading r = new Reading("temp-1", 23.5);
System.out.println(r.value());          // 23.5
System.out.println(r);                  // Reading[sensorId=temp-1, value=23.5]
```

### 4.3 ラムダ式と Stream

処理そのものを値として渡せます。コレクションの集計が短く書けます。

```java
double average = readings.stream()
        .mapToDouble(Double::doubleValue)
        .average()
        .orElse(0.0);

readings.removeIf(v -> v < 0);   // 負の値を取り除く
```

### 4.4 列挙型（enum）

```java
public enum Status { IDLE, RUNNING, ERROR }

Status s = Status.RUNNING;
```

C の `enum` と違い、決められた値以外は代入できないため安全です。フィールドやメソッドも持てます。

## 5. 設計の指針

- **単一責任**: 1 つのクラスには 1 つの役割だけを持たせる（測定、保存、送信を分ける）。
- **インタフェースに依存する**: `ArrayList<String> list` より `List<String> list` と書き、実装を差し替えやすくする。
- **イミュータブルを優先する**: 変更しないフィールドは `final` にする。スレッドでの安全性も上がる（[03 スレッド](../03_threads/README.md)）。
- **パッケージで整理する**: `package jp.example.monitoring.sensor;` のように役割ごとに分ける。

## セルフチェック

1. カプセル化の目的と、そのために付けるアクセス修飾子は何か。
2. Java のクラスは同時にいくつのクラスを継承できるか。インタフェースはいくつ実装できるか。
3. `Device d = new Camera("C1"); d.describe();` で呼ばれるのはどちらのクラスのメソッドか。
4. `List<int>` と書けないのはなぜか。代わりにどう書くか。
5. 値を保持するだけのクラスを短く書ける構文は何か。

<details>
<summary>答え</summary>

1. 内部データを外部から直接変更させず、不正な値を防ぐため。フィールドに `private` を付ける。  
2. クラスは 1 つだけ。インタフェースは複数実装できる。  
3. `Camera` の `describe()`（実際のオブジェクトの型で決まる。ポリモーフィズム）。  
4. ジェネリクスには基本型を指定できないため。`List<Integer>` と書く。  
5. `record`。
</details>
