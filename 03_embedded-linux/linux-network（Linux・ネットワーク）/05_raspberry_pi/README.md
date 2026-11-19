# 05. Raspberry Pi — Linux・回路・ネットワークをつなぐ実習機

Raspberry Pi は、Linux が動く小型コンピュータに GPIO を備えたボードです。これまでの C 言語・電子回路・ネットワーク・Linux の知識を 1 台で試せます。

## 1. Raspberry Pi の種類

| モデル | 特徴 | 用途の目安 |
| --- | --- | --- |
| Raspberry Pi 5 / 4 Model B | 4 コア CPU、有線 LAN、USB 3.0、メモリ 1〜16 GB | デスクトップ、サーバ、画像処理 |
| Raspberry Pi Zero 2 W | 小型・低消費電力、Wi-Fi のみ | 小さな IoT 端末 |
| Raspberry Pi Pico / Pico 2 | **マイコン**（RP2040 / RP2350）。Linux は動かない | 厳密なタイミング制御、C / MicroPython |

「Raspberry Pi」と「Pico」は別物です。Pico は 02_microcontroller で学ぶマイコン側に近い存在です。

## 2. 準備と OS のインストール

必要なもの: 本体、microSD カード（16 GB 以上）、電源（Pi 5 は 5V/5A の USB-C 推奨、Pi 4 は 5V/3A）、必要に応じて LAN ケーブル。

1. PC に **Raspberry Pi Imager** を入れる。
2. デバイス・OS（Raspberry Pi OS）・書き込み先の microSD を選ぶ。
3. 「OS のカスタマイズ」で **ホスト名、ユーザ名とパスワード、Wi-Fi、SSH の有効化** を設定する。
4. 書き込んだ microSD を差して電源を入れる。

画面・キーボードなし（**ヘッドレス**）で使う場合は、手順 3 の SSH 設定が必須です。

## 3. SSH で接続する

```sh
ping raspberrypi.local          # mDNS で見つかるか（ホスト名は設定した名前）
ssh pi@raspberrypi.local        # ユーザ名@ホスト名
```

見つからない場合は、ルータの管理画面の DHCP 割り当て一覧で IP アドレスを確認し、`ssh pi@192.168.1.xx` で接続します。

初回にやること:

```sh
sudo apt update && sudo apt full-upgrade -y
sudo raspi-config        # I2C・SPI・シリアルの有効化、ロケール、タイムゾーンの設定
passwd                   # パスワード変更（初期設定のままにしない）
```

パスワードの代わりに **公開鍵認証** を使うと安全で便利です。PC で `ssh-keygen` を実行し、`ssh-copy-id pi@raspberrypi.local` で鍵を登録します。

## 4. GPIO ピン配置

40 ピンヘッダの代表的なピンです（物理ピン番号と **BCM 番号**（GPIOxx）は別物）。

| 物理ピン | 機能 | 物理ピン | 機能 |
| --- | --- | --- | --- |
| 1 | 3.3 V | 2 | 5 V |
| 3 | GPIO2（I2C SDA） | 4 | 5 V |
| 5 | GPIO3（I2C SCL） | 6 | GND |
| 7 | GPIO4 | 8 | GPIO14（UART TX） |
| 9 | GND | 10 | GPIO15（UART RX） |
| 11 | GPIO17 | 12 | GPIO18（PWM0） |
| 13 | GPIO27 | 14 | GND |
| 19 | GPIO10（SPI MOSI） | 21 | GPIO9（SPI MISO） |
| 23 | GPIO11（SPI SCLK） | 24 | GPIO8（SPI CE0） |

`pinout` コマンドで、手元のボードのピン配置を表示できます。

**電気的な注意**（[電子回路ノート](../../../01_fundamentals/electronics（電子回路）/README.md) 参照）

- GPIO は **3.3 V** です。**5 V の信号を入れると壊れます**。5 V 系センサはレベル変換を使う。
- 1 ピンから取れる電流は数 mA 程度。モーターやリレーは直接駆動せず、トランジスタやドライバ IC を介す。
- 配線は電源を切った状態で行う。5 V ピンと GPIO のショートに注意する。
- Raspberry Pi には **ADC がありません**。アナログセンサは MCP3008（SPI）や ADS1115（I2C）などの外付け ADC で読む。

## 5. 実習の流れ（L チカからデータ送信まで）

### 5.1 LED を点滅させる（L チカ）

```text
GPIO17（物理 11）──[ 330 Ω ]──|>|── GND（物理 9）
                              LED
```

Python（Raspberry Pi OS に標準で入っている `gpiozero`）:

```python
from gpiozero import LED
from time import sleep

led = LED(17)
for _ in range(10):
    led.toggle()
    sleep(0.5)
```

C で書く場合は [04 組込み Linux プログラム](../04_embedded_linux_programming/README.md) の libgpiod の例を使います。

### 5.2 スイッチを読む

```python
from gpiozero import Button
from signal import pause

button = Button(27)              # 内部プルアップが有効。押すと GND につながる配線
button.when_pressed = lambda: print("押された")
pause()
```

内部プルアップにより、押していないときは `1`、押すと `0` になります（負論理）。

### 5.3 I2C センサを読む

```sh
i2cdetect -y 1        # 例: 0x76 に BME280（温湿度・気圧センサ）が見える
```

### 5.4 ネットワークに送る

MQTT でセンサ値を送る例です（`sudo apt install mosquitto-clients`）。

```sh
# 受信側（別ターミナル）
mosquitto_sub -h 192.168.1.10 -t "lab/raspi/temp"

# 送信側
mosquitto_pub -h 192.168.1.10 -t "lab/raspi/temp" -m "23.5"
```

ここまでで「センサ → Raspberry Pi → ネットワーク → サーバ」という IoT の基本形ができます。04_application/monitoring の土台になります。

## 6. 運用のポイント

- **電源不足**: 低電圧だと動作が不安定になる。`vcgencmd get_throttled` が `0x0` 以外なら電源や発熱を疑う。
- **温度**: `vcgencmd measure_temp` で確認。高負荷時はヒートシンクやファンを付ける。
- **シャットダウン**: 電源を抜く前に `sudo shutdown -h now`。いきなり抜くと SD カードが壊れることがある。
- **バックアップ**: 設定が固まったら SD カードのイメージを保存しておく。

## セルフチェック

1. Raspberry Pi の GPIO の電圧は何 V か。5 V 信号を直接入れてよいか。
2. 物理ピン 11 は BCM 番号で何番か。
3. Raspberry Pi でアナログセンサの値を読むには何が必要か。
4. 画面なしで初回から SSH 接続するには、どの段階で何を設定するか。
5. 電源や温度による性能制限が起きているかを確認するコマンドは何か。

<details>
<summary>答え</summary>

1. 3.3 V。5 V を直接入れてはいけない（レベル変換が必要）。  
2. GPIO17。  
3. 外付けの ADC（MCP3008、ADS1115 など）。  
4. Raspberry Pi Imager の「OS のカスタマイズ」で、SSH の有効化とユーザ・Wi-Fi を書き込み時に設定する。  
5. `vcgencmd get_throttled`。
</details>
