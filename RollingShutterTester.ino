/*
RollingShutterTester
V1.0.0 citriena

カメラの幕速解析用に複数のLEDバーを点灯切替するする装置用Arduinoスケッチ

縦向きのLEDバーを平行に5本、もしくは10，16本設置し、一定間隔で順次点灯させる。
グローバルシャッターなら点灯したLEDバーが切れずに全体が点灯した状態で写るが、ローリングシャッターでは
LEDの点灯切替時間が幕速よりも速ければLEDバーが一部分だけ点灯した状態で写る。
LEDの点灯時間と上記のLEDバーの画像から幕速を算出可能
*/

//#define I2C                 // 非I2C接続のキーパッドシールド使用時はコメントアウト
#define UNO_ASBL   //UNOの場合、UNOではアセンブラで多少高速化（0.1ms以下では指定しないと誤差が大きい）

// LEDバーの数と点滅方法の選択
#define NUM_BAR  5  // アナログキーパッド用 5本順次点灯（デジタルピン：2, 3, 11, 12, 13）
//#define NUM_BAR 10  // アナログキーパッド用10本順次点灯（アナログキーパッド最大数；デジタルピン：2, 3, 11, 12, 13, 15, 16, 17, 18, 19）
//#define NUM_BAR 16  // I2Cキーパッド用16本順次点灯（I2Cキーパッドの最大数；デジタルピン：2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17）
//#define BLINK   // 順に点灯、消灯ではなく全体を一斉に点滅させる場合　自動的にUNO_ASBLが定義される。LEDバーは5本

//#define SHOW_KEY_READ_TIME  //キー読みこみ時間表示

#ifdef BLINK
#define UNO_ASBL
#endif

#ifdef I2C
#include <LiquidTWI2.h>         // https://github.com/lincomatic/LiquidTWI2
#include <simpleKeypad_I2C.h>   // https://github.com/citriena/simpleKeypad_I2C
LiquidTWI2 lcd(0x20);           // LCDのI2Cアドレス設定
#else
#include <LiquidCrystal.h>
#include <simpleKeypad.h>       // https://github.com/citriena/simpleKeypad
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#endif

simpleKeypad keypad(200, 800, 200, true);

#if NUM_BAR == 5
#define NO_CYCLE 5
byte row1[5] = { 2, 3, 11, 12, 13}; // LED列のピン番号。この順にLED列を点灯。
byte row2[5] = {13, 2,  3, 11, 12}; // row1を点灯後にこのピン番号のLED列を消灯。

// アセンブラでのポート制御用。最後は全点灯、全消灯用
byte portDo[6] = {0b00000100, 0b00001000, 0b00000000, 0b00000000, 0b00000000, 0b00001100}; // 点灯は1 各ビットは D07, D06, D05, D04, D03, D02, D01, D00
byte portDa[6] = {0b11111111, 0b11111011, 0b11110111, 0b11111111, 0b11111111, 0b11110011}; // 消灯は0

byte portBo[6] = {0b00000000, 0b00000000, 0b00001000, 0b00010000, 0b00100000, 0b00111000}; // 点灯は1 各ビットは XT2, XT1, D13, D12, D11, D10, D09, D08
byte portBa[6] = {0b11011111, 0b11111111, 0b11111111, 0b11110111, 0b11101111, 0b11000111}; // 消灯は0

#endif

#if NUM_BAR == 10
#define NO_CYCLE 10
byte row1[10] = { 2, 3, 11, 12, 13, 15, 16, 17, 18, 19}; // LED列のピン番号。この順にLED列を点灯。
byte row2[10] = {19, 2,  3, 11, 12, 13, 15, 16, 17, 18}; // row1を点灯後にこのピン番号のLED列を消灯。

// アセンブラでのポート制御用。最後は全点灯、全消灯用
byte portDo[11] = {0b00000100, 0b00001000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001100}; // 各ビットは D07, D06, D05, D04, D03, D02, D01, D00
byte portDa[11] = {0b11111111, 0b11111011, 0b11110111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11110011};

byte portBo[11] = {0b00000000, 0b00000000, 0b00001000, 0b00010000, 0b00100000, 0b00000100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111000}; // 各ビットは XT2, XT1, D13, D12, D11, D10, D09, D08
byte portBa[11] = {0b11111111, 0b11111111, 0b11111111, 0b11110111, 0b11101111, 0b11011111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11000111};

byte portCo[11] = {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b00111110}; // 各ビットは ???, RST, A05, A04, A03, A02, A01, A00
byte portCa[11] = {0b11011111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111101, 0b11111011, 0b11110111, 0b11101111, 0b11000001}; //                    D19, D18, D17, D16, D15, D14
#endif

#if NUM_BAR == 16
#define NO_CYCLE 16
byte row1[16] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}; // LED列のピン番号。この順にLED列を点灯。最後は全消灯用。
byte row2[16] = {17, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13, 14, 15, 16}; // row1を点灯後にこのピン番号のLED列を消灯。

