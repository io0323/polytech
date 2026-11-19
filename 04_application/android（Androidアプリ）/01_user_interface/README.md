# 01. ユーザインタフェース — 画面の部品と配置

Android アプリの画面は、**Activity**（1 つの画面を担当するクラス）と、**レイアウト**（部品の配置を書いた XML）の組で作ります。まずプロジェクトの構成と、画面がいつ作られて・いつ消えるかを押さえます。

## 1. Android の仕組みとプロジェクト構成

```text
アプリ（Java / Kotlin）
Android フレームワーク（Activity、View、Bluetooth などの API）
Android ランタイム（ART）・ネイティブライブラリ
Linux カーネル（ドライバ、プロセス管理）
```

Android は Linux カーネルの上で動きます（[Linux・ネットワークノート](../../../03_embedded-linux/linux-network（Linux・ネットワーク）/README.md) 参照）。アプリは 1 つずつ別のユーザ・別のプロセスとして隔離されて動きます。

```text
app/
├── src/main/
│   ├── AndroidManifest.xml      # アプリの宣言（画面一覧、パーミッション、起動画面）
│   ├── java/com/example/app/    # Java / Kotlin のソース
│   │   └── MainActivity.java
│   └── res/                     # リソース
│       ├── layout/activity_main.xml   # 画面のレイアウト
│       ├── values/strings.xml         # 文字列
│       ├── values/colors.xml          # 色
│       ├── values/themes.xml          # テーマ
│       └── drawable/, mipmap/         # 画像・アイコン
└── build.gradle(.kts)           # ビルド設定（minSdk、targetSdk、依存ライブラリ）
```

| 設定 | 意味 |
| --- | --- |
| `minSdk` | 動作する最も古い Android の API レベル |
| `targetSdk` | 動作を想定している API レベル。新しいほど新しい制約（パーミッションなど）が適用される |
| `compileSdk` | ビルドに使う SDK の API レベル |

API レベルは Android のバージョン番号に対応します（例: Android 12 = API 31、Android 13 = API 33、Android 14 = API 34）。

## 2. Activity とライフサイクル

```java
public class MainActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);   // res/layout/activity_main.xml を表示
    }
}
```

`R` はリソースから自動生成されるクラスで、`R.layout.activity_main`、`R.id.button`、`R.string.app_name` のように ID でリソースを参照します。

Activity は状態に応じて OS からメソッドが呼ばれます。

```text
          onCreate() ─→ onStart() ─→ onResume() ─→ 【操作できる状態】
                                                       │ 別の画面が前に来る
                         onRestart()                onPause()
                            ↑                          │ 完全に見えなくなる
                            └──── 戻ってくる ──────── onStop()
                                                       │ 終了・破棄
                                                    onDestroy()
```

| メソッド | 主にやること |
| --- | --- |
| `onCreate` | レイアウトの設定、ビューの取得、初期化（1 回だけ） |
| `onStart` / `onStop` | 画面が見える間だけ必要な処理の開始・停止 |
| `onResume` / `onPause` | センサ・カメラ・BLE スキャンなどの開始・停止 |
| `onDestroy` | 後片付け（接続を閉じるなど） |

**画面の回転** や言語設定の変更で Activity は **作り直されます**（`onDestroy` → `onCreate`）。フィールドに持っていた値は消えるため、保持したいデータは `ViewModel` に置きます。

## 3. View と ViewGroup

画面は **View**（部品）と、それを並べる **ViewGroup**（レイアウト）の木構造です。

| 部品 | 用途 |
| --- | --- |
| `TextView` | 文字の表示 |
| `EditText` | 文字の入力 |
| `Button` | ボタン |
| `ImageView` | 画像 |
| `CheckBox` / `Switch` | オン・オフ |
| `SeekBar` | スライダー（明るさ・速度の調整など） |
| `ProgressBar` | 処理中の表示 |
| `RecyclerView` | 大量の項目を効率よくスクロール表示する一覧（BLE の検出機器一覧など） |

