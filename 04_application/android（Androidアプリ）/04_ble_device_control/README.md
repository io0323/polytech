# 04. BLE アプリによるデバイス制御 — スマホから機器を動かす

**BLE（Bluetooth Low Energy）** は、ボタン電池でも長く動くことを目的にした省電力の近距離無線です。センサ、ウェアラブル、スマートロックなど多くの IoT 機器が BLE でスマートフォンとつながります。ここでは BLE の仕組みと、Android からスキャン・接続・読み書き・通知を行う手順を押さえます。

## 1. Bluetooth Classic と BLE

| 項目 | Bluetooth Classic | BLE |
| --- | --- | --- |
| 主な用途 | イヤホン、スピーカーなど連続した大量データ | センサ値・状態・コマンドなど小さなデータ |
| 消費電力 | 大きい | 非常に小さい（普段は寝て、必要なときだけ通信） |
| 通信のモデル | ストリーム（シリアル通信のように流す） | 属性（GATT）を読み書きする |

## 2. 役割と通信の流れ

| 役割 | 意味 | 例 |
| --- | --- | --- |
| ペリフェラル（Peripheral） | アドバタイズ（存在の告知）を出し、接続を待つ | センサ機器、ESP32、micro:bit |
| セントラル（Central） | スキャンして見つけ、接続しにいく | スマートフォン |
| GATT サーバ | データ（属性）を持つ側 | 多くの場合ペリフェラル |
| GATT クライアント | データを読み書きする側 | 多くの場合スマートフォン |

```text
機器（ペリフェラル）               スマホ（セントラル）
  アドバタイズ ─ ─ ─ ─ ─ ─ ─ ─ →  スキャンして発見
                  ←──────────── 接続要求
  接続確立 ───────────────────→
                  ←──────────── サービス探索（どんなデータを持っているか）
  サービス一覧 ─────────────────→
                  ←──────────── Read / Write
  値の変化を通知（Notify）──────→   画面を更新
```

## 3. GATT のデータ構造

```text
GATT サーバ（機器）
└── サービス（Service）            例: 環境センササービス
    ├── キャラクタリスティック       例: 温度（Read / Notify）
    │   ├── 値（Value）              例: 0x0929 → 23.45 ℃
    │   └── ディスクリプタ CCCD      通知のオン・オフを書き込む場所（UUID 0x2902）
    └── キャラクタリスティック       例: LED 制御（Write）
```

| 用語 | 意味 |
| --- | --- |
| サービス | 関連するデータのまとまり |
| キャラクタリスティック | 1 つのデータ項目。プロパティ（Read / Write / Notify / Indicate）を持つ |
| ディスクリプタ | キャラクタリスティックの補足情報。通知の有効化に使う **CCCD** が重要 |
| UUID | それぞれを識別する ID。Bluetooth SIG が決めた 16 ビットの標準 UUID（例: 心拍 `0x180D`）と、独自の 128 ビット UUID がある |

| プロパティ | 意味 |
| --- | --- |
| Read | スマホから値を読みにいく |
| Write | スマホから値を書き込む（応答あり） |
| Write Without Response | 応答なしで書き込む（速いが届いた確認がない） |
| Notify | 機器から値の変化を送ってくる（確認応答なし） |
| Indicate | 機器から送ってくる（確認応答あり） |

機器側の UUID とデータ形式（バイト順、単位、スケール）は、機器の仕様書やファームウェアのソースで確認します。値は **バイト列** で届くため、[C 言語](../../../01_fundamentals/c-language（C言語）/) や [Java](../../../01_fundamentals/java（Java言語）/01_basic_syntax/README.md) で学んだバイト操作（リトルエンディアン、符号なし変換）が必要になります。

## 4. パーミッション

Android のバージョンによって必要な権限が異なります。

```xml
<!-- Android 12（API 31）以降 -->
<uses-permission android:name="android.permission.BLUETOOTH_SCAN"
    android:usesPermissionFlags="neverForLocation" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />

<!-- Android 11 以前 -->
<uses-permission android:name="android.permission.BLUETOOTH" android:maxSdkVersion="30" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" android:maxSdkVersion="30" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" android:maxSdkVersion="30" />

<!-- BLE に対応した端末だけにインストールさせる -->
<uses-feature android:name="android.hardware.bluetooth_le" android:required="true" />
```

| Android のバージョン | 実行時に求める権限 |
| --- | --- |
| 12 以降 | `BLUETOOTH_SCAN`、`BLUETOOTH_CONNECT` |
| 11 以前 | `ACCESS_FINE_LOCATION`（BLE スキャンで位置が推測できるため）。端末の位置情報設定もオンが必要 |

