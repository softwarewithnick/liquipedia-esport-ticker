/*
 * Liquipedia-Public-LED - Tournament display for 64x32 RGB LED matrix
 * Uses task-based display refresh (not timer ISR) to avoid xQueueSemaphoreTake assert.
 */

#include "config.h"
#include "game_logos.h"
#include "data_provider.h"
#include <vector>
#include <algorithm>
#include <PxMatrix.h>
#include <GFX_Layer.hpp>

#if WIFI_USE_MANAGER
#include <WiFiManager.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16

#define matrix_width 64
#define matrix_height 32

uint8_t display_draw_time = 60;

PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

#ifdef ESP32
TaskHandle_t displayTask;
void refreshDisplay(void* parameter) {
  while (true) {
    display.display(display_draw_time);
    delay(3);
  }
}
#endif

unsigned long isAnimationDue;
int delayBetweeenAnimations = 30;
int textXPosition = 64;
int textYPosition = 32 - 8;

int16_t xOne, yOne;
uint16_t w, h;

void layer_draw_callback(int16_t x, int16_t y, uint8_t r_data, uint8_t g_data, uint8_t b_data) {
  display.drawPixelRGB888(x, y, r_data, g_data, b_data);
}

GFX_Layer gfx_layer_fg(64, 32, layer_draw_callback);

uint16_t myRED, myGREEN, myWHITE, myPURPLE, myBLACK, myBLUE;
uint16_t myCOLOR;

uint16_t colors[] = {
  display.color565(255, 255, 255),
  display.color565(0, 0, 255),
  display.color565(255, 0, 0),
  display.color565(0, 255, 0),
  display.color565(128, 0, 128),
  display.color565(255, 165, 0)
};

std::vector<String> all_tournaments;
std::vector<String> all_tiers;
std::vector<String> all_prize_pools;
std::vector<String> upcoming_date;
std::vector<String> tier_filters;
std::vector<String> all_games;

int cyclesCount = CYCLES;

void parseCommaList(const char* str, std::vector<String>& out) {
  out.clear();
  String s = String(str);
  int start = 0;
  int pos;
  while ((pos = s.indexOf(',', start)) >= 0) {
    String part = s.substring(start, pos);
    part.trim();
    if (part.length() > 0) out.push_back(part);
    start = pos + 1;
  }
  String part = s.substring(start);
  part.trim();
  if (part.length() > 0) out.push_back(part);
}

void scrollText(const char* text) {
  const char* str = text;
  byte offSet = 25;
  unsigned long now = millis();
  if (now > isAnimationDue) {
    gfx_layer_fg.setTextSize(1);
    isAnimationDue = now + delayBetweeenAnimations;
    textXPosition -= 1;

    uint16_t textW, textH;
    gfx_layer_fg.getTextBounds(str, textXPosition, textYPosition, &xOne, &yOne, &textW, &textH);
    if (textXPosition + (int)textW <= -1) {
      textXPosition = gfx_layer_fg.width() + offSet;
    }

    display.setCursor(textXPosition, textYPosition);
    display.fillRect(0, 23, 64, 9, myBLACK);

    for (size_t idx = 0; idx < strlen(str); idx++) {
      display.setTextColor(myWHITE);
      display.print(str[idx]);
    }
  }
}

