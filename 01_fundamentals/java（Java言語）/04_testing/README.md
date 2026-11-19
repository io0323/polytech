# 04. テスト技法 — 少ないテストで多くの不具合を見つける

すべての入力を試すことはできません。テスト技法は「どの入力を選べば効率よく不具合を見つけられるか」を決める方法です。後半では、設計したテストを JUnit で自動化します。

## 1. テストの目的と用語

- テストは「正しく動くことの証明」ではなく、**不具合を見つけるための活動**です。テストで不具合が見つからなくても、不具合がないとは言えません。
- **テストケース**: 入力（前提条件を含む）と、期待する結果の組。期待結果は実行前に決めておく。

### テストレベル（V 字モデル）

```text
要求定義 ───────────────────── 受け入れテスト（利用者の要求を満たすか）
  基本設計 ───────────────── システムテスト（システム全体として動くか）
    詳細設計 ───────────── 結合テスト（部品同士がつながるか）
      実装 ─────────── 単体テスト（メソッド・クラス単位で正しいか）
```

開発の左側の各工程が、右側の同じ高さのテストの基準になります。単体テストで見つけた不具合は、後のテストで見つけるより修正コストがずっと小さくなります。

## 2. ブラックボックステストとホワイトボックステスト

| 種類 | 見るもの | 代表的な技法 |
| --- | --- | --- |
| ブラックボックステスト | 仕様（入力と出力）。中身は見ない | 同値分割、境界値分析、デシジョンテーブル、状態遷移テスト |
| ホワイトボックステスト | プログラムの内部構造 | 命令網羅、分岐網羅、条件網羅 |

以下では次の仕様を例にします。

> 点数（0〜100 の整数）から評価を返す。80 以上は A、60 以上 80 未満は B、60 未満は C。範囲外は例外。

## 3. 同値分割

同じ扱いになる入力をグループ（**同値クラス**）に分け、各グループから代表値を 1 つ選びます。

| 同値クラス | 範囲 | 代表値 | 期待結果 |
| --- | --- | --- | --- |
| 無効（小さすぎ） | 〜 -1 | -10 | 例外 |
| 有効：C | 0 〜 59 | 30 | C |
| 有効：B | 60 〜 79 | 70 | B |
| 有効：A | 80 〜 100 | 90 | A |
| 無効（大きすぎ） | 101 〜 | 150 | 例外 |

**無効な入力のクラス** も忘れずにテストします。

## 4. 境界値分析

不具合は「`>` と `>=` の書き間違い」のように **境界** に集中します。各境界の上下の値をテストします。

```text
  無効  │   C    │   B    │   A    │  無効
 ... -1 │ 0 … 59 │ 60 … 79│ 80 … 100│ 101 ...
       ↑↑      ↑↑       ↑↑        ↑↑
```

テストする値: **-1、0、59、60、79、80、100、101**

同値分割と組み合わせて、少ないケースで漏れを減らします。

## 5. デシジョンテーブル

複数の条件の組み合わせで結果が変わるときに使います。

> 例：アラームを鳴らすのは「温度が上限超え」かつ「監視が有効」のとき。ただし「保守モード」中は鳴らさない。

| 条件・動作 | 1 | 2 | 3 | 4 | 5 |
| --- | --- | --- | --- | --- | --- |
| 温度が上限超え | Y | Y | N | Y | N |
| 監視が有効 | Y | Y | Y | N | N |
| 保守モード | N | Y | N | N | − |
| **アラームを鳴らす** | **○** | − | − | − | − |

条件の組み合わせを表にすると、仕様の抜け（「この場合はどうなる？」）にも気付けます。

## 6. 状態遷移テスト

状態を持つものは、状態と遷移を図や表にして、すべての遷移をテストします。

```text
        start()          error 発生
 IDLE ──────────→ RUNNING ──────────→ ERROR
  ↑                  │                 │
  └──── stop() ──────┘                 │
  └──────────────── reset() ───────────┘
```

| 現在の状態＼イベント | `start()` | `stop()` | エラー発生 | `reset()` |
| --- | --- | --- | --- | --- |
| IDLE | RUNNING | （無効） | （無効） | （無効） |
| RUNNING | （無効） | IDLE | ERROR | （無効） |
| ERROR | （無効） | （無効） | （無効） | IDLE |

「（無効）」の組み合わせで、例外になる・無視されるなど、仕様どおりに振る舞うかもテストします。

## 7. ホワイトボックステストと網羅率（カバレッジ）

```java
if (temperature > limit && enabled) {
    alarm();
}
```

| 基準 | 意味 | この例で必要なケース |
| --- | --- | --- |
| C0：命令網羅 | すべての命令を 1 回は実行する | 条件が真になる 1 ケース |
| C1：分岐網羅 | すべての分岐の真と偽を 1 回は通る | 真のケース ＋ 偽のケース |
| C2：条件網羅 | 個々の条件の真偽をすべて試す | `temperature > limit` と `enabled` のそれぞれが真・偽になるケース |

