#include "MarketPage.h"
#include "../Style.h"

void MarketPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    _assets[0].symbol = "^IXIC";   _assets[0].name = "NASDAQ";
    _assets[1].symbol = "NVDA";    _assets[1].name = "NVDA";
    _assets[2].symbol = "GOOGL";   _assets[2].name = "GOOGLE";
    _assets[3].symbol = "BTC-USD"; _assets[3].name = "BTC";
    // 初次載入數據
    updateMarket();
}

void MarketPage::loop() {
    // 異步抓取：每 4 秒只更新一個項目，確保介面不卡頓
    if (_lastUpdate == 0 || millis() - _lastUpdate > 4000) {
        updateMarket();
        _lastUpdate = millis();
    }
}

void MarketPage::draw() {
    // 局部刷新，保留頂部導覽欄
    _tft->fillRect(0, 25, 320, 215, TFT_BLACK);
    _tft->setTextColor(TT_CYAN, TFT_BLACK);
    _tft->drawCentreString("MARKET TICKER", 160, 32, 2);
    _tft->drawFastHLine(0, 48, 320, TT_BORDER_COLOR);

    int w = 158;
    int h = 88;
    drawCell(0, 1, 52, w, h);
    drawCell(1, 161, 52, w, h);
    drawCell(2, 1, 142, w, h);
    drawCell(3, 161, 142, w, h);
}

void MarketPage::drawCell(int idx, int x, int y, int w, int h) {
    MarketAsset& a = _assets[idx];
    _tft->drawRect(x, y, w, h, TT_BORDER_COLOR);
    
    if (!a.isValid) {
        _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        _tft->drawCentreString("Loading...", x + w/2, y + h/2 - 12, 2);
        _tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
        _tft->drawCentreString(_lastError, x + w/2, y + h/2 + 8, 1);
        return;
    }

    uint32_t trendColor = (a.change >= 0) ? TFT_RED : TFT_GREEN;
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawCentreString(a.name, x + w/2, y + 10, 4);
    _tft->setTextColor(trendColor, TFT_BLACK);
    char priceStr[20];
    sprintf(priceStr, "%.2f", a.price);
    _tft->drawCentreString(priceStr, x + w/2, y + 42, 4);
    char changeStr[40];
    sprintf(changeStr, "%+.2f (%.1f%%)", a.change, a.changePercent);
    _tft->drawCentreString(changeStr, x + w/2, y + 74, 2);
}

void MarketPage::updateMarket() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        int i = _updatingIndex;
        String sym = _assets[i].symbol;
        
        // 主 API 路徑 (Brapi)
        String url = "https://brapi.dev/api/quote/" + sym; 
        if (sym == "^IXIC") url = "https://brapi.dev/api/quote/%5EIXIC"; 

        http.begin(url);
        http.setUserAgent("ESP32-Ticker");
        http.setTimeout(4000); 
        
        int httpCode = http.GET();
        if (httpCode == 200) {
            JsonDocument doc;
            deserializeJson(doc, http.getString());
            if (doc["results"].size() > 0) {
                JsonObject data = doc["results"][0];
                _assets[i].price = data["regularMarketPrice"];
                if (!data["regularMarketChange"].isNull()) {
                    _assets[i].change = data["regularMarketChange"];
                    _assets[i].changePercent = data["regularMarketChangePercent"];
                } else if (!data["change"].isNull()) {
                    _assets[i].change = data["change"];
                    _assets[i].changePercent = data["changePercent"];
                }
                _assets[i].isValid = true;
                _lastError = "OK";
                draw(); 
            }
        } else {
            // 備援 API 路徑 (Yahoo Finance Chart)
            String backupUrl = "https://query1.finance.yahoo.com/v8/finance/chart/" + sym + "?interval=1d&range=1d";
            http.begin(backupUrl);
            http.setTimeout(3000);
            int bCode = http.GET();
            if (bCode == 200) {
                JsonDocument doc;
                deserializeJson(doc, http.getString());
                if (doc["chart"]["result"].size() > 0) {
                    JsonObject meta = doc["chart"]["result"][0]["meta"];
                    _assets[i].price = meta["regularMarketPrice"];
                    double prevClose = meta["previousClose"];
                    if (prevClose == 0) prevClose = meta["chartPreviousClose"];
                    _assets[i].change = _assets[i].price - prevClose;
                    _assets[i].changePercent = (prevClose != 0) ? (_assets[i].change / prevClose) * 100.0 : 0;
                    _assets[i].isValid = true;
                    _lastError = "B-OK";
                    draw();
                }
            } else {
                _lastError = "E:" + String(httpCode);
            }
        }
        http.end();
        // 循環更新下一個項目
        _updatingIndex = (_updatingIndex + 1) % 4;
    }
}
