# 04. 組込み Linux プログラム — C から周辺機器を動かす

マイコン（ベアメタル）ではレジスタを直接書きますが、組込み Linux ではカーネルのドライバを通して周辺機器を扱います。C 言語ノートで学んだファイル操作の知識が、そのままデバイス操作につながります。

## 1. マイコンと組込み Linux の違い

| 項目 | マイコン（ベアメタル / RTOS） | 組込み Linux |
| --- | --- | --- |
| 例 | Arduino、STM32、Raspberry Pi Pico | Raspberry Pi、BeagleBone、i.MX 系ボード |
| ハードウェアへのアクセス | レジスタを直接読み書き | ドライバ経由（`/dev`、`/sys`、システムコール） |
| メモリ保護 | なし（または簡易） | MMU でプロセスごとに保護 |
| 起動時間 | ミリ秒 | 数秒〜数十秒 |
| リアルタイム性 | 高い（割り込み応答が予測しやすい） | 一般的には保証されない（PREEMPT_RT で改善） |
| 得意なこと | 厳密なタイミング制御、低消費電力 | ネットワーク、ファイル、マルチタスク、豊富なライブラリ |

厳密なタイミングが必要な部分はマイコンに、通信や記録は Linux に任せる **組み合わせ構成** もよく使われます。

## 2. 開発環境とクロスコンパイル

- **セルフ開発**: Raspberry Pi 上で `gcc` を使って直接ビルドする。手軽だが大きなプログラムは遅い。
- **クロス開発**: PC（ホスト、x86_64）で ARM 向け（ターゲット）の実行ファイルを作る。

```sh
# Ubuntu の PC に 64 ビット ARM 向けクロスコンパイラを入れる
sudo apt install gcc-aarch64-linux-gnu

aarch64-linux-gnu-gcc -std=c11 -Wall -O2 hello.c -o hello
file hello        # ELF 64-bit LSB executable, ARM aarch64 … と表示される
scp hello pi@192.168.1.20:~/
```

ターゲットとホストでライブラリ（glibc）のバージョンが大きく違うと実行できないことがあります。本格的な製品開発では、OS イメージごと作る **Yocto Project** や **Buildroot** を使います。

## 3. Linux の起動の流れ

```text
電源 ON → ブートローダ（U-Boot など） → カーネル + デバイスツリー読み込み
       → init（systemd）起動 → 各サービス起動 → ログイン / アプリ起動
```

**デバイスツリー**（`.dtb`）は「このボードにはどこに何の周辺機器がつながっているか」をカーネルに伝えるデータです。Raspberry Pi では `config.txt` の `dtoverlay=` で追加の設定を読み込みます。

## 4. デバイスへのアクセス方法

| 方法 | 例 | 使いどころ |
| --- | --- | --- |
| デバイスファイル + `open`/`read`/`write`/`ioctl` | `/dev/i2c-1`、`/dev/spidev0.0`、`/dev/ttyAMA0` | I2C・SPI・UART |
| sysfs | `/sys/class/thermal/…`、`/sys/class/leds/…` | 状態の読み出し、簡単な設定 |
| GPIO キャラクタデバイス + libgpiod | `/dev/gpiochip0` | GPIO（現在の推奨方式） |

GPIO を `/sys/class/gpio` で操作する方法は **非推奨** です。新しく書くプログラムでは **libgpiod** を使います。

```sh
sudo apt install gpiod libgpiod-dev
gpiodetect                  # GPIO チップの一覧
gpioinfo                    # 各ラインの状態
```

## 5. C で GPIO を制御する（libgpiod v1 の例）

