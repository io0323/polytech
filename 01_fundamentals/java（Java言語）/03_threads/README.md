# 03. スレッド — 複数の処理を同時に進める

IoT のサーバでは、多数の機器からの受信、データの保存、画面への応答を同時にこなす必要があります。1 つのプログラムの中で処理を並行に進める単位が **スレッド** です。便利な反面、正しく書かないと再現しにくい不具合を生みます。

## 1. プロセスとスレッド

| 項目 | プロセス | スレッド |
| --- | --- | --- |
| 単位 | 実行中のプログラム | プロセスの中の実行の流れ |
| メモリ | プロセスごとに独立 | 同じプロセスのスレッドはヒープを **共有** |
| 作成コスト | 大きい | 小さい |
| 通信 | パイプ、ソケットなど | 共有変数（だから排他制御が必要） |

- **並行（concurrent）**: 複数の処理を切り替えながら進める。1 コアでも可能。
- **並列（parallel）**: 複数のコアで本当に同時に実行する。

## 2. スレッドの作り方

```java
public class HelloThreads {
    public static void main(String[] args) throws InterruptedException {
        Runnable task = () -> {
            String name = Thread.currentThread().getName();
            for (int i = 0; i < 3; i++) {
                System.out.println(name + ": " + i);
            }
        };

        Thread t1 = new Thread(task, "worker-1");
        Thread t2 = new Thread(task, "worker-2");
        t1.start();     // run() ではなく start() で新しいスレッドとして起動する
        t2.start();
        t1.join();      // t1 の終了を待つ
        t2.join();
        System.out.println("すべて終了");
    }
}
```

- 実行するたびに出力の順番が変わることがあります。**スレッドの実行順は保証されません**。
- `run()` を直接呼ぶと、新しいスレッドは作られず、呼び出し元のスレッドで普通に実行されるだけです。
- `Thread` クラスを継承する方法もありますが、`Runnable`（またはラムダ式）を渡す方法が一般的です。

### スレッドの状態

```text
NEW ──start()──→ RUNNABLE ──run() 終了──→ TERMINATED
                   │  ↑
   ロック待ち・sleep・wait │  │ 解除
                   ↓  │
        BLOCKED / WAITING / TIMED_WAITING
```

## 3. 競合状態と排他制御

複数のスレッドが同じ変数を同時に書き換えると、結果が壊れます（**競合状態、レースコンディション**）。

```java
public class Counter {
    private int count = 0;

    public void increment() {
        count++;          // 「読む → 1 足す → 書く」の 3 段階。途中で割り込まれる
    }

    public int get() {
        return count;
    }
}
```

2 つのスレッドで 10 万回ずつ `increment()` しても、結果が 20 万にならないことがあります。対策は次のとおりです。

```java
// 1. synchronized：同時に 1 スレッドだけが実行できるようにする
public synchronized void increment() {
    count++;
}

// 2. アトミック変数：ロックなしで安全に更新する
private final AtomicInteger count = new AtomicInteger();
public void increment() {
    count.incrementAndGet();
}
```

| 手段 | 使いどころ |
| --- | --- |
| `synchronized` | 複数の変数をまとめて一貫性を保ちたいとき |
| `ReentrantLock` | タイムアウト付きのロックなど、細かい制御が必要なとき |
| `AtomicInteger` など | 単一の数値やフラグの更新 |
| `volatile` | 別スレッドからの書き込みを確実に見えるようにする（フラグ用）。`count++` は安全にならない |
| `ConcurrentHashMap` など | スレッドセーフなコレクション |
| イミュータブルなオブジェクト | そもそも変更しないので安全 |

C の組込み開発で、割り込みハンドラと共有する変数に `volatile` を付けたり割り込みを禁止したりしたのと同じ発想です。

## 4. デッドロック

2 つのスレッドがお互いの持つロックを待ち続け、永久に止まる状態です。

