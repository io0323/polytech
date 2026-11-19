# 03. インテント — 画面や他のアプリを呼び出す

**インテント（Intent）** は「何をしたいか」を OS に伝えるメッセージです。自分のアプリの別の画面を開く、データを渡す、地図やブラウザなど他のアプリに処理を頼む、といった場面で使います。

## 1. インテントの 2 種類

| 種類 | 指定するもの | 使いどころ | 例 |
| --- | --- | --- | --- |
| 明示的インテント | 呼び出すクラス（コンポーネント）そのもの | 自分のアプリ内の画面遷移、サービスの起動 | 機器一覧画面 → 機器詳細画面 |
| 暗黙的インテント | やりたいこと（アクション）とデータ | 他のアプリに処理を任せる | Web ページを開く、共有する、電話をかける |

## 2. 明示的インテントで画面遷移する

```java
// MainActivity から DeviceActivity を開く
Intent intent = new Intent(this, DeviceActivity.class);
startActivity(intent);
```

呼び出す Activity は必ず `AndroidManifest.xml` に宣言します。宣言がないと実行時にエラーになります。

```xml
<application ...>
    <activity
        android:name=".MainActivity"
        android:exported="true">
        <intent-filter>
            <action android:name="android.intent.action.MAIN" />
            <category android:name="android.intent.category.LAUNCHER" />
        </intent-filter>
    </activity>
    <activity android:name=".DeviceActivity" android:exported="false" />
</application>
```

- `MAIN` + `LAUNCHER` の `intent-filter` を持つ Activity が、ホーム画面のアイコンから起動される画面になります。
- `android:exported` は他のアプリから起動できるかどうか。`intent-filter` を持つ Activity では明示が必須（Android 12 以降）。

Activity は **バックスタック** に積まれ、戻るボタンで前の画面に戻ります。現在の画面を閉じるには `finish()` を呼びます。

## 3. データを渡す（エクストラ）

```java
// 送る側
Intent intent = new Intent(this, DeviceActivity.class);
intent.putExtra(DeviceActivity.EXTRA_ADDRESS, "AA:BB:CC:DD:EE:FF");
intent.putExtra(DeviceActivity.EXTRA_NAME, "Sensor-01");
startActivity(intent);
```

```java
// 受け取る側
public class DeviceActivity extends AppCompatActivity {
    public static final String EXTRA_ADDRESS = "com.example.app.EXTRA_ADDRESS";
    public static final String EXTRA_NAME = "com.example.app.EXTRA_NAME";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device);

        String address = getIntent().getStringExtra(EXTRA_ADDRESS);
        if (address == null) {
            finish();     // 必要なデータがなければ閉じる
            return;
        }
        // address を使って接続する
    }
}
```

- キーは定数にして、送る側と受け取る側の書き間違いを防ぎます。
- 渡せるのは基本型・文字列・配列・`Parcelable` などです。大きなデータ（画像など）は直接渡さず、ファイルや ID を渡します。

## 4. 結果を受け取る（Activity Result API）

別の画面で選んだ結果を受け取るときは `registerForActivityResult` を使います（古い `startActivityForResult` は非推奨）。

```java
private final ActivityResultLauncher<Intent> selectDevice =
        registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), result -> {
            if (result.getResultCode() == RESULT_OK && result.getData() != null) {
                String address = result.getData().getStringExtra(DeviceActivity.EXTRA_ADDRESS);
                textSelected.setText(address);
            }
        });

// 起動
selectDevice.launch(new Intent(this, ScanActivity.class));
```

```java
// ScanActivity 側で結果を返して閉じる
Intent data = new Intent();
data.putExtra(DeviceActivity.EXTRA_ADDRESS, selectedAddress);
setResult(RESULT_OK, data);
finish();
```

## 5. 暗黙的インテントで他のアプリを使う

