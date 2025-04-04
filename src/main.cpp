#include <Arduino.h>
#include <BleKeyboard.h>
#include <esp_sleep.h>

BleKeyboard bleKeyboard("XiaoC3_Media");

#define PIN_WRITE 20 // GPIO2を使用する
#define PIN_READ 2

unsigned long previousMillis = 0; // will store last time LED was updated
const long interval = 1 * 200;    // interval at which to blink (milliseconds)

unsigned long lastActivityTime = 0;               // 最後にボタンが押された時刻を記録
const unsigned long inactivityThreshold = 60000;  // 1分（60000ミリ秒）操作がない場合にスリープするよう設定

void setup()
{
  // PIN出力モード
  pinMode(PIN_WRITE, OUTPUT);
  pinMode(PIN_READ, INPUT_PULLUP);
  Serial.begin(115200);
  digitalWrite(PIN_WRITE, HIGH);
  bleKeyboard.begin();
  // GPIO2が LOW（ボタンが押された状態）になるとESP32をスリープ解除
  esp_deep_sleep_enable_gpio_wakeup(BIT(PIN_READ), ESP_GPIO_WAKEUP_GPIO_LOW);
}

void loop()
{
  unsigned long currentMillis = millis();
  // 1秒周期で点滅を行う(500ms点灯、500ms消灯)
  if (digitalRead(PIN_READ) == 0 && currentMillis - previousMillis >= interval)
  {
    lastActivityTime = currentMillis;
    previousMillis = currentMillis;
    if (bleKeyboard.isConnected())
    {
      Serial.println("Sending Play/Pause media key...");
      bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
      Serial.println("Sending Play/Pause media key Complete!");
    }
  }

  if (millis() - lastActivityTime > inactivityThreshold) {
    Serial.println("Go to DeepSleep!!");
    delay(1000);             // 少し待ってからスリープ
    esp_deep_sleep_start();  // ディープスリープに入る
  }
}