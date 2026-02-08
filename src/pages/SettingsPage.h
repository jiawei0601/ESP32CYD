#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include "../Page.h"
#include <WiFi.h>
#include <Preferences.h>
#include <vector>

enum SettingsState {
    STATE_SCAN,
    STATE_INPUT_PASS,
    STATE_CONNECTING
};

struct Key {
    int x, y, w, h;
    char label[4];
    char value; 
};

class SettingsPage : public Page {
public:
    void setup(TFT_eSPI* tft) override;
    void loop() override;
    void draw() override;
    const char* getName() override { return "Settings"; }

private:
    TFT_eSPI* _tft;
    Preferences prefs;
    SettingsState _state = STATE_SCAN;
    
    // Scan Data
    int _networkCount = 0;
    std::vector<String> _ssids;
    int _selectedNetworkIndex = -1;
    
    // Password Input
    String _password = "";
    
    // UI Helpers
    void drawScanList();
    void drawKeyboard();
    void handleTouch();
    void connectWiFi();
    void scanNetworks();

    // Keyboard Layout
    std::vector<Key> _keys;
    void initKeyboard();
};

#endif
