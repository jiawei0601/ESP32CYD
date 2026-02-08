#include "SettingsPage.h"

// External access to touch (from main.cpp or passed in setup, but we use global for now to keep it simple or fix main loop to pass touch)
// Ideally Page::loop() should take touch as argument, but for now we'll assume touch is handled or we use a global pointer if needed.
// Actually, main loop calls handleTouch() logic for page switching. We need page-specific touch.
// Let's rely on the fact that `touch` object is global in main.cpp, we can declare `extern`.
#include <XPT2046_Touchscreen.h>
extern XPT2046_Touchscreen touch;

void SettingsPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    prefs.begin("wifi-config", false); // Namespace "wifi-config"
    initKeyboard();
    scanNetworks();
}

void SettingsPage::loop() {
    handleTouch();
}

void SettingsPage::draw() {
    _tft->fillScreen(TFT_BLACK);
    
    if (_state == STATE_SCAN) {
        drawScanList();
    } else if (_state == STATE_INPUT_PASS) {
        drawKeyboard();
    } else if (_state == STATE_CONNECTING) {
        _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        _tft->drawCentreString("Connecting to:", 160, 80, 4);
        _tft->drawCentreString(_ssids[_selectedNetworkIndex], 160, 120, 4);
        _tft->drawCentreString("Please Wait...", 160, 160, 4);
    }
}

void SettingsPage::scanNetworks() {
    _state = STATE_SCAN;
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawCentreString("Scanning WiFi...", 160, 120, 4);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    _networkCount = WiFi.scanNetworks();
    
    _ssids.clear();
    for (int i = 0; i < _networkCount && i < 6; ++i) { // Limit to 6
        _ssids.push_back(WiFi.SSID(i));
    }
    draw();
}

void SettingsPage::drawScanList() {
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawCentreString("Select WiFi Network", 160, 10, 2);
    _tft->drawFastHLine(0, 30, 320, TFT_BLUE);

    for (int i = 0; i < _ssids.size(); i++) {
        int y = 40 + i * 35;
        _tft->drawRect(10, y, 300, 30, TFT_DARKGREY);
        _tft->drawString(_ssids[i], 20, y + 5, 2);
        
        // Signal strength bar (mock or real if we saved RSSI)
        // _tft->drawString(String(WiFi.RSSI(i)), 280, y+5, 2);
    }
    
    // Rescan Button
    _tft->fillRect(100, 210, 120, 25, TFT_DARKGREEN);
    _tft->setTextColor(TFT_WHITE);
    _tft->drawCentreString("Rescan", 160, 215, 2);
}

void SettingsPage::initKeyboard() {
    _keys.clear();
    int startX = 5;
    int startY = 80;
    int keyW = 30;
    int keyH = 30;
    int gap = 2;
    
    const char* rows[] = {
        "1234567890",
        "QWERTYUIOP",
        "ASDFGHJKL",
        "ZXCVBNM"
    };
    
    // Row 1-4
    for (int r = 0; r < 4; r++) {
        int len = strlen(rows[r]);
        int rowX = startX + (320 - (len * (keyW + gap))) / 2;
        for (int c = 0; c < len; c++) {
            Key k;
            k.x = rowX + c * (keyW + gap);
            k.y = startY + r * (keyH + gap);
            k.w = keyW; k.h = keyH;
            k.value = rows[r][c];
            sprintf(k.label, "%c", k.value);
            _keys.push_back(k);
        }
    }
    
    // Special Keys
    Key kBack = {240, 200, 70, 30, "<-", 8};   // Backspace
    Key kEnter = {10, 200, 70, 30, "OK", 13};  // Enter
    Key kCancel = {100, 200, 70, 30, "X", 27}; // ESC/Cancel
    
    _keys.push_back(kBack);
    _keys.push_back(kEnter);
    _keys.push_back(kCancel);
}

void SettingsPage::drawKeyboard() {
    _tft->fillScreen(TFT_BLACK);
    
    // Input Box
    _tft->drawRect(10, 40, 300, 30, TFT_WHITE);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString(_password, 15, 45, 4);
    
    // Keys
    _tft->setTextDatum(MC_DATUM);
    for (auto &k : _keys) {
        _tft->fillRect(k.x, k.y, k.w, k.h, TFT_DARKGREY);
        _tft->drawRect(k.x, k.y, k.w, k.h, TFT_WHITE);
        _tft->setTextColor(TFT_WHITE);
        _tft->drawString(k.label, k.x + k.w/2, k.y + k.h/2, 2);
    }
}

void SettingsPage::connectWiFi() {
    _state = STATE_CONNECTING;
    draw();
    
    WiFi.begin(_ssids[_selectedNetworkIndex].c_str(), _password.c_str());
    
    unsigned long start = millis();
    bool connected = false;
    while(millis() - start < 15000) {
        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            break;
        }
        delay(100);
    }
    
    if (connected) {
        prefs.putString("ssid", _ssids[_selectedNetworkIndex]);
        prefs.putString("pass", _password);
        _tft->fillScreen(TFT_GREEN);
        _tft->setTextColor(TFT_BLACK);
        _tft->drawCentreString("Connected!", 160, 120, 4);
        delay(2000);
        // Maybe return to main page?
        scanNetworks(); // Go back to list
    } else {
        _tft->fillScreen(TFT_RED);
        _tft->setTextColor(TFT_WHITE);
        _tft->drawCentreString("Failed!", 160, 120, 4);
        delay(2000);
        _state = STATE_INPUT_PASS; // Retry password
        draw();
    }
}

void SettingsPage::handleTouch() {
    if (touch.touched()) {
        TS_Point p = touch.getPoint();
        
        // Calibration (Same as main.cpp, should be unified)
        int tx = map(p.x, 350, 3750, 320, 0); 
        int ty = map(p.y, 250, 3850, 240, 0); 
        tx = constrain(tx, 0, 319); 
        ty = constrain(ty, 0, 239);
        
        if (_state == STATE_SCAN) {
             // Check list items
             for (int i = 0; i < _ssids.size(); i++) {
                 int y = 40 + i * 35;
                 if (tx > 10 && tx < 310 && ty > y && ty < y + 30) {
                     _selectedNetworkIndex = i;
                     _password = "";
                     _state = STATE_INPUT_PASS;
                     draw();
                     delay(300);
                     return;
                 }
             }
             // Check Rescan Buttom (Simplified hit check)
             if (ty > 210 && ty < 235 && tx > 100 && tx < 220) {
                 scanNetworks();
             }
             
        } else if (_state == STATE_INPUT_PASS) {
            for (auto &k : _keys) {
                if (tx >= k.x && tx <= k.x + k.w && ty >= k.y && ty <= k.y + k.h) {
                    if (k.value == 8) { // Backspace
                        if (_password.length() > 0) _password.remove(_password.length()-1);
                    } else if (k.value == 13) { // Enter
                        connectWiFi();
                        return; 
                    } else if (k.value == 27) { // Cancel
                        _state = STATE_SCAN;
                        draw();
                        return;
                    } else {
                        _password += k.value;
                    }
                    // Redraw Input Box only to avoid flicker? 
                    // No, simple enough to full redraw or just box.
                    // Full redraw for simplicity
                    drawKeyboard();
                    delay(200); // Debounce
                    return;
                }
            }
        }
    }
}