```java
private final ActivityResultLauncher<String[]> requestBlePermissions =
        registerForActivityResult(new ActivityResultContracts.RequestMultiplePermissions(), result -> {
            if (!result.containsValue(false)) {
                startScan();
            } else {
                Toast.makeText(this, "Bluetooth の権限が必要です", Toast.LENGTH_LONG).show();
            }
        });

private void checkPermissionsAndScan() {
    String[] permissions = Build.VERSION.SDK_INT >= Build.VERSION_CODES.S
            ? new String[] {Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT}
            : new String[] {Manifest.permission.ACCESS_FINE_LOCATION};
    requestBlePermissions.launch(permissions);
}
```

Bluetooth がオフの場合の有効化依頼は [03 インテント](../03_intents/README.md) を参照してください。

## 5. スキャン

```java
private static final UUID SERVICE_UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef0");

private BluetoothLeScanner scanner;
private final Handler handler = new Handler(Looper.getMainLooper());

private final ScanCallback scanCallback = new ScanCallback() {
    @Override
    public void onScanResult(int callbackType, ScanResult result) {
        BluetoothDevice device = result.getDevice();
        int rssi = result.getRssi();                 // 電波の強さ（dBm、0 に近いほど強い）
        addToList(device, rssi);                     // 一覧に追加（重複はアドレスで除く）
    }

    @Override
    public void onScanFailed(int errorCode) {
        Log.e(TAG, "スキャン失敗: " + errorCode);
    }
};

@SuppressLint("MissingPermission")   // 権限は checkPermissionsAndScan で確認済み
private void startScan() {
    BluetoothManager manager = getSystemService(BluetoothManager.class);
    scanner = manager.getAdapter().getBluetoothLeScanner();

    List<ScanFilter> filters = List.of(
            new ScanFilter.Builder().setServiceUuid(new ParcelUuid(SERVICE_UUID)).build());
    ScanSettings settings = new ScanSettings.Builder()
            .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
            .build();

    scanner.startScan(filters, settings, scanCallback);
    handler.postDelayed(this::stopScan, 10_000);   // 10 秒で止める（電池の消耗を防ぐ）
}

@SuppressLint("MissingPermission")
private void stopScan() {
    if (scanner != null) {
        scanner.stopScan(scanCallback);
    }
}
```

- スキャンは電池を多く使うため、**時間を区切る** か、画面が前面の間だけ（`onResume` / `onPause`）にします。
- 短時間にスキャンの開始・停止を繰り返すと、OS がスキャンを制限することがあります。
- `ScanFilter` でサービス UUID や機器名を絞り込むと、目的の機器だけが見つかります。

## 6. 接続とサービス探索

```java
private BluetoothGatt gatt;

@SuppressLint("MissingPermission")
private void connect(BluetoothDevice device) {
    stopScan();   // 接続前にスキャンを止める
    gatt = device.connectGatt(this, false, gattCallback, BluetoothDevice.TRANSPORT_LE);
}

private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
    @SuppressLint("MissingPermission")
    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
        if (status == BluetoothGatt.GATT_SUCCESS && newState == BluetoothProfile.STATE_CONNECTED) {
            gatt.discoverServices();                       // 接続できたらサービス探索
        } else {
            gatt.close();                                  // 切断・失敗時は必ず解放
            runOnUiThread(() -> showStatus("切断されました"));
        }
    }

    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status) {
        if (status == BluetoothGatt.GATT_SUCCESS) {
            BluetoothGattService service = gatt.getService(SERVICE_UUID);
            // service.getCharacteristic(UUID) で目的のキャラクタリスティックを取得
            runOnUiThread(() -> showStatus("接続しました"));
        }
    }
};
```

**重要: `BluetoothGattCallback` はメインスレッド以外で呼ばれます。** 画面の更新は `runOnUiThread` で行います（[02 イベント処理](../02_event_handling/README.md)）。

## 7. 読み書きと通知

```java
private static final UUID LED_UUID  = UUID.fromString("12345678-1234-5678-1234-56789abcdef1");
private static final UUID TEMP_UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef2");
private static final UUID CCCD_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

// LED を点灯（1）・消灯（0）させる
@SuppressLint("MissingPermission")
private void writeLed(boolean on) {
    BluetoothGattCharacteristic led = gatt.getService(SERVICE_UUID).getCharacteristic(LED_UUID);
    byte[] value = {(byte) (on ? 1 : 0)};

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {        // Android 13 以降
        gatt.writeCharacteristic(led, value, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
    } else {
        led.setValue(value);
        led.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
        gatt.writeCharacteristic(led);
    }
}

// 温度の通知を有効にする（2 段階）
@SuppressLint("MissingPermission")
private void enableTemperatureNotify() {
    BluetoothGattCharacteristic temp = gatt.getService(SERVICE_UUID).getCharacteristic(TEMP_UUID);
    gatt.setCharacteristicNotification(temp, true);               // 1. スマホ側で受け付ける

    BluetoothGattDescriptor cccd = temp.getDescriptor(CCCD_UUID); // 2. 機器側に送信を依頼する
    byte[] enable = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE;
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
        gatt.writeDescriptor(cccd, enable);
    } else {
        cccd.setValue(enable);
        gatt.writeDescriptor(cccd);
    }
}
```

