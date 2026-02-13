#ifndef MARKET_PAGE_H
#define MARKET_PAGE_H

#include "../Page.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

struct MarketAsset {
    String symbol;
    String name;
    double price;
    double change;
    double changePercent;
    bool isValid = false;
};

class MarketPage : public Page {
public:
    void setup(TFT_eSPI* tft) override;
    void loop() override;
    void draw() override;
    const char* getName() override { return "Market"; }

private:
    TFT_eSPI* _tft;
    unsigned long _lastUpdate = 0;
    const unsigned long _interval = 10000; // 10s 更新一次

    MarketAsset _assets[4]; 
    String _lastError = "None";
    int _updatingIndex = 0;
    
    void updateMarket();
    void drawCell(int idx, int x, int y, int w, int h);
};

#endif
