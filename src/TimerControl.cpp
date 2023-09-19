#include "TimerControl.h"
#include "Arduino.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <DS1307RTC.h>


using namespace std;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
time_t tmr;

void printDateTime(time_t t, const char *tz)
{
    char buf[32];
    char m[4];    // temporary storage for month string (DateStrings.cpp uses shared buffer)
    strcpy(m, monthShortStr(month(t)));
    sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d %s",
        hour(t), minute(t), second(t), dayShortStr(weekday(t)), day(t), m, year(t), tz);
    Serial.println(buf);
}

list<string> splitString(string orig, string delimiter) {
  list<string> ret;
  size_t pos = 0;

  while ((pos = orig.find(delimiter)) != string::npos) {
    string part = orig.substr(0, pos);
    ret.push_front(part);
    orig.erase(0, pos + delimiter.length());
  }

  ret.push_front(orig);

  return ret;
}



int unwrapTimeStr(string timeStr) {
  list<string> times = splitString(timeStr, ":");
  int timeSinceStart = 0;
  string hourStr = times.back();
  times.pop_back();
  string minutesStr = times.back();

  timeSinceStart += (stoi(hourStr) * (3600)) + (stoi(minutesStr) * (60));

  return timeSinceStart;

}

int getTimeNum(string timeStr) {
  list<string> times = splitString(timeStr, ":");
  int timeSinceStart = 0;
  string hourStr = times.back();
  times.pop_back();
  string minutesStr = times.back();

  timeSinceStart += (stoi(hourStr) * (3600)) + (stoi(minutesStr) * (60));

  return timeSinceStart;

}

TimerControl::TimerControl(char *fileName, bool hasWifi)
{
  this->hasWifi = hasWifi;
  this->loadTimerInfo(fileName);
  this->setupClocks(hasWifi);
}

void TimerControl::setupClocks(bool hasWifi)
{

  if (hasWifi)
  {
    Serial.println("Has wifi");
    timeClient.begin();
    delay(1000);
    timeClient.update();
    delay(1000);
    tmr = timeClient.getEpochTime();
    RTC.set(tmr);
    delay(1000);
  }

  this->myTZ = new Timezone(this->myDST, this->mySTD);

  if (!RTC.chipPresent())
  {
    Serial.println("DS1307 read error!  Please check the circuitry.");
  }
  setSyncProvider(RTC.get);
}

void TimerControl::loadTimerInfo(char *fileName)
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  DynamicJsonDocument doc(1024);

  File file = SPIFFS.open(fileName, "r");

  String json = "";
  while (file.available())
  {
    json += file.readStringUntil('\n');
  }
  file.close();

  DeserializationError error = deserializeJson(doc, json);

  if (error)
  {
    Serial.println("ERROR");
    Serial.println(error.c_str());
    return;
  }

  JsonArray triggers = doc["triggers"];

  for (JsonVariant trigger : triggers) {
    string days = trigger["days"];
    string times = trigger["times"];
    string seq = trigger["sequence"];

    list<string> dayList = splitString(days, ",");
    list<string> timeList = splitString(times, ",");

    for(string day : dayList) {
      for(string time: timeList) {
        int timeNum = unwrapTimeStr(time);
        Trigger trig = {.day = this->dow[day], .time = timeNum, .sequence = seq};
        this->triggers.push_back(trig);
      }
    }

  }

  this->seqManager = new SequenceManager(doc["sequences"].as<JsonArray>());
}

int TimerControl::getTimeId(int day, int hour, int minute)
{
  int minutesPerDay = 60 * 24;
  return (day * minutesPerDay) + (hour * 60) + minute;
}

void TimerControl::update()
{
  if (this->hasWifi)
    timeClient.update();
  
  TimeChangeRule *tcr; 
  RTC.read(this->currentTime);
  // Serial.println(this->getCurrentTimeNum());
  this->utc = now();
  this->local = this->myTZ->toLocal(utc, &tcr);
  // printDateTime(utc, "UTC");
  // printDateTime(local, tcr -> abbrev);
  this->seqManager->update(this->getCurrentTimeNum());
  this->fireTrigger();  

  static int test = 0;
  test++;

  if(test == 10) {
     Serial.println("Firing Sequence");
    this->seqManager->startSequence("Angelus", this->getCurrentTimeNum());
  }
}

int TimerControl::getCurrentTimeNum()
{
  return (hour(this->local) * 3600) + (minute(this->local) * 60) + second(this->local);
}

string TimerControl::fireTrigger() {
  int today = weekday(this->local);
  int now = this->getCurrentTimeNum();

  string sequence;

  for(Trigger trigger : this->triggers) {
    // char buf[265];
    //  sprintf(buf, "Current Day %i, Current Time %i - Trigger Day %i, Trigger Time %i",
    //  today, now, trigger.day, trigger.time);
    // Serial.println(buf);
    if(trigger.day == today && trigger.time == now) {
      sequence = trigger.sequence;
      Serial.println("firing");
      Serial.println(sequence.c_str());
      this->seqManager->startSequence(sequence, this->getCurrentTimeNum());

      break;
    }
  }

  return sequence;

}

TimerControl::~TimerControl()
{
}