// アセンブラでのポート制御用。最後は全点灯、全消灯用
byte portDo[17] = {0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111100}; // 各ビットは D07, D06, D05, D04, D03, D02, D01, D00
byte portDa[17] = {0b11111111, 0b11111011, 0b11110111, 0b11101111, 0b11011111, 0b10111111, 0b01111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000011};

byte portBo[17] = {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111111}; // 各ビットは XT2, XT1, D13, D12, D11, D10, D09, D08
byte portBa[17] = {0b11111111, 0b11111111, 0b11111111, 0b11110111, 0b11111111, 0b11111111, 0b11111111, 0b11111110, 0b11111101, 0b11111011, 0b11110111, 0b11101111, 0b11011111, 0b11111111, 0b11111111, 0b11111111, 0b11000000};

byte portCo[17] = {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b00000000, 0b00000000, 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00001111}; // 各ビットは ???, RST, A05, A04, A03, A02, A01, A00
byte portCa[17] = {0b11110111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111101, 0b11111011, 0b11110111, 0b11101111, 0b11011111, 0b11111111, 0b11111111, 0b11111110, 0b11111101, 0b11111011, 0b11110000}; //                    D19, D18, D17, D16, D15, D14
#endif



#ifdef BLINK
#define NUM_BAR 5
#define NO_CYCLE 2
byte row1[5] = { 2, 3, 11, 12, 13}; // LED列のピン番号

byte portDo[3] = {0b00001100, 0b00000000, 0b00001100}; // 点灯は1 各ビットは D07, D06, D05, D04, D03, D02, D01, D00
byte portDa[3] = {0b11111111, 0b11110011, 0b11110011}; // 消灯は0

byte portBo[3] = {0b00111000, 0b00000000, 0b00111000}; // 点灯は1 各ビットは XT2, XT1, D13, D12, D11, D10, D09, D08
byte portBa[3] = {0b11111111, 0b11000111, 0b11000111}; // 消灯は0
#endif

unsigned long switchingTime = 20000;   // 切替時間初期値（uSec）


