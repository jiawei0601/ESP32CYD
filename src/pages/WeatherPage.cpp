#include "WeatherPage.h"
#include <Preferences.h>
#include "../Style.h"

void WeatherPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    updateWeather();
}

void WeatherPage::loop() {
    // 檢查是否有新的城市設定 (每秒檢查一次)
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 2000) {
        lastCheck = millis();
        Preferences prefs;
        prefs.begin("weather_v3", true);
        String currentSafeCity = prefs.getString("city", "Taipei");
        prefs.end();
        
        if (currentSafeCity != _lastSavedCity) {
            updateWeather();
            draw();
        }
    }

    if (millis() - _lastUpdate > _interval) {
        _lastUpdate = millis(); // 立即更新時間戳記，防止失敗時無限重刷繪圖
        updateWeather(); 
        draw(); 
    }
}

String WeatherPage::getWeatherText(int code) {
    switch (code) {
        case 0: return "晴朗";
        case 1: return "晴時多雲";
        case 2: return "局部多雲";
        case 3: return "陰天";
        case 45: case 48: return "有霧";
        case 51: case 53: case 55: return "細雨";
        case 61: case 63: case 65: return "陣雨";
        case 71: case 73: case 75: return "下雪";
        case 95: case 96: case 99: return "雷陣雨";
        default: return "雲量多";
    }
}

void WeatherPage::draw() {
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TT_CYAN, TFT_BLACK);
    _tft->drawCentreString("WEATHER STATION", 160, 5, 2);
    _tft->drawFastHLine(0, 25, 320, TT_BORDER_COLOR);

    _tft->setTextDatum(TC_DATUM);

    if (_lastUpdate == 0) {
        _tft->setTextColor(TFT_RED, TFT_BLACK);
        _tft->drawString("Loading Data...", 160, 100, 4);
        _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        _tft->drawString(_weatherDesc, 160, 140, 2); // 顯示錯誤訊息如 HTTP -1
    } else {
        _tft->setTextColor(TFT_CYAN, TFT_BLACK);
        _tft->drawString(_city, 160, 40, 4);
        
        _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        _tft->drawString(_weatherDesc, 160, 75, 2);
        
        char tempStr[20];
        sprintf(tempStr, "%.1f C", _temp);
        _tft->setTextColor(TFT_YELLOW, TFT_BLACK);
        _tft->drawString(tempStr, 130, 100, 7);

        _tft->setTextDatum(TL_DATUM);
        _tft->setTextColor(TFT_LIGHTGREY);
        _tft->setCursor(215, 95);
        _tft->print("Feels: "); _tft->print(_feelsLike, 1);
        _tft->setCursor(215, 120);
        _tft->print("Wind: "); _tft->print(_windSpeed, 1);
    }

    _tft->drawFastHLine(10, 210, 300, TT_BORDER_COLOR);
    _tft->setTextDatum(TC_DATUM);
    _tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
    _tft->drawString("Open-Meteo Stable Sync", 160, 220, 1);
}

void WeatherPage::updateWeather() {
    Preferences prefs;
    prefs.begin("weather_v3", true); 
    float lat = prefs.getFloat("lat", 25.03); 
    float lon = prefs.getFloat("lon", 121.56);
    _city = prefs.getString("city", "Taipei");
    _lastSavedCity = _city;
    prefs.end();

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        // 使用最簡易的 URL 參數
        String url = "http://api.open-meteo.com/v1/forecast?latitude=" + String(lat, 2) + 
                     "&longitude=" + String(lon, 2) + 
                     "&current_weather=true&forecast_days=1";
        
        Serial.printf("[Weather] Updating Taipei...\n");
        
        http.begin(url);
        http.setTimeout(10000); 
        
        int httpCode = http.GET();
        if (httpCode == 200) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                _temp = doc["current_weather"]["temperature"];
                _windSpeed = doc["current_weather"]["windspeed"]; 
                int code = doc["current_weather"]["weathercode"];
                
                // 英文描述防止字體崩潰
                if (code == 0) _weatherDesc = "Clear Sky";
                else if (code <= 3) _weatherDesc = "Cloudy";
                else if (code >= 51 && code <= 67) _weatherDesc = "Rainy";
                else _weatherDesc = "Overcast";
                
                _feelsLike = _temp - 0.3;
                _lastUpdate = millis();
                Serial.println("[Weather] Taipei Loaded!");
            } else {
                _weatherDesc = "JSON Error";
            }
        } else {
            _weatherDesc = "HTTP Error: " + String(httpCode);
        }
        http.end();
    } else {
        _weatherDesc = "WiFi Disconnected";
    }
}
