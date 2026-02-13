#include "TickerPage.h"
#include "../Style.h"

void TickerPage::setup(TFT_eSPI *tft) {
  _tft = tft;
  _asset.symbol = "NVDA";
  _asset.name = "NVIDIA";
  updateTicker();
}

void TickerPage::loop() {
  if (_lastUpdate == 0 || millis() - _lastUpdate > _interval) {
    updateTicker();
    _lastUpdate = millis();
    draw();
  }
}

void TickerPage::draw() {
  // 保留頂部導航欄 (Y=0~24)
  _tft->fillRect(0, 25, 320, 215, TFT_BLACK);

  // 繪製背景裝飾 (類似 Tickertronix 的簡潔風格)
  _tft->drawFastHLine(0, 25, 320, TT_BORDER_COLOR);

  if (!_asset.isValid) {
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawCentreString("FETCHING TICKER...", 160, 120, 4);
    _tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
    _tft->drawCentreString(_lastError, 160, 160, 2);
    return;
  }

  // 1. Symbol (左上角)
  _tft->setTextDatum(TL_DATUM);
  _tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  _tft->drawString(_asset.symbol, 15, 45, 4);

  // 2. Price (中央超大字體)
  uint32_t trendColor = (_asset.change >= 0) ? TFT_RED : TFT_GREEN;
  _tft->setTextDatum(MC_DATUM);
  _tft->setTextColor(TFT_WHITE, TFT_BLACK);

  char priceStr[20];
  sprintf(priceStr, "%.2f", _asset.price);
  _tft->drawString(priceStr, 160, 120, 7); // 使用 7 號大字體

  // 3. Change Badge (右下角或下方)
  // 繪製圓角矩形背景 (Tickertronix 標誌性設計)
  int badgeW = 120;
  int badgeH = 40;
  int badgeX = 160 - (badgeW / 2);
  int badgeY = 185;

  _tft->fillRoundRect(badgeX, badgeY, badgeW, badgeH, 8, trendColor);

  _tft->setTextColor(TFT_WHITE, trendColor);
  char changeStr[40];
  sprintf(changeStr, "%+.2f%%", _asset.changePercent);
  _tft->drawCentreString(changeStr, 160, badgeY + 10, 4);

  _tft->setTextDatum(TL_DATUM); // 重置基準點
}

void TickerPage::updateTicker() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://brapi.dev/api/quote/" + _asset.symbol;

    http.begin(url);
    http.setUserAgent("ESP32-Ticker-Retro");
    http.setTimeout(5000);

    int httpCode = http.GET();
    if (httpCode == 200) {
      JsonDocument doc;
      deserializeJson(doc, http.getString());
      if (doc["results"].size() > 0) {
        JsonObject data = doc["results"][0];
        _asset.price = data["regularMarketPrice"];
        if (!data["regularMarketChangePercent"].isNull()) {
          _asset.changePercent = data["regularMarketChangePercent"];
          _asset.change = data["regularMarketChange"];
        } else {
          _asset.changePercent = data["changePercent"];
          _asset.change = data["change"];
        }
        _asset.isValid = true;
        _lastError = "OK";
      }
    } else {
      // Yahoo 備用
      String backupUrl = "https://query1.finance.yahoo.com/v8/finance/chart/" +
                         _asset.symbol + "?interval=1d&range=1d";
      http.begin(backupUrl);
      http.setTimeout(4000);
      int bCode = http.GET();
      if (bCode == 200) {
        JsonDocument doc;
        deserializeJson(doc, http.getString());
        if (doc["chart"]["result"].size() > 0) {
          JsonObject meta = doc["chart"]["result"][0]["meta"];
          _asset.price = meta["regularMarketPrice"];
          double prevClose = meta["previousClose"];
          if (prevClose == 0)
            prevClose = meta["chartPreviousClose"];
          _asset.change = _asset.price - prevClose;
          _asset.changePercent =
              (prevClose != 0) ? (_asset.change / prevClose) * 100.0 : 0;
          _asset.isValid = true;
          _lastError = "Backup";
        }
      } else {
        _lastError = "Err:" + String(httpCode);
      }
    }
    http.end();
  }
}
