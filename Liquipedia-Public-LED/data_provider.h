#ifndef DATA_PROVIDER_H
#define DATA_PROVIDER_H
#include <Arduino.h>
#include <vector>
bool loadTournamentData(const String& game, const String& temporal,
  std::vector<String>& tournaments, std::vector<String>& tiers,
  std::vector<String>& prizePools, std::vector<String>& dates);
#endif