通知の受信（`gattCallback` に追加）:

```java
// Android 13 以降
@Override
public void onCharacteristicChanged(BluetoothGatt gatt,
        BluetoothGattCharacteristic characteristic, byte[] value) {
    handleValue(characteristic.getUuid(), value);
}

// Android 12 以前（非推奨だが古い端末ではこちらが呼ばれる）
@Override
public void onCharacteristicChanged(BluetoothGatt gatt,
        BluetoothGattCharacteristic characteristic) {
    handleValue(characteristic.getUuid(), characteristic.getValue());
}

private void handleValue(UUID uuid, byte[] value) {
    if (TEMP_UUID.equals(uuid) && value.length >= 2) {
        // 例: 符号付き 16 ビット、リトルエンディアン、0.01 ℃ 単位
        int raw = (short) ((value[0] & 0xFF) | (value[1] << 8));
        double celsius = raw / 100.0;
        runOnUiThread(() -> textTemperature.setText(String.format("%.2f ℃", celsius)));
    }
}
```

## 8. つまずきやすい点

- **GATT 操作は 1 つずつ**: 書き込みの完了（`onCharacteristicWrite` など）を待たずに次の操作をすると、後の操作が失敗します。複数の操作は **キューに入れて順に実行** します。
- **必ず `close()` する**: 使い終わった・切断された `BluetoothGatt` は `gatt.close()` で解放します。解放しないと接続数の上限に達し、再接続できなくなります。Activity の `onDestroy` でも解放します。
- **status 133**: 原因がさまざまな接続エラーです。`close()` してから少し待って再接続する、スキャン停止後に接続する、などで改善することがあります。
- **一度に送れるデータは小さい**: 初期の MTU は 23 バイト（データ部は 20 バイト）。`gatt.requestMtu(247)` で拡張を交渉できます。
- **バイト順と単位**: 機器側と同じ形式（エンディアン、符号、スケール）で変換しないと、値がおかしくなります。
- **画面の回転で Activity が作り直される**: 接続はサービスや ViewModel に持たせ、Activity の作り直しで切れないようにします。

## 9. 動作確認の道具

- **nRF Connect for Mobile**（Nordic Semiconductor）: 別のスマホで BLE 機器のふりをさせたり、実際の機器のサービス・キャラクタリスティックを確認したりできます。自作アプリの前に、まずこのアプリで機器と通信できることを確かめると切り分けが楽になります。
- **ESP32 / micro:bit / Raspberry Pi**: ペリフェラル側を自作できます（[Raspberry Pi ノート](../../../03_embedded-linux/linux-network（Linux・ネットワーク）/05_raspberry_pi/README.md)）。
- **Logcat**: Android Studio のログ。`Log.d(TAG, "...")` でコールバックの順番や値を確認します。

## セルフチェック

1. スマートフォンと BLE センサ機器では、どちらがセントラルでどちらがペリフェラルか。
2. サービス・キャラクタリスティック・ディスクリプタの関係を説明せよ。
3. Android 12 以降で BLE のスキャンと接続に必要な権限は何か。
4. 通知（Notify）を受け取るために必要な 2 つの手順は何か。
5. `BluetoothGattCallback` の中で `TextView` を更新するとき、何に気を付けるか。
6. 続けて 2 つのキャラクタリスティックに書き込んだら、2 つ目が失敗した。原因と対策は何か。

<details>
<summary>答え</summary>

1. スマートフォンがセントラル、センサ機器がペリフェラル。  
2. サービスの中に複数のキャラクタリスティックがあり、キャラクタリスティックは値と、補足情報のディスクリプタを持つ。  
3. `BLUETOOTH_SCAN` と `BLUETOOTH_CONNECT`。  
4. `setCharacteristicNotification(…, true)` でスマホ側の受信を有効にし、CCCD ディスクリプタ（0x2902）に `ENABLE_NOTIFICATION_VALUE` を書き込む。  
5. コールバックはメインスレッド以外で呼ばれるので、`runOnUiThread` で更新する。  
6. GATT 操作は同時に 1 つしか実行できないため。1 つ目の完了コールバックを待ってから次を実行する（操作をキューで管理する）。
</details>
