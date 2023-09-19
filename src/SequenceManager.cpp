#include "SequenceManager.h"
const int relayPin = D5;

SequenceManager::SequenceManager(JsonArray seq)
{
  Serial.println("Loading Sequqnces");
  for (JsonVariant sequence : seq)
  {
    std::string name = sequence["name"];
    JsonArray triggerSeq = sequence["seq"];
    std::deque<SequenceStage> stages;

    Sequence newSequence = {
        .name = name,
        .stages = stages};
    int stageSum = 0;
    int currentVal = 1;
    for (int stage : triggerSeq)
    {
      stageSum += stage;
      SequenceStage triggerSeqStage = {
          .endTime = stageSum,
          .relayValue = currentVal};

      if (currentVal)
        currentVal--;
      else
        currentVal++;

      newSequence.stages.push_back(triggerSeqStage);
    }

    this->sequences[name] = newSequence;
  }
}

void SequenceManager::startSequence(std::string sequence, int currentTime)
{
  Serial.println("STARTING1");
  Serial.println(sequence.c_str());
  Serial.println(currentTime);

  if (this->activeSequence)
    return;
  Serial.println("STARTING2");

  Sequence newSeq = this->sequences[sequence];

  this->currentActiveSequence = sequence;
  this->currentSequenceStartTime = currentTime;
  this->currentSequenceStageNum = 0;
  this->currentSequenceTotalStages = newSeq.stages.size();

  Serial.println(newSeq.stages[this->currentSequenceStageNum].endTime);
  Serial.println(newSeq.stages[this->currentSequenceStageNum].relayValue);

  this->activeSequence = true;

  char str[128];
  sprintf(str, "Starting sequence %s, First Stage till %i", sequence.c_str(), currentTime + newSeq.stages[this->currentSequenceStageNum].endTime);
  Serial.println(str);
}

void SequenceManager::update(int currentTime)
{

  if (this->activeSequence)
  {
    Serial.println(currentTime);
    Sequence activeSeq = this->sequences[this->currentActiveSequence];
    SequenceStage activeStage = activeSeq.stages[this->currentSequenceStageNum];
    digitalWrite(relayPin, activeStage.relayValue);

    if (activeSeq.stages[this->currentSequenceStageNum].endTime + this->currentSequenceStartTime < currentTime)
    {
      char str[128];

      this->currentSequenceStageNum++;
      if (this->currentSequenceStageNum >= this->currentSequenceTotalStages)
      {
        this->activeSequence = false;
        digitalWrite(relayPin, 0);
        sprintf(str, "Sequence %s complete", activeSeq.name);
        Serial.println(str);

        return;
      }

      sprintf(str, "Sequence %s: stage %i complete. starting Next till %i", activeSeq.name, this->currentSequenceStageNum - 1, currentTime + activeSeq.stages[this->currentSequenceStageNum].endTime);
      Serial.println(str);
    }
  }
}

SequenceManager::~SequenceManager()
{
}
