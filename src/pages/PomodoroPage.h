#ifndef POMODORO_PAGE_H
#define POMODORO_PAGE_H

#include "../Page.h"

enum PomodoroState { P_WORK, P_BREAK };

class PomodoroPage : public Page {
public:
  void setup(TFT_eSPI *tft) override;
  void loop() override;
  void draw() override;
  const char *getName() override { return "Pomodoro"; }

private:
  TFT_eSPI *_tft;

  // Timer Logic
  uint32_t _timerMs = 25 * 60; // Default 25 min in seconds
  uint32_t _remainingS = 25 * 60;
  bool _isRunning = false;
  PomodoroState _mode = P_WORK;
  unsigned long _lastTick = 0;

  void handleButtons();
  void drawInterface();
  void drawButtons();
  void toggleTimer();
  void resetTimer();
  void switchMode();
};

#endif
