
#ifndef sequence_manager_h
#define sequence_manager_h

#include <ArduinoJson.h>
#include <Arduino.h>
#include <TimeLib.h>
#include <list>
#include <map>
#include <deque>

struct SequenceStage {
  int endTime;
  int relayValue;
};

struct Sequence {
  std::string name;
  std::deque<SequenceStage> stages;
};

class SequenceManager
{
private:
  /* data */
  bool activeSequence = false;
  std::string currentActiveSequence = "";
  int currentSequenceStartTime = 0;
  int currentSequenceStageNum = 0;
  int currentSequenceTotalStages = 0;

  std::map<std::string, Sequence> sequences;

public:
  SequenceManager(JsonArray seq);
  ~SequenceManager();
  void update(int currentTime);
  void startSequence(std::string sequence, int currentTime);
};

#endif
