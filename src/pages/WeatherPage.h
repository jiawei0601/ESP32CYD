#ifndef WEATHER_PAGE_H
#define WEATHER_PAGE_H

#include "../Page.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../Config.h"

struct DailyForecast {
    String day;
    float min;
    float max;
    int code;
};

struct HourlyForecast {
    String time;
    float temp;
    int code;
};

class WeatherPage : public Page {
public:
    void setup(TFT_eSPI* tft) override;
    void loop() override;
    void draw() override;
    const char* getName() override { return "Weather"; }

private:
    TFT_eSPI* _tft;
    unsigned long _lastUpdate = 0;
    const unsigned long _interval = 600000; // 10 minutes
    String _city;
    String _weatherDesc;
    float _temp;
    float _feelsLike;
    int _humidity;
    float _windSpeed;
    String _lastSavedCity = ""; 
    
    DailyForecast _daily[5];
    HourlyForecast _hourly[5];
    
    void updateWeather();
    String getWeatherText(int code);
};

#endif