```c
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

#define CHIP_NAME "gpiochip0"
#define LED_LINE  17          /* BCM 番号の GPIO17 */

int main(void)
{
    struct gpiod_chip *chip = gpiod_chip_open_by_name(CHIP_NAME);
    struct gpiod_line *line;

    if (chip == NULL) {
        perror("gpiod_chip_open_by_name");
        return 1;
    }

    line = gpiod_chip_get_line(chip, LED_LINE);
    if (line == NULL || gpiod_line_request_output(line, "blink", 0) < 0) {
        perror("GPIO を出力に設定できません");
        gpiod_chip_close(chip);
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        gpiod_line_set_value(line, i % 2);   /* 1 で点灯、0 で消灯 */
        usleep(500 * 1000);
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}
```

```sh
gcc -Wall blink.c -o blink -lgpiod
./blink
```

libgpiod は v1 と v2 で API が大きく異なります（v2 は `gpiod_line_request` 系）。`apt show libgpiod-dev` でバージョンを確認し、対応するドキュメントを見てください。

## 6. I2C デバイスを読む

```sh
sudo raspi-config           # Interface Options → I2C を有効化
sudo apt install i2c-tools
i2cdetect -y 1              # バス 1 に接続されたデバイスのアドレスを表示
```

```c
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(void)
{
    int fd = open("/dev/i2c-1", O_RDWR);
    unsigned char reg = 0x00;      /* 読みたいレジスタ（デバイスのデータシート参照） */
    unsigned char data[2];

    if (fd < 0) {
        perror("open");
        return 1;
    }
    if (ioctl(fd, I2C_SLAVE, 0x48) < 0) {   /* 0x48 はデバイスのアドレス例 */
        perror("ioctl");
        close(fd);
        return 1;
    }
    if (write(fd, &reg, 1) != 1 || read(fd, data, 2) != 2) {
        perror("I2C 通信に失敗しました");
        close(fd);
        return 1;
    }

    printf("raw = 0x%02X%02X\n", data[0], data[1]);
    close(fd);
    return 0;
}
```

## 7. 常駐プログラム（systemd サービス）

電源投入で自動起動し、落ちたら再起動させるには systemd のユニットファイルを作ります。

```ini
# /etc/systemd/system/sensor-logger.service
[Unit]
Description=Sensor logger
After=network-online.target
Wants=network-online.target

[Service]
ExecStart=/home/pi/bin/sensor_logger
Restart=on-failure
User=pi

[Install]
WantedBy=multi-user.target
```

```sh
sudo systemctl daemon-reload
sudo systemctl enable --now sensor-logger
journalctl -u sensor-logger -f      # printf / stderr の出力はここに出る
```

## 8. 組込みで気を付けること

- **戻り値を必ず確認する**: `open`、`read`、`write` は失敗し得る。`perror` で原因を出す。
- **シグナルで安全に終了する**: `SIGTERM` を受けたら GPIO を初期状態に戻し、ファイルを閉じてから終了する。
- **SD カードの寿命**: 頻繁な書き込みは寿命を縮める。ログはまとめて書く、`/tmp`（tmpfs）を使うなどの工夫をする。
- **突然の電源断**: 書き込み途中で電源が切れるとファイルが壊れる。重要データは `fsync` してから閉じる。
- **時刻**: RTC がないボードでは、NTP 同期前の時刻が正しくない。

## セルフチェック

1. 組込み Linux がマイコンに比べて苦手なことを 1 つ挙げよ。
2. PC で ARM 向けの実行ファイルを作ることを何と呼ぶか。
3. 現在推奨される GPIO の操作方法（ライブラリ）は何か。
4. I2C バス 1 に接続されたデバイスのアドレスを調べるコマンドは何か。
5. systemd サービスが異常終了したときに自動で再起動させる設定は何か。

<details>
<summary>答え</summary>

1. 厳密なリアルタイム性（ほかに起動時間、消費電力）。  
2. クロスコンパイル（クロス開発）。  
3. libgpiod（GPIO キャラクタデバイス `/dev/gpiochipN`）。  
4. `i2cdetect -y 1`。  
5. `[Service]` セクションの `Restart=on-failure`。
</details>
