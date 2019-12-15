#include "Sniffer.h"
#include "Arduino.h"
#define minPause 10000
#define MAX_PULSE 128
uint8_t interruptPin = 4;

volatile unsigned int prev_time = 0;
volatile bool CodeReady = false;
volatile bool isValid = false;
volatile unsigned int timing[MAX_PULSE];
volatile int nTiming = 0;

Sniffer::Sniffer()
{
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), SignalInterrupt, CHANGE);
}

void IRAM_ATTR Sniffer::SignalInterrupt() {
  long time = micros();
  unsigned int duration = time - prev_time;
  prev_time = time;
  
  if((duration < 32000 && duration > 128) && CodeReady == false)
  {
    if(isValid == false)
    {
      if(duration > minPause)//Pause pulse detected
      {
        //pause pulse
        timing[0] = duration;
        nTiming++;
        isValid = true;
      }
    }
    else
    {
      timing[nTiming] = duration;
      nTiming++;
      //Check final pause pulse
      if(isEqual(timing[0], duration, 30) && nTiming > 23)
      {
        nTiming++;
        timing[nTiming] = duration;
        if( ((nTiming - 2)/2)%2 == 0)
        {
          CodeReady = true;
          isValid = false;
          detachInterrupt(digitalPinToInterrupt(interruptPin));
        }
        else
        {
          CodeReady = false;
          isValid = false;
          nTiming = 0;
          timing[0] = 0;              
        }
      }
      if(nTiming == MAX_PULSE)
      {
        CodeReady = false;
        isValid = false;
        nTiming = 0;
        timing[0] = 0;
      }
    }
  }
}


bool Sniffer::isEqual(int A, int B, int tolerance)
{
  int diff = abs(A-B);
  if(diff > tolerance){return false;}
  if(diff <= tolerance){return true;}
}

bool Sniffer::available() {
  return CodeReady;
}

int Sniffer::nPulse()
{
  return nTiming - 2;
}

int Sniffer::GetPauseLenght()
{
  return timing[0];
}

int Sniffer::GetStartPulseLenght()
{
  return timing[1];
}

int Sniffer::GetMinPulseLenght()
{
  int pmin = 2000;
  for(int i = 2; i < 20; i++)
  {
    if(timing[i] < pmin){pmin = timing[i];}
  }
  return pmin;
}

int Sniffer::GetMaxPulseLenght()
{
  int pmin = 0;
  for(int i = 2; i < 20; i++)
  {
    if(timing[i] > pmin){pmin = timing[i];}
  }
  return pmin;
}

unsigned int Sniffer::GetCode(int index)
{
  return timing[index];
}

void Sniffer::continueReceive()
{
  CodeReady = false;
  nTiming = 0;
  timing[0] = 0;
  attachInterrupt(digitalPinToInterrupt(interruptPin), SignalInterrupt, CHANGE);
}

void Sniffer::disableReceive()
{
  detachInterrupt(digitalPinToInterrupt(interruptPin));
}
