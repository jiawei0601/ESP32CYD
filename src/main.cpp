#include "Config.h"
#include "Page.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <XPT2046_Touchscreen.h>


// Page Includes
#include "pages/MarketPage.h"
#include "pages/SettingsPage.h"
#include "pages/StockPage.h"
#include "pages/TickerPage.h" // 替換相簿為新的 Ticker 頁面
#include "pages/WeatherPage.h"
#include <Preferences.h>


// Hardware
TFT_eSPI tft = TFT_eSPI();
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass touchSPI = SPIClass(VSPI);
// XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);
XPT2046_Touchscreen touch(XPT2046_CS); // 移除 IRQ 引腳以提高穩定性

// Pages
const int PAGE_COUNT = 5;
Page *pages[PAGE_COUNT];
int currentPage = 0;

void switchPage(int index);

// UI
void drawTopBar() {
  tft.fillRect(0, 0, 320, 24, TFT_NAVY);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  for (int i = 0; i < PAGE_COUNT; i++) {
    int w = 320 / PAGE_COUNT;
    int x = i * w;
    if (i == currentPage)
      tft.fillRect(x, 0, w, 24, TFT_BLUE);
    // tft.drawString(pages[i]->getName(), x + w/2, 12, 2);
    char label[2];
    sprintf(label, "%d", i + 1);
    tft.drawString(label, x + w / 2, 12, 2);
  }
}

void setup() {
  Serial.begin(115200);

  // Display Init
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH); // 打開背光
  tft.init();
  tft.setRotation(3);
  tft.invertDisplay(true); // 強制反相修正，使背景變黑
  tft.fillScreen(TFT_BLACK);

  // Touch Init
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touch.begin(touchSPI);
  touch.setRotation(3); // 觸控同步旋轉 3
  Serial.println("Touch Initialized");

  // WiFi from Preferences
  Preferences prefs;
  prefs.begin("wifi-config", true); // Read-only
  String savedSSID = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  prefs.end();

  if (savedSSID.length() == 0) {
    savedSSID = WIFI_SSID;
    savedPass = WIFI_PASS;
  }

  // WiFi Init (Wait for connection to avoid 0 values)
  if (savedSSID.length() > 0 && String(savedSSID) != "Your_SSID") {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString("WiFi Connecting...", 160, 100, 2);

    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    int limit = 0;
    while (WiFi.status() != WL_CONNECTED && limit < 20) {
      delay(500);
      limit++;
    }
  }

  // Initialize Pages
  pages[2] = new WeatherPage();
  pages[3] = new TickerPage(); // 這裡由 AlbumPage 更換為 TickerPage
  pages[1] = new StockPage();
  pages[0] = new MarketPage();
  pages[4] = new SettingsPage();

  for (int i = 0; i < PAGE_COUNT; i++) {
    pages[i]->setup(&tft);
  }

  // Always start at Market Page
  switchPage(0);
}

void loop() {
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > 10000) { // 每 10 秒檢查一次
    lastWiFiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi lost, reconnecting...");
      // 嘗試從 Preferences 取得帳密
      Preferences prefs;
      prefs.begin("wifi-config", true);
      String savedSSID = prefs.getString("ssid", "");
      String savedPass = prefs.getString("pass", "");
      prefs.end();

      if (savedSSID.length() > 0) {
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());
      }
    }
  }

  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    int tx = map(p.x, 3550, 350, 0, 320);
    int ty = map(p.y, 3750, 350, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    if (ty < 30) {
      int newPage = tx / (320 / PAGE_COUNT);
      if (newPage != currentPage && newPage < PAGE_COUNT) {
        switchPage(newPage);
        delay(300); // 只在切換時保持短暫停頓
        return;     // 切換後立即結束本次 loop，以便進入新頁面繪製
      }
    } else {
      // 設定頁面的特殊觸控傳遞保持不變
      if (currentPage == 4)
        pages[4]->loop();
    }
  }

  // 執行當前頁面的 loop
  if (pages[currentPage])
    pages[currentPage]->loop();
  delay(5); // 縮短 delay 以提升響應頻率
}

void switchPage(int index) {
  if (index < 0 || index >= PAGE_COUNT)
    return;
  currentPage = index;
  tft.fillScreen(TFT_BLACK); // Clear once on switch
  // pages[currentPage]->draw(); // Optimisation: Let loop handle draw or call
  // draw explicitly
  drawTopBar();
  pages[currentPage]->draw(); // Explicitly draw the new page
}
