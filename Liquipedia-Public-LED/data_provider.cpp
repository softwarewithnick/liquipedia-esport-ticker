/*
 * Firebase data provider - same paths as game_ticker_pxmatrix
 * Live:  live/game/{game}/live
 * Upcoming: notLive/game/{game}/upcoming
 */

#include "data_provider.h"
#include "config.h"
#include <Firebase.h>
#include <ArduinoJson.h>

static Firebase fb(FIREBASE_URL);

bool loadTournamentData(const String& game, const String& temporal,
  std::vector<String>& tournaments,
  std::vector<String>& tiers,
  std::vector<String>& prizePools,
  std::vector<String>& dates)
{
  tournaments.clear();
  tiers.clear();
  prizePools.clear();
  dates.clear();

  String path;
  if (temporal == "Live") {
    path = "live/game/" + game + "/live";
  } else {
    path = "notLive/game/" + game + "/upcoming";
  }

  Serial.print("Firebase fetch: ");
  Serial.println(path);

  String response = fb.getJson(path);

  if (response.length() == 0) {
    Serial.println("Empty response from Firebase");
    tournaments.push_back("None");
    return false;
  }

  Serial.print("Response (first 300 chars): ");
  Serial.println(response.length() > 300 ? response.substring(0, 300) + "..." : response);

  DynamicJsonDocument doc(4000);
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    tournaments.push_back("None");
    return false;
  }

  JsonObject root = doc.as<JsonObject>();
  if (root.isNull() || root.size() == 0) {
    tournaments.push_back("None");
    return false;
  }

  if (root.containsKey("League")) {
    JsonArray arr = root["League"].as<JsonArray>();
    for (JsonVariant v : arr) {
      tournaments.push_back(v.as<String>());
    }
  }
  if (root.containsKey("Tier")) {
    JsonArray arr = root["Tier"].as<JsonArray>();
    for (JsonVariant v : arr) {
      tiers.push_back(v.as<String>());
    }
  }
  if (root.containsKey("Prize")) {
    JsonArray arr = root["Prize"].as<JsonArray>();
    for (JsonVariant v : arr) {
      prizePools.push_back(v.as<String>());
    }
  }
  if (root.containsKey("Date")) {
    JsonArray arr = root["Date"].as<JsonArray>();
    for (JsonVariant v : arr) {
      dates.push_back(v.as<String>());
    }
  }

  if (tournaments.size() == 0) {
    tournaments.push_back("None");
  }

  Serial.print("Loaded ");
  Serial.print(tournaments.size());
  Serial.println(" tournaments");
  return true;
}