| レイアウト | 特徴 |
| --- | --- |
| `LinearLayout` | 縦または横に一列に並べる。単純な画面向け |
| `ConstraintLayout` | 部品同士や親との位置関係（制約）で配置する。現在の標準 |
| `FrameLayout` | 重ねて配置する |

## 4. レイアウト XML

```xml
<?xml version="1.0" encoding="utf-8"?>
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    android:orientation="vertical"
    android:padding="16dp">

    <TextView
        android:id="@+id/textTemperature"
        android:layout_width="wrap_content"
        android:layout_height="wrap_content"
        android:text="@string/temperature_placeholder"
        android:textSize="32sp" />

    <Button
        android:id="@+id/buttonRefresh"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:layout_marginTop="16dp"
        android:text="@string/refresh" />
</LinearLayout>
```

| 指定 | 意味 |
| --- | --- |
| `match_parent` | 親いっぱいに広げる |
| `wrap_content` | 中身に合わせた大きさ |
| `dp` | 密度非依存ピクセル。画面の細かさが違っても同じ見た目の大きさになる。**大きさや余白に使う** |
| `sp` | `dp` に加え、ユーザの文字サイズ設定を反映する。**文字サイズに使う** |
| `@+id/名前` | 新しい ID を定義する |
| `@string/名前` | `strings.xml` の文字列を参照する |

文字列を XML やコードに直接書かず `strings.xml` にまとめると、多言語対応（`values-en/strings.xml` など）が容易になります。

## 5. コードからビューを操作する

```java
TextView textTemperature = findViewById(R.id.textTemperature);
textTemperature.setText(getString(R.string.temperature_format, 23.5));
```

```xml
<!-- res/values/strings.xml -->
<string name="temperature_format">%.1f ℃</string>
```

`findViewById` の代わりに **ViewBinding** を有効にすると、型安全にビューを取得できます。

```java
// build.gradle で android { buildFeatures { viewBinding true } } を有効にする
private ActivityMainBinding binding;

@Override
protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    binding = ActivityMainBinding.inflate(getLayoutInflater());
    setContentView(binding.getRoot());
    binding.textTemperature.setText("23.5 ℃");
}
```

## 6. ユーザへの通知

| 部品 | 使いどころ |
| --- | --- |
| `Toast` | 短いメッセージを一時的に表示（「保存しました」） |
| `Snackbar` | 画面下部のメッセージ。「元に戻す」などのボタンを付けられる |
| `AlertDialog` | 確認や選択を求める（「接続を切断しますか？」） |

```java
Toast.makeText(this, "接続しました", Toast.LENGTH_SHORT).show();
```

## 7. Jetpack Compose（補足）

新しい UI の作り方で、XML を使わず Kotlin の関数で画面を記述します。状態（データ）が変わると画面が自動で描き直されます。

```kotlin
@Composable
fun TemperatureScreen(temperature: Double, onRefresh: () -> Unit) {
    Column(modifier = Modifier.padding(16.dp)) {
        Text(text = "%.1f ℃".format(temperature), fontSize = 32.sp)
        Button(onClick = onRefresh) { Text("更新") }
    }
}
```

## セルフチェック

1. 画面の部品や配置はどのディレクトリのどの形式のファイルに書くか。
2. 画面を回転させると Activity はどうなるか。データを保持するにはどうするか。
3. 文字サイズに使う単位と、余白に使う単位はそれぞれ何か。
4. BLE スキャンのように「画面が前面にある間だけ」動かしたい処理は、どのライフサイクルメソッドで開始・停止するか。
5. 文字列を `strings.xml` にまとめる利点は何か。

<details>
<summary>答え</summary>

1. `res/layout/` の XML ファイル。  
2. 破棄されて作り直される（`onDestroy` → `onCreate`）。`ViewModel` にデータを置く。  
3. 文字サイズは `sp`、余白や大きさは `dp`。  
4. `onResume` で開始し、`onPause` で停止する。  
5. 多言語対応や文言の修正が 1 か所でできる。
</details>