void setup() {
  int i;
#ifdef I2C
  lcd.setMCPType(LTI_TYPE_MCP23017); // I2C LCDキーパッドシールドの設定（IOエキスパンダーの指定）
#endif
  lcd.begin(16, 2);
  Serial.begin(9600);

  for (i = 0; i < NUM_BAR; i ++) {
    pinMode(row1[i], OUTPUT);
    digitalWrite(row1[i], LOW);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rolling Time Tst");
  lcd.setCursor(0, 1);
  lcd.print("CHG Int:");
  printInterval();
}

void loop() {
  byte i, k;
  byte cycle = 20;  // ループの回数。切替時間が短い場合は50に上げる。ループ間にread_key()等の処理を行う。切替時間が長い場合は次のループも
                    // タイムラグ無しで処理できるが、切替時間がread_key()等の処理時間より短い場合はサイクルを1回とばす。
                    // 切替時間が長い場合は20回も待っていたらキー処理のタイムラグが長くなるので、切替毎にread_key()処理を行う。
                    // 割り込みを使えばこんな面倒なことしなくて済むが。
  unsigned long mTime;
  byte portDx, portBx, portCx;
  unsigned long pSwitchingTime = switchingTime;
  byte withinTime = 5; // これが0になるまで判定。2だとギリギリのタイミングの場合、キーの押し方によっては間に合わないと判定されることがある。
                       // このため、キーを押した直後は判定中なのでループの最後の切替は不安定。数msなので事実上問題ない。

  mTime = micros() + 20;
  do {
    if (switchingTime < 200) {       // 0.2msより短い場合は最低50回にする。
      cycle = 50;
    }
    if (mTime < micros()) {          // 一旦ループを出てmTimeを過ぎていたら設定し直す。また、判定に使わないように1周分消灯する。
      if (withinTime > 0) {          // 現在は0.20msまではループ間でタイムラグ無しで処理できている。0.19ms以下では処理が間に合わず、ループ間で1サイクル飛ばしている。
        withinTime--;
      } else {
        mTime += (switchingTime * NUM_BAR);
        while (mTime > micros());   // switchingTime * NUM_BAR だけ待つ（＝1周）
      }
      mTime = micros() + 20;        // 少し大きくしないと最初の1回目が安定しない。
    }

    for (k = 0; k < cycle; k++) {   // 20回もしくは50回はタイムラグ無しで回す。この間は周をまたいでも幕速測定に使える。
      for (i = 0; i < NO_CYCLE; i++) {
        if (switchingTime < 200) {               // 200us(0.2ms)以下ではdelayMicroseconds()を使う。
#ifdef UNO_ASBL
          delayMicroseconds(switchingTime - 4);  // 5で9us弱なので補正
#else
          delayMicroseconds(switchingTime - 10); // アセンブラを使わない場合は補正値が異なる。
#endif
        } else {
          while (mTime > micros());              // 200us以上は誤差小さいのでmicros()を監視。delayMicroseconds()は数千以上では使わない方が良いらしい。
        }

#ifndef UNO_ASBL
        digitalWrite(row2[i], LOW);   // 現在のLEDを消灯する。
        digitalWrite(row1[i], HIGH);  // 次のLEDを点灯し
#else
        portDx = (PORTD | portDo[i]) & portDa[i];
        portBx = (PORTB | portBo[i]) & portBa[i];
#if (NUM_BAR == 10) || (NUM_BAR == 16)
        portCx = (PORTC | portCo[i]) & portCa[i];
#endif
        PORTD = portDx;
        PORTB = portBx;
#if (NUM_BAR == 10) || (NUM_BAR == 16)
        PORTC = portCx;
#endif
#endif
        mTime += switchingTime;
        if ((switchingTime > 2000) && (mTime > (micros() + 512))) {
          key_read();  // key_read()にアナログでは最低112us、I2Cでは最低約464usかかるので余裕がなければ処理しない。
        }
      } // ここまでが1周処理
    }   // ここまでが20 or 50回処理

    if (withinTime == 0) {                     // 時間内に次周に合わない場合は、ここで最後のバーを指定時間後消灯する。
      if (switchingTime < 200) {               // 200us(0.2ms)以下ではdelayMicroseconds()を使う。
#ifdef UNO_ASBL
        delayMicroseconds(switchingTime - 4);  // 5で9us弱なので補正（8.7usくらいか）補正後は設定10usで実測10.3us程度
#else
        delayMicroseconds(switchingTime - 10); // アセンブラを使わない場合は補正値が異なる。
#endif
      } else {
        while (mTime > micros());              // 200us以上は誤差小さいのでmicros()を監視。delayMicroseconds()は数千以上では使わない方が良い。
      }
#ifndef UNO_ASBL
      digitalWrite(row2[0], LOW);              // ここで消灯
#else  // 全消灯
      PORTD &= portDa[NO_CYCLE];
      PORTB &= portBa[NO_CYCLE];
#if (NUM_BAR == 10) || (NUM_BAR == 16)
      PORTC &= portCa[NO_CYCLE];
#endif
#endif
    }

#ifdef SHOW_KEY_READ_TIME
    long kt1 = micros();
#endif

    key_read();

    if (switchingTime != pSwitchingTime) {
      pSwitchingTime = switchingTime;
      withinTime = 5;         // 切替時間が変わった場合は、1周内に処理しきれるか一旦判定するために数をリセットする。
      mTime = micros() + 20;  // 判定のために一度はtrueで回す必要があるのでmTime再設定
    }

#ifdef SHOW_KEY_READ_TIME
    long kt2 = micros();
    lcd.setCursor(0, 0);
    lcd.print(kt2 - kt1);
    lcd.print(F(" us"));
#endif
  } while(1);
}


void key_read() {
  int lcd_key;

  lcd_key = keypad.read_buttons();
  switch (lcd_key){
    case btnRIGHT:{
      if (switchingTime < 1000) {         // 1ms未満では10us上げる。
        switchingTime += 10;
      } else if (switchingTime < 50000) {
        switchingTime += 100;             // 50ms未満では100us上げる。
      } else {                            // 50ms以上では1mS上げる。
        if (switchingTime <= (0xFFFFFFFF - 1000)) switchingTime += 1000;
      }
      break;
    }
    case btnLEFT:{
      if (switchingTime <= 1000) {
        if (switchingTime >  10) switchingTime -=  10;
      } else if (switchingTime <= 50000) {
        if (switchingTime > 100) switchingTime -= 100;
      } else {
        switchingTime -= 1000;
      }
      break;
    }    
    case btnUP:{
      if (switchingTime < 1000) {         // 1ms未満では100us上げる。
        switchingTime += 100;
        if (switchingTime > 1000) {
          switchingTime /= 100;
          switchingTime *= 100;
        }
      } else if (switchingTime < 50000) { // 50ms未満では1ms上げる。
        switchingTime += 1000;
        if (switchingTime > 50000) {
          switchingTime /= 1000;
          switchingTime *= 1000;
        }
      } else {
        if (switchingTime <= (0xFFFFFFFF - 10000)) switchingTime += 10000;
      }
      break;
    }
    case btnDOWN:{
      if (switchingTime <= 1000) {
        if (switchingTime > 100) switchingTime -= 100;
      } else if (switchingTime <= 50000) {
        if (switchingTime > 1000) switchingTime -= 1000;
      } else {
        switchingTime -= 10000;
      }
      break;
    }
    default:{
      return;
    }
  }
  printInterval();
}

void printInterval() {
  lcd.setCursor(8, 1);
  if (switchingTime < 100000) lcd.print(" ");
  if (switchingTime <  10000) lcd.print(" ");
//  if (switchingTime < 1000)  lcd.print(" ");
//  if (switchingTime < 100)   lcd.print(" ");
//  if (switchingTime < 10)    lcd.print(" ");
  lcd.print(switchingTime / 1000);
  lcd.print('.');
  unsigned long iud = (switchingTime % 1000) / 10;
  if (iud < 10) lcd.print("0");
  lcd.print(iud);
  lcd.print("ms");
}