```text
スレッド A: ロック X を取得 → ロック Y を待つ
スレッド B: ロック Y を取得 → ロック X を待つ   → どちらも進めない
```

防ぎ方:

- **ロックを取る順番をすべてのスレッドで統一する**（必ず X → Y の順）。
- ロックを持ったまま時間のかかる処理や外部呼び出しをしない。
- `tryLock(タイムアウト)` で待ち時間に上限を設ける。

## 5. スレッドプール（Executor フレームワーク）

`new Thread()` を大量に作るとコストが大きいため、実務では **スレッドプール** を使います。決まった数のスレッドを使い回し、タスクを順に処理させます。

```java
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

public class PoolExample {
    public static void main(String[] args) throws Exception {
        ExecutorService pool = Executors.newFixedThreadPool(4);
        List<Future<Integer>> results = new ArrayList<>();

        for (int i = 1; i <= 8; i++) {
            int sensorId = i;
            results.add(pool.submit(() -> readSensor(sensorId)));  // 戻り値ありのタスク
        }

        for (Future<Integer> f : results) {
            System.out.println(f.get());   // 結果が出るまで待つ
        }

        pool.shutdown();                                 // 新しいタスクの受付を止める
        pool.awaitTermination(10, TimeUnit.SECONDS);     // 終了を待つ
    }

    static int readSensor(int id) throws InterruptedException {
        Thread.sleep(200);                 // 通信待ちの代わり
        return id * 10;
    }
}
```

- `Runnable` は戻り値なし、`Callable` は戻り値と例外を返せる。
- `Future.get()` で結果を受け取る。非同期処理をつなげたいときは `CompletableFuture` を使う。
- `shutdown()` を忘れるとプログラムが終了しないことがある。
- Java 21 以降では、通信待ちの多い処理に軽量な **仮想スレッド**（`Executors.newVirtualThreadPerTaskExecutor()`）が使えます。

## 6. スレッド間の連携

**生産者・消費者パターン**: センサを読むスレッド（生産者）と、データを保存するスレッド（消費者）を **キュー** でつなぎます。

```java
BlockingQueue<Double> queue = new LinkedBlockingQueue<>(100);

// 生産者
queue.put(23.5);          // キューが満杯なら空くまで待つ

// 消費者
double value = queue.take();   // キューが空なら届くまで待つ
```

`wait()` / `notify()` を自分で書くより、`BlockingQueue` や `CountDownLatch` などの部品を使う方が安全です。

## 7. スレッドを止める

スレッドを外から強制停止する `Thread.stop()` は危険なため使いません。**割り込み（interrupt）** で停止を依頼し、スレッド側が自分で終了します。

```java
Thread worker = new Thread(() -> {
    while (!Thread.currentThread().isInterrupted()) {
        try {
            // センサを読む処理
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();   // 割り込み状態を戻してループを抜ける
        }
    }
    System.out.println("後片付けして終了");
});
worker.start();
// ...
worker.interrupt();
```

## セルフチェック

1. スレッドを起動するとき、`run()` ではなく何を呼ぶか。
2. `count++` を複数スレッドから呼ぶと結果が合わない理由は何か。
3. デッドロックを防ぐ代表的な方法を 1 つ挙げよ。
4. 大量のタスクを処理するとき、毎回 `new Thread()` する代わりに何を使うか。
5. `volatile` を付けると `count++` は安全になるか。

<details>
<summary>答え</summary>

1. `start()`。  
2. 読み出し・加算・書き込みの 3 段階に分かれており、途中で別スレッドが割り込むため（アトミックでない）。  
3. すべてのスレッドでロックを取る順番を統一する（ほかに `tryLock` でタイムアウトを設ける）。  
4. スレッドプール（`ExecutorService`）。  
5. ならない。`volatile` は変更が他スレッドから見えることを保証するだけで、`count++` の 3 段階をまとめて実行することは保証しない。`AtomicInteger` か `synchronized` を使う。
</details>
