# AE-LSM6DSV16X 動作確認 (ESP32 DevKit)

## 配線

電源を切ってから接続する。センサーの電源・信号は3.3 Vとし、5 Vを接続しない。

| センサーモジュール | ESP32 |
| --- | --- |
| 3V3 | 3V3 |
| GND | GND |
| SDA | GPIO21 |
| SCL | GPIO22 |
| CS | 3V3（I2Cモード） |
| SDO/SA0 | GND（アドレス0x6A） |

- J4（CSとGNDを接続）は開放する。短絡したままCSを3V3へ接続しない。
- SA0をGNDにする場合、J2（0x6B選択）は開放する。J3で0x6Aを選択してもよい。J2とJ3は同時に短絡しない。
- SDA/SCLには3.3 Vへのプルアップが必要。基板のJ1（I2C PULLUP）で有効にするか、外付け抵抗を使う。
- INT1/INT2など、表にない信号端子は本サンプルでは接続不要。
- GPIO21を他の機器の制御に使っている場合は配線を外すか、ソースのsdaPinを変更する。

## 書き込み・表示

PlatformIOの環境 `LSM6DSV16X_test` を選んでUploadし、115200 baudでシリアルモニターを開く。
既定の環境は変更していないので、環境を明示的に選ぶこと。

ターミナルから実行する場合:

```powershell
pio run -e LSM6DSV16X_test -t upload
pio device monitor -b 115200
```

起動時に0x6Aと0x6Bを探索し、WHO_AM_Iが0x70であることを確認する。
SparkFun 6DoF LSM6DSV16X 1.0.2を使用する。PlatformIOのlib_depsに設定済みのため、ビルド時に自動取得される。
120 Hzで測定し、約100 msごとに次のCSVを出力する。

```text
ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps
```

加速度はg単位（重力を含む）、角速度はdeg/s単位。
ライブラリが返すmgとmdpsを、それぞれ1000で割って表示する。
測定範囲は加速度±2 g、角速度±250 deg/s。
静止時、加速度ベクトルの大きさは約1 g、角速度は概ね0 deg/sになる。
オフセット補正や姿勢角の計算は行っていない。

検出失敗、通信失敗、データ更新のタイムアウト時はエラーを表示して停止する。
配線を確認してESP32をリセットする。

## 参考

- [SparkFun Arduinoライブラリ](https://github.com/sparkfun/SparkFun_LSM6DSV16X_Arduino_Library)
- [秋月 商品ページ](https://akizukidenshi.com/catalog/g/g130950/)
- [回路図](https://akizukidenshi.com/goodsaffix/回路図_20250113.pdf)
- [LSM6DSV16X データシート](https://akizukidenshi.com/goodsaffix/lsm6dsv16x.pdf)
