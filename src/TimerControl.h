
#ifndef timer_control_h
#define timer_control_h

#include <TimeLib.h>
#include <list>
#include <map>
#include <Timezone.h> 
#include "SequenceManager.h"

struct Trigger {
  int day;
  int time; // seconds from midnight
  std::string sequence;
};

class TimerControl
{
private:
  /* data */
  bool hasWifi;
  std::map<std::string, int > dow = { { "sun", 1},{ "mon", 2},{ "tue", 3},{ "wed", 4},{ "thurs", 5},{ "fri", 6},{ "sat", 7} };
  tmElements_t currentTime;
  std::list<Trigger> triggers;
  time_t local, utc;

  int getTimeId(int day, int hour, int minute);
  void loadTimerInfo(char * fileName);
  void setupClocks(bool hasWifi);
  int getCurrentTimeNum();
  TimeChangeRule myDST = {"IST", Last, Sun, Mar, 2, 60};   
  TimeChangeRule mySTD = {"GMT", Last, Sun, Oct, 2, 0};  
  Timezone *myTZ;
  SequenceManager * seqManager;

public:
  TimerControl(char *fileName, bool hasWifi);
  ~TimerControl();

  char* checkTimer();
  void update();
  char* getRTCTimeString();
  std::string fireTrigger();
};

#endif
