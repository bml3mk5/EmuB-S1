# HITACHI MB-S1 model05 Emulator Qt edition

#### Copyright(C) Common Source Code Project, Sasaji 2012-2025 All Rights Reserved.

## ファイル構成

    docs/ .................. ドキュメント
    font/ .................. 画面フォント作成スクリプト
      kanji/ ............... 擬似漢字ROM作成ツール
        dict/ .............. 擬似漢字辞書ROM作成スクリプト
        SDL/ ............... 擬似漢字ROM作成プログラム(SDL使用)
        Win/ ............... 擬似漢字ROM作成プログラム(WinAPI使用)
    source/
      data/ ................ データファイル
      include/ ............. インクルードファイル
      lib/ ................. ライブラリファイル
      locale/ .............. 翻訳用(gettext)
        list.xml ........... 言語一覧
        ja/
          mbs1_ja.qm ....... Qt用 日本語翻訳ファイル(変換済み)
          mbs1_ja.ts ....... Qt用 日本語翻訳ファイル
          LC_MESSAGES/
      src/ ................. ソースファイル
        extra/ ............. その他ソース
        gui/ ............... GUI関連ソース
          qt/ .............. Qt GUI関連ソース
        osd/ ............... OS依存関連ソース
          qt/ .............. Qt依存関連ソース
        res/ ............... リソースファイル
        video/ ............. 録画用関連ソース
          avkit/ ........... AVkit(mac用)関連ソース
          ffmpeg/ .......... ffmpeg関連ソース
          qt/ .............. Qt用関連ソース
          qtkit/ ........... qtkit(mac用)関連ソース
          vfw/ ............. video for windows関連ソース
          wave/ ............ waveフォーマット関連ソース
        vm/ ................ VMメインプログラムソース
      patch/ ............... パッチファイル
      Qt/ .................. Qt creator用プロジェクトファイル
      README_QT.md ......... このファイル


## 必要なバージョン

  Qt Ver5.7以上 + QT Gamepad [1]

  [1] QT GamepadはQt5.9から標準コンポーネントに含まれるようです。
  [2] Qt6ではQGamepadがサポート外になったため、ゲームパッドは
      使用できません。


## QT Creatorを使用したコンパイル方法

 * QT Creator で .proファイルを読み込んでください。
 * ビルドステップを追加。
   + makeを選択 Makeの引数に"install"と入力。
     * installを実行すると、リソースなどをビルドディレクトリにコピーします。
       システムディレクトリ(/usr/localなど)にインストールはしません。

 * FFmpeg-4.x (https://ffmpeg.org/)

   (FFmpegを使用しない場合、src/rec_video_defs.hにある #define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)

   + ヘッダファイルが必要です。
     includeフォルダにヘッダファイルを入れてください。

   + Ubuntuならパッケージから以下をインストール:
     libavcodec-dev, libavformat-dev, libswscale-dev


## QT Creatorを使用しないでコンパイルする方法

 * Ubuntu19.04の例：
   1. パッケージのインストール(sudo apt install xxxxx)
      libqt5opengl5-dev, libqt5gamepad5-dev, qtmultimedia5-dev
    * ffmpegを使用する場合
      libavcodec-dev, libavformat-dev, libswscale-devも必要。

   2. source/Qt配下にビルド用のサブディレクトリを作成し、そこにcdする。

   3. qtchooser -run-tool=qmake -qt=5 ../mbs1_qt.pro
      正常ならMakefileができるはず
    * デバッガ付きの場合
      qtchooser -run-tool=qmake -qt=5 ../mbs1_qt.pro "CONFIG+=debugger"

   4. makeとmake installを実行。
      正常なら実行ファイルができるはず。
    * installを実行すると、リソースなどをこのディレクトリにコピーします。
      システムディレクトリ(/usr/localなど)にインストールはしません。


## 翻訳ファイル作成方法

  1. 翻訳対象をソースコードから抽出
     lupdate -tr-function-alias QT_TR_NOOP+=_TX mbs1_qt.pro
  2. リリース .qmファイルを作る。
     lrelease


## 翻訳ファイル(.po)マージ方法

  1. linguistを起動して mbs1.po を読み込む。
  2. 名前を付けて保存で、mbs1_ja.tsファイルに書き出す。
  3. 翻訳対象をソースコードから抽出
     lupdate -tr-function-alias QT_TR_NOOP+=_TX mbs1_qt.pro
  4. ツール -> 外部 -> lreleaseで翻訳ファイルをリリース。


----------------------------------------
## 免責事項

* このソフトはフリーウェアです。ただし、著作権は放棄しておりません。
  実行モジュールについては作者Sasajiにあります。
  ソースコードについてはそれぞれの作者にあります。
* このソフトによって発生したいかなる損害についても著作権者は一切責任を負いません。
  このソフトを使用するにあたってはすべて自己責任で行ってください。
* 雑誌やネットなどに転載される場合、不特定多数の方に再配布を行う場合でも
  承諾の必要はありませんが、転載の旨をご連絡いただけたら幸いです。

------------------------------------------------------------------------------

連絡先：Sasaji (sasaji@s-sasaji.ddo.jp)
 * My WebPage: http://s-sasaji.ddo.jp/bml3mk5/
 * GitHub:     https://github.com/bml3mk5/EmuB-S1
 * X(Twitter): https://x.com/bml3mk5

------------------------------------------------------------------------------