void showTournaments(String game, String temporal) {
  Serial.println("Beginning " + temporal + " tournaments for " + game);

  display.clearDisplay();
  display.setTextColor(myRED);

  String temporal_string = temporal;
  String upcoming_tourneys = "Tourneys";

  display.setCursor((64 - temporal_string.length() * 6) / 2, 2);
  display.print(temporal_string);
  display.setCursor((64 - upcoming_tourneys.length() * 6) / 2, 10);


  if (game == "halo") {
    display.drawRGBBitmap(25, 18, halo_logo_small, 14, 14);
  } else if (game == "ageofempires") {
    display.drawRGBBitmap(25, 18, aoe2_logo_small, 14, 14);
  }

  delay(2000);

  if (all_tournaments.size() == 0 || all_tournaments[0] == "None") {
    display.clearDisplay();
    display.setTextColor(myRED);
    display.setCursor(4, 12);
    display.print("No tournaments");
    delay(3000);
    return;
  }

  int displayed_tourneys = 0;
  for (size_t i = 0; i < all_tournaments.size() && displayed_tourneys < 4; i++) {
    if (i >= all_tiers.size() || i >= all_prize_pools.size() || i >= upcoming_date.size()) break;

    if (std::find(tier_filters.begin(), tier_filters.end(), all_tiers[i]) == tier_filters.end()) {
      continue;
    }

    display.clearDisplay();
    displayed_tourneys++;

    String tier_string = "Tier: " + all_tiers[i];
    String prize_string = all_prize_pools[i];
    String upcoming_match_date = upcoming_date[i];
    String upcoming_tourn = "League: ";
    String stats_string[] = { tier_string, upcoming_tourn };

    display.setTextSize(1);
    display.setTextColor(myWHITE);

    if (game == "halo") {
      display.drawRGBBitmap(1, 1, halo_logo, 20, 20);
    } else if (game == "ageofempires") {
      display.drawRGBBitmap(1, 1, aoe2_logo, 20, 20);
    }

    if (game == "halo" || game == "ageofempires") {
      display.setCursor(((44 - upcoming_match_date.length() * 6) / 2) + 20, 2);
      display.print(upcoming_match_date);
      display.setCursor(((44 - prize_string.length() * 6) / 2) + 20, 11);
      display.setTextColor(myGREEN);
      display.print(prize_string);
    }

    int z = 0;
    while (z < cyclesCount) {
      for (int j = 0; j < 2; j++) {
        display.fillRect(0, 24, 64, 8, myBLACK);
        display.setCursor((64 - stats_string[j].length() * 6) / 2, 24);

        if (stats_string[j] == "League: ") {
          display.setTextColor(myWHITE);
          textXPosition = 64;

          if (all_tournaments[i].length() >= 10) {
            display.setTextWrap(false);
            String testing_scroll = all_tournaments[i];
            const char* t_scroll = testing_scroll.c_str();

            uint16_t textWidth, textHeight;
            gfx_layer_fg.getTextBounds(testing_scroll, textXPosition, textYPosition, &xOne, &yOne, &textWidth, &textHeight);

            while (textXPosition + (int)textWidth > 0) {
              scrollText(t_scroll);
              delay(18);
            }
            display.fillRect(0, 23, 64, 9, myBLACK);
          } else {
            display.setTextColor(myCOLOR);
            display.setCursor((64 - all_tournaments[i].length() * 6) / 2, 24);
            display.print(all_tournaments[i]);
            delay(3000);
          }
        } else {
          display.setTextColor(myCOLOR);
          display.print(stats_string[j]);
          delay(3000);
        }
      }
      z++;
    }
  }
}

void setup() {
  Serial.begin(9600);

  display.begin(16);

  myRED = display.color565(255, 0, 0);
  myGREEN = display.color565(0, 255, 0);
  myBLUE = display.color565(0, 0, 255);
  myWHITE = display.color565(255, 255, 255);
  myPURPLE = display.color565(128, 0, 128);
  myBLACK = display.color565(0, 0, 0);
  myCOLOR = colors[COLOR_INDEX >= 0 && COLOR_INDEX < 6 ? COLOR_INDEX : 3];

  gfx_layer_fg.clear();
  gfx_layer_fg.setTextWrap(false);

  parseCommaList(GAMES, all_games);
  parseCommaList(TIER_FILTER, tier_filters);
  cyclesCount = CYCLES;
  if (cyclesCount < 1) cyclesCount = 1;
  if (cyclesCount > 10) cyclesCount = 10;

  display.clearDisplay();
  display.setTextColor(myRED);
  display.setTextSize(1);
  display.setCursor(2, 12);
  display.print("Connecting...");

#ifdef ESP32
  xTaskCreatePinnedToCore(refreshDisplay, "DisplayRefresh", 1000, NULL, 1, &displayTask, 1);
#endif

#if WIFI_USE_MANAGER
  WiFiManager wifiManager;
  wifiManager.setConnectTimeout(20);
  if (!wifiManager.autoConnect("TournamentLED-Setup")) {
    Serial.println("WiFi connect failed. Restarting...");
    delay(3000);
    ESP.restart();
  }
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    attempts++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connect failed.");
    display.clearDisplay();
    display.setCursor(2, 12);
    display.print("WiFi failed!");
    delay(5000);
    ESP.restart();
  }
#endif

  Serial.println("Connected. IP: " + WiFi.localIP().toString());
}

void loop() {
  for (size_t g = 0; g < all_games.size(); g++) {
    String game = all_games[g];
    game.trim();

    loadTournamentData(game, "Upcoming", all_tournaments, all_tiers, all_prize_pools, upcoming_date);
    showTournaments(game, "Upcoming");

    loadTournamentData(game, "Live", all_tournaments, all_tiers, all_prize_pools, upcoming_date);
    showTournaments(game, "Live");
  }

  delay(5000);
}