```java
// Web ページを開く
startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("https://example.com/manual")));

// 電話アプリにダイヤル番号を入れた状態で開く（発信はユーザが行う）
startActivity(new Intent(Intent.ACTION_DIAL, Uri.parse("tel:0120000000")));

// 測定結果を他のアプリに共有する
Intent send = new Intent(Intent.ACTION_SEND);
send.setType("text/plain");
send.putExtra(Intent.EXTRA_TEXT, "温度: 23.5 ℃");
startActivity(Intent.createChooser(send, "共有先を選択"));
```

| アクション | 意味 |
| --- | --- |
| `ACTION_VIEW` | データを表示する（URL、地図、ファイル） |
| `ACTION_SEND` | データを送る・共有する |
| `ACTION_DIAL` | ダイヤル画面を開く |
| `MediaStore.ACTION_IMAGE_CAPTURE` | カメラで撮影する |

対応するアプリがない場合は `ActivityNotFoundException` が発生するため、`try-catch` で囲んでメッセージを出します。

OS 側は、各アプリの `intent-filter`（アクション・データの種類・カテゴリ）と照らし合わせて起動先を決めます。

## 6. 設定画面を開く・システムの機能を頼む

BLE アプリでは、Bluetooth がオフのときにユーザにオンを頼む場面があります。これもインテントで行います。

```java
private final ActivityResultLauncher<Intent> enableBluetooth =
        registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), result -> {
            if (result.getResultCode() == RESULT_OK) {
                startScan();
            }
        });

// Bluetooth がオフなら有効化を依頼する（Android 12 以降は BLUETOOTH_CONNECT 権限が必要）
if (!bluetoothAdapter.isEnabled()) {
    enableBluetooth.launch(new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE));
}
```

## 7. パーミッション（権限）の要求

カメラ・位置情報・Bluetooth など、ユーザのプライバシーに関わる機能は **実行時パーミッション** が必要です。マニフェストへの宣言に加え、実行時にユーザの許可を得ます。

```xml
<uses-permission android:name="android.permission.CAMERA" />
```

```java
private final ActivityResultLauncher<String> requestCamera =
        registerForActivityResult(new ActivityResultContracts.RequestPermission(), granted -> {
            if (granted) {
                openCamera();
            } else {
                Toast.makeText(this, "カメラの権限がないため撮影できません", Toast.LENGTH_LONG).show();
            }
        });

if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
        == PackageManager.PERMISSION_GRANTED) {
    openCamera();
} else {
    requestCamera.launch(Manifest.permission.CAMERA);
}
```

複数の権限をまとめて求めるときは `ActivityResultContracts.RequestMultiplePermissions` を使います（[04 BLE](../04_ble_device_control/README.md) で使用）。

## 8. インテントの他の使い道（参考）

| コンポーネント | 役割 | インテントとの関係 |
| --- | --- | --- |
| Activity | 画面 | `startActivity` |
| Service | 画面なしで動く処理（長時間の通信、音楽再生） | `startService` / `bindService` |
| BroadcastReceiver | システムや他アプリからの通知を受け取る（電池残量低下、Bluetooth のオン・オフ） | `sendBroadcast` / `intent-filter` |
| ContentProvider | アプリ間でデータを共有する | URI で指定 |

この 4 つを **Android の 4 大コンポーネント** と呼びます。

## セルフチェック

1. 自分のアプリの別の画面を開くのは、明示的・暗黙的どちらのインテントか。
2. 新しく作った Activity を開こうとしたらエラーになった。まず何を確認するか。
3. 次の画面に機器のアドレスを渡す方法と、受け取る方法を答えよ。
4. 別の画面の結果を受け取るために、現在推奨される API は何か。
5. Android の 4 大コンポーネントを挙げよ。

<details>
<summary>答え</summary>

1. 明示的インテント。  
2. `AndroidManifest.xml` に `<activity>` が宣言されているか。  
3. `intent.putExtra(キー, 値)` で渡し、受け取る側で `getIntent().getStringExtra(キー)` で取り出す。  
4. Activity Result API（`registerForActivityResult`）。  
5. Activity、Service、BroadcastReceiver、ContentProvider。
</details>