網羅率 100% でも、仕様の誤りや抜けている処理は見つかりません。ブラックボックステストと組み合わせます。Java では **JaCoCo** で網羅率を測れます。

## 8. JUnit で自動テストを書く

テスト対象:

```java
public class Grader {
    public static String grade(int score) {
        if (score < 0 || score > 100) {
            throw new IllegalArgumentException("score は 0〜100: " + score);
        }
        if (score >= 80) {
            return "A";
        } else if (score >= 60) {
            return "B";
        }
        return "C";
    }
}
```

テストコード（JUnit Jupiter。JUnit 5 以降）:

```java
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;
import org.junit.jupiter.params.provider.ValueSource;

class GraderTest {

    @ParameterizedTest
    @CsvSource({
        "0, C", "59, C",      // C の境界
        "60, B", "79, B",     // B の境界
        "80, A", "100, A"     // A の境界
    })
    void 境界値で正しい評価を返す(int score, String expected) {
        assertEquals(expected, Grader.grade(score));
    }

    @ParameterizedTest
    @ValueSource(ints = {-1, 101})
    void 範囲外は例外(int score) {
        assertThrows(IllegalArgumentException.class, () -> Grader.grade(score));
    }

    @Test
    void 代表値_70はB() {
        assertEquals("B", Grader.grade(70));
    }
}
```

| アノテーション・メソッド | 意味 |
| --- | --- |
| `@Test` | テストメソッド |
| `@ParameterizedTest` + `@CsvSource` | 1 つのテストを複数の入力で繰り返す（境界値テストに便利） |
| `@BeforeEach` / `@AfterEach` | 各テストの前後に実行（準備・後片付け） |
| `@DisplayName` | テスト結果に表示する名前 |
| `assertEquals(期待値, 実際の値)` | 等しいことを確認。**引数の順番に注意** |
| `assertTrue` / `assertThrows` | 真であること／例外が投げられること |

ビルドツール（Gradle / Maven）で JUnit を依存関係に追加し、`./gradlew test` や `mvn test` で全テストを実行します。

### 良い単体テストの条件（FIRST）

- **F**ast：速い（何度でも気軽に実行できる）
- **I**ndependent：他のテストや実行順に依存しない
- **R**epeatable：いつ、どこで実行しても同じ結果になる
- **S**elf-validating：成功／失敗を自動で判定する（目で出力を確認しない）
- **T**imely：コードを書くのと同じ時期に書く

## 9. テストしやすい設計とモック

センサやネットワークのように本物を使うと遅い・不安定なものは、インタフェースで切り離し、テストでは **偽物（モック、スタブ）** を差し込みます（[02 オブジェクト指向](../02_object_oriented/README.md) のインタフェース）。

```java
interface TemperatureSensor {
    double read();
}

class Monitor {
    private final TemperatureSensor sensor;
    Monitor(TemperatureSensor sensor) { this.sensor = sensor; }   // 外から渡す（依存性の注入）
    boolean isOverheated() { return sensor.read() > 60.0; }
}

// テストでは本物のセンサの代わりにラムダで固定値を返す
@Test
void 上限を超えたら過熱と判定する() {
    Monitor monitor = new Monitor(() -> 60.1);
    assertTrue(monitor.isOverheated());
}
```

本格的には **Mockito** というライブラリで、呼ばれた回数の確認などもできます。

## 10. TDD（テスト駆動開発）

```text
Red（失敗するテストを書く）→ Green（最小限のコードで通す）→ Refactor（整理する）→ 繰り返し
```

先にテストを書くことで、仕様を具体的な例として確認しながら開発できます。テストがあるのでリファクタリングも安心して行えます。

## セルフチェック

1. 有効範囲が 1〜99 の入力に対し、境界値分析でテストする値をすべて挙げよ。
2. 同値分割で忘れがちなクラスは何か。
3. C0（命令網羅）と C1（分岐網羅）の違いは何か。
4. `assertEquals` の第 1 引数と第 2 引数には何を書くか。
5. センサを使うクラスを単体テストしやすくするには、どう設計するか。

<details>
<summary>答え</summary>

1. 0、1、99、100。  
2. 無効な入力（範囲外、`null`、不正な形式など）のクラス。  
3. C0 はすべての命令を 1 回以上実行すること、C1 はすべての分岐の真・偽を 1 回以上通ること。C1 を満たせば C0 も満たす。  
4. 第 1 引数に期待値、第 2 引数に実際の値。  
5. センサをインタフェースにして、コンストラクタなどで外から渡せるようにする（テストではモックやスタブを渡す）。
</details>
