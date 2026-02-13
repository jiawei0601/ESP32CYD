#ifndef TICKER_PAGE_H
#define TICKER_PAGE_H

#include "../Page.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>


struct TickerAsset {
  String symbol;
  String name;
  double price;
  double change;
  double changePercent;
  bool isValid = false;
};

class TickerPage : public Page {
public:
  void setup(TFT_eSPI *tft) override;
  void loop() override;
  void draw() override;
  const char *getName() override { return "Ticker"; }

private:
  TFT_eSPI *_tft;
  TickerAsset _asset;
  unsigned long _lastUpdate = 0;
  const unsigned long _interval = 10000; // 10s refresh
  String _lastError = "None";

  void updateTicker();
};

#endif
