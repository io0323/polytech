# 02. イベント処理 — ユーザの操作に反応する

ボタンが押された、文字が入力された、画面に触れた、といった出来事を **イベント** と呼びます。Android では、イベントを受け取る **リスナ**（インタフェースを実装したオブジェクト）を部品に登録して処理します。

## 1. イベント駆動の考え方

```text
ユーザの操作 ─→ OS ─→ メインスレッド（UI スレッド）のキュー ─→ 登録したリスナが呼ばれる
```

- プログラムは上から順に流れるのではなく、**イベントが来るのを待って反応する**。
- リスナは **メインスレッド（UI スレッド）** で呼ばれる。ここで時間のかかる処理をすると画面が固まる（後述）。

## 2. クリックイベント

```java
Button buttonRefresh = findViewById(R.id.buttonRefresh);

// ラムダ式で書く（View.OnClickListener は抽象メソッドが 1 つのインタフェース）
buttonRefresh.setOnClickListener(v -> {
    textTemperature.setText("更新中…");
});
```

同じ処理を匿名クラスで書くと次のとおりです（[Java ノート](../../../01_fundamentals/java（Java言語）/02_object_oriented/README.md) のインタフェースとラムダ式）。

```java
buttonRefresh.setOnClickListener(new View.OnClickListener() {
    @Override
    public void onClick(View v) {
        textTemperature.setText("更新中…");
    }
});
```

**長押し** は戻り値で「イベントを消費したか」を返します。`true` を返すと、続くクリックイベントは発生しません。

```java
buttonRefresh.setOnLongClickListener(v -> {
    Toast.makeText(this, "長押し", Toast.LENGTH_SHORT).show();
    return true;    // ここで処理済み
});
```

## 3. よく使うリスナ

| 部品 | リスナ | 呼ばれるとき |
| --- | --- | --- |
| `Button` など | `setOnClickListener` | タップされた |
| `CompoundButton`（`CheckBox`、`Switch`） | `setOnCheckedChangeListener` | オン・オフが変わった |
| `SeekBar` | `setOnSeekBarChangeListener` | つまみが動いた |
| `EditText` | `addTextChangedListener`（`TextWatcher`） | 文字が変わった |
| `EditText` | `setOnEditorActionListener` | キーボードの完了ボタンが押された |
| `RecyclerView` の項目 | アダプタ内で `setOnClickListener` | 一覧の項目がタップされた |

```java
// LED の明るさを SeekBar（0〜100）で変える
seekBrightness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        textBrightness.setText(progress + " %");
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) { }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        sendBrightness(seekBar.getProgress());   // 指を離したときだけ送信する
    }
});
```

BLE 機器に値を送る場合、`onProgressChanged` のたびに送ると通信が詰まります。`onStopTrackingTouch` で確定値だけ送る、一定間隔に間引くなどの工夫をします。

## 4. タッチイベント

指の動きそのものを扱うときは `OnTouchListener` で `MotionEvent` を受け取ります。

```java
joystickArea.setOnTouchListener((v, event) -> {
    switch (event.getActionMasked()) {
        case MotionEvent.ACTION_DOWN:   // 触れた
        case MotionEvent.ACTION_MOVE:   // 動かした
            updatePosition(event.getX(), event.getY());
            return true;
        case MotionEvent.ACTION_UP:     // 離した
            v.performClick();           // アクセシビリティのためクリックも通知する
            resetPosition();
            return true;
        default:
            return false;
    }
});
```

`ACTION_DOWN` で `false` を返すと、そのジェスチャの続き（`MOVE`、`UP`）は届きません。

## 5. メインスレッドをふさがない

UI の描画とイベント処理はすべてメインスレッドで行われます。ここで通信やファイル読み込みのような時間のかかる処理をすると画面が固まり、約 5 秒応答しないと **ANR（Application Not Responding）** ダイアログが出ます。

- 時間のかかる処理は **別スレッド** で行う。
- 逆に、**ビューの操作はメインスレッドからしかできない**（別スレッドから触ると例外）。

```java
private final ExecutorService executor = Executors.newSingleThreadExecutor();

buttonRefresh.setOnClickListener(v -> {
    buttonRefresh.setEnabled(false);
    executor.execute(() -> {
        double value = fetchTemperatureFromServer();   // 時間のかかる処理（別スレッド）
        runOnUiThread(() -> {                          // UI の更新はメインスレッドで
            textTemperature.setText(value + " ℃");
            buttonRefresh.setEnabled(true);
        });
    });
});

@Override
protected void onDestroy() {
    super.onDestroy();
    executor.shutdown();
}
```

[Java ノートのスレッド](../../../01_fundamentals/java（Java言語）/03_threads/README.md) で学んだ `ExecutorService` がそのまま使えます。Kotlin では **コルーチン** で同じことを簡潔に書くのが一般的です。

`Handler(Looper.getMainLooper()).post(...)` でもメインスレッドに処理を渡せます。`postDelayed` を使えば一定時間後の実行もできます。

## 6. 戻るボタンとメニュー

- 戻る操作の上書きには `OnBackPressedDispatcher` を使います（古い `onBackPressed()` の上書きは非推奨）。
- 画面上部のメニューは `onCreateOptionsMenu` で作り、`onOptionsItemSelected` で選ばれた項目を処理します。

```java
getOnBackPressedDispatcher().addCallback(this, new OnBackPressedCallback(true) {
    @Override
    public void handleOnBackPressed() {
        new AlertDialog.Builder(MainActivity.this)
                .setMessage("接続を切断して戻りますか？")
                .setPositiveButton("はい", (d, w) -> finish())
                .setNegativeButton("いいえ", null)
                .show();
    }
});
```

## セルフチェック

1. リスナはどのスレッドで呼ばれるか。そこで時間のかかる処理をすると何が起きるか。
2. 別スレッドで取得した値を `TextView` に表示するにはどうするか。
3. `setOnLongClickListener` の戻り値 `true` は何を意味するか。
4. SeekBar の値を BLE 機器に送るとき、`onProgressChanged` で毎回送らない方がよい理由は何か。
5. タッチイベントで `ACTION_DOWN` に `false` を返すとどうなるか。

<details>
<summary>答え</summary>

1. メインスレッド（UI スレッド）。画面が固まり、長く続くと ANR になる。  
2. `runOnUiThread`（または `Handler` の `post`）でメインスレッドに処理を渡して更新する。  
3. イベントを消費した（処理済み）という意味。続くクリックイベントは発生しない。  
4. 指の動きに合わせて大量の書き込みが発生し、通信が詰まったり遅れたりするため。  
5. そのジェスチャの続き（`ACTION_MOVE`、`ACTION_UP`）が届かなくなる。
</details>
