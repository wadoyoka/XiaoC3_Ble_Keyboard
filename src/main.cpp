#include <Arduino.h>
#include <BleKeyboard.h>
#include <esp_sleep.h>

BleKeyboard bleKeyboard("XiaoC3_Media");

#define LEFT_PIN 7
#define PWD_PAUSE_PIN 5
#define RIGHT_PIN 6
#define VOLUME_UP_PIN 4
#define VOLUME_DOWN_PIN 10
#define LED_PIN 21

#define KEY_SHIFT 0x02

const int PINS_INDEXS[] = {LEFT_PIN, PWD_PAUSE_PIN, RIGHT_PIN, VOLUME_UP_PIN, VOLUME_DOWN_PIN};
const int PINS_SIZE = sizeof(PINS_INDEXS) / sizeof(int);

unsigned long previousMillis = 0; // will store last time LED was updated
const long interval = 1 * 200;    // interval at which to blink (milliseconds)

unsigned long lastActivityTime = 0;              // 最後にボタンが押された時刻を記録
const unsigned long inactivityThreshold = 60000; // 1分（60000ミリ秒）操作がない場合にスリープするよう設定

bool isPressed = false; // 長押し対策
const unsigned long holdInterval = 1.1 * 1000;
bool isCompletedHoldAction = false;

int pressedPin = -1;

void setup()
{
  // PIN出力モード
  for (size_t i = 0; i < PINS_SIZE; i++)
  {
    pinMode(PINS_INDEXS[i], INPUT_PULLUP);
  }
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  bleKeyboard.begin();
  // GPIO2が LOW（ボタンが押された状態）になるとESP32をスリープ解除
  esp_deep_sleep_enable_gpio_wakeup(BIT(PWD_PAUSE_PIN), ESP_GPIO_WAKEUP_GPIO_LOW);
  digitalWrite(LED_PIN, HIGH);
}

void loop()
{
  unsigned long currentMillis = millis();
  // 1秒周期で点滅を行う(500ms点灯、500ms消灯)
  if (bleKeyboard.isConnected() && currentMillis - previousMillis >= interval && !isPressed)
  {
    for (size_t i = 0; i < PINS_SIZE; i++)
    {
      if (digitalRead(PINS_INDEXS[i]) == 0)
      {
        pressedPin = PINS_INDEXS[i];
        break;
      }
    }

    if (pressedPin != -1)
    {
      lastActivityTime = currentMillis;
      previousMillis = currentMillis;
      isPressed = true;
    }

    switch (pressedPin)
    {
    case LEFT_PIN:
      Serial.println("Sending KEY_LEFT_ARROW...");
      bleKeyboard.write(KEY_LEFT_ARROW);
      Serial.println("Sending KEY_LEFT_ARROW Complete!");
      break;
    case PWD_PAUSE_PIN:
      Serial.println("Sending Play/Pause media key...");
      bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
      Serial.println("Sending Play/Pause media key Complete!");
      break;
    case RIGHT_PIN:
      Serial.println("Sending KEY_RIGHT_ARROW...");
      bleKeyboard.write(KEY_RIGHT_ARROW);
      Serial.println("Sending KEY_RIGHT_ARROW Complete!");
      break;
    case VOLUME_UP_PIN:
      Serial.println("Sending KEY_MEDIA_VOLUME_UP...");
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
      Serial.println("Sending KEY_MEDIA_VOLUME_UP Complete!");
      break;
    case VOLUME_DOWN_PIN:
      Serial.println("Sending KEY_MEDIA_VOLUME_DOWN...");
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      Serial.println("Sending KEY_MEDIA_VOLUME_DOWN Complete!");
      break;
    default:
      break;
    }
  }
  else if (digitalRead(pressedPin) == 1)
  {
    isPressed = false;
    isCompletedHoldAction = false;
    pressedPin = -1;
  }
  else if (isPressed && currentMillis - previousMillis >= holdInterval && pressedPin != -1 && !isCompletedHoldAction)
  {
    switch (pressedPin)
    {
    case LEFT_PIN:
      Serial.println("Sending SHIFT+P");
      // Press SHIFT and N together
      bleKeyboard.press(KEY_SHIFT);
      bleKeyboard.press('P');
      // Small delay
      delay(100);
      // Release both keys
      bleKeyboard.releaseAll();
      Serial.println("Sending SHIFT+N Complete!");
      isCompletedHoldAction = true;
      break;
    case RIGHT_PIN:
      Serial.println("Sending SHIFT+N");
      // Press SHIFT and N together
      bleKeyboard.press(KEY_SHIFT);
      bleKeyboard.press('N');
      // Small delay
      delay(100);
      // Release both keys
      bleKeyboard.releaseAll();
      Serial.println("Sending SHIFT+N Complete!");
      isCompletedHoldAction = true;
      break;
    case VOLUME_UP_PIN:
      Serial.println("Sending KEY_MEDIA_VOLUME_UP...");
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
      Serial.println("Sending KEY_MEDIA_VOLUME_UP Complete!");
      delay(100);
      break;
    case VOLUME_DOWN_PIN:
      Serial.println("Sending KEY_MEDIA_VOLUME_DOWN...");
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      Serial.println("Sending KEY_MEDIA_VOLUME_DOWN Complete!");
      delay(100);
      break;
    default:
      break;
    }
  }

  if (millis() - lastActivityTime > inactivityThreshold)
  {
    Serial.println("Go to DeepSleep!!");
    delay(1000);            // 少し待ってからスリープ
    esp_deep_sleep_start(); // ディープスリープに入る
  }
}