#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include "Config.h"
#include "Page.h"

// Page Includes
#include "pages/WeatherPage.h"
#include "pages/StockPage.h"
#include "pages/MarketPage.h"
#include "pages/AlbumPage.h"
#include "pages/SettingsPage.h"
#include <Preferences.h>

// Hardware
TFT_eSPI tft = TFT_eSPI();
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// Pages
const int PAGE_COUNT = 5;
Page* pages[PAGE_COUNT];
int currentPage = 0;

void switchPage(int index);

// UI
void drawTopBar() {
    tft.fillRect(0, 0, 320, 24, TFT_NAVY);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    for(int i=0; i<PAGE_COUNT; i++) {
        int w = 320 / PAGE_COUNT;
        int x = i * w;
        if(i == currentPage) tft.fillRect(x, 0, w, 24, TFT_BLUE);
        // tft.drawString(pages[i]->getName(), x + w/2, 12, 2);
        char label[2]; sprintf(label, "%d", i+1);
        tft.drawString(label, x + w/2, 12, 2);
    }
}

    // Display Init
    tft.init();
    tft.setRotation(1); 
    tft.invertDisplay(true); 
    tft.fillScreen(TFT_BLACK);

    // Touch Init
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    // WiFi from Preferences
    Preferences prefs;
    prefs.begin("wifi-config", true); // Read-only
    String savedSSID = prefs.getString("ssid", "");
    String savedPass = prefs.getString("pass", "");
    prefs.end();

    if (savedSSID.length() > 0) {
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());
        tft.drawString("Connecting " + savedSSID, 160, 120, 2);
        
        // Timeout for WiFi
        int limit = 0;
        while (WiFi.status() != WL_CONNECTED && limit < 20) {
            delay(500);
            Serial.print(".");
            limit++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi Connected");
            tft.drawString("Connected!", 160, 140, 2);
            delay(1000);
        } else {
             tft.drawString("WiFi Failed", 160, 140, 2);
             delay(1000);
        }
    } else {
        tft.drawString("No WiFi Configured", 160, 120, 2);
        delay(1000);
    }

    // Initialize Pages
    pages[2] = new WeatherPage(); 
    pages[3] = new AlbumPage();
    pages[1] = new StockPage();
    pages[0] = new MarketPage(); 
    pages[4] = new SettingsPage();

    for(int i=0; i<PAGE_COUNT; i++) {
        pages[i]->setup(&tft);
    }
    
    // Default to Settings if no WiFi, else Market
    if (WiFi.status() != WL_CONNECTED) {
        switchPage(4);
    } else {
        switchPage(0);
    }
}

void loop() {
    if (touch.touched()) {
        TS_Point p = touch.getPoint();
        // CYD Rotation 1 Calibration
        int tx = map(p.x, 350, 3750, 320, 0); 
        int ty = map(p.y, 250, 3850, 240, 0); 
        tx = constrain(tx, 0, 319); 
        ty = constrain(ty, 0, 239);
        
        if (ty < 40) { // Top Bar Touch
             int newPage = tx / (320 / PAGE_COUNT);
             if (newPage != currentPage && newPage < PAGE_COUNT) switchPage(newPage);
             delay(200); 
        }
    }
    
    pages[currentPage]->loop();
    delay(10);
}

void switchPage(int index) {
    if(index < 0 || index >= PAGE_COUNT) return;
    currentPage = index;
    // pages[currentPage]->draw();
    drawTopBar();
}
