#include "PomodoroPage.h"
#include <XPT2046_Touchscreen.h>

extern XPT2046_Touchscreen touch; // 引用 main.cpp 中的觸控實例

void PomodoroPage::setup(TFT_eSPI *tft) {
  _tft = tft;
  _remainingS = 25 * 60;
}

void PomodoroPage::loop() {
  // 1. 處理計時邏輯
  if (_isRunning) {
    if (millis() - _lastTick >= 1000) {
      _lastTick = millis();
      if (_remainingS > 0) {
        _remainingS--;
        drawInterface(); // 每一秒更新一次數字
      } else {
        _isRunning = false;
        switchMode(); // 時間到，切換模式
        draw();       // 全畫面刷新
      }
    }
  }

  // 2. 處理觸控 (簡化處理，具體座標需與 main.cpp 同步)
  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    int tx = map(p.x, 3550, 350, 0, 320);
    int ty = map(p.y, 3750, 350, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // 如果點擊區域在下方按鈕區
    if (ty > 160) {
      if (tx < 160) {
        toggleTimer(); // 左半部：開始/暫停
      } else {
        resetTimer(); // 右半部：重設
      }
      delay(200); // 簡單去彈跳
    }
  }
}

void PomodoroPage::draw() {
  _tft->fillRect(0, 25, 320, 215, TFT_BLACK);
  drawInterface();
  drawButtons();
}

void PomodoroPage::drawInterface() {
  _tft->setTextDatum(MC_DATUM);

  // 顯示模式標題 (工作/休息)
  _tft->setTextColor(TFT_WHITE, TFT_BLACK);
  _tft->fillRect(0, 30, 320, 40, (_mode == P_WORK) ? TFT_RED : TFT_BLUE);
  _tft->drawCentreString((_mode == P_WORK) ? "WORKING TIME" : "RELXING TIME",
                         160, 50, 4);

  // 顯示大計時器
  char timeStr[10];
  int mins = _remainingS / 60;
  int secs = _remainingS % 60;
  sprintf(timeStr, "%02d:%02d", mins, secs);

  _tft->setTextColor(TFT_WHITE, TFT_BLACK);
  _tft->fillRect(40, 80, 240, 80, TFT_BLACK); // 局部清除數字區
  _tft->drawCentreString(timeStr, 160, 120, 7);
}

void PomodoroPage::drawButtons() {
  // 左按鈕：開始/暫停
  uint32_t btnColor = _isRunning ? TFT_DARKGREY : TFT_ORANGE;
  _tft->fillRoundRect(20, 180, 130, 45, 8, btnColor);
  _tft->setTextColor(TFT_WHITE, btnColor);
  _tft->drawCentreString(_isRunning ? "PAUSE" : "START", 85, 202, 4);

  // 右按鈕：重設
  _tft->fillRoundRect(170, 180, 130, 45, 8, TFT_MAROON);
  _tft->setTextColor(TFT_WHITE, TFT_MAROON);
  _tft->drawCentreString("RESET", 235, 202, 4);
}

void PomodoroPage::toggleTimer() {
  _isRunning = !_isRunning;
  if (_isRunning)
    _lastTick = millis();
  drawButtons();
}

void PomodoroPage::resetTimer() {
  _isRunning = false;
  _remainingS = (_mode == P_WORK) ? (25 * 60) : (5 * 60);
  draw();
}

void PomodoroPage::switchMode() {
  if (_mode == P_WORK) {
    _mode = P_BREAK;
    _remainingS = 5 * 60;
  } else {
    _mode = P_WORK;
    _remainingS = 25 * 60;
  }
}
