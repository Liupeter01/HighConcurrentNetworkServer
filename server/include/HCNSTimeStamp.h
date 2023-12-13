#pragma once
#ifndef _HCNSTIMESTAMP_H_
#define _HCNSTIMESTAMP_H_
#include<chrono>

class HCNSTimeStamp
{
public:
          HCNSTimeStamp();
          virtual ~HCNSTimeStamp();

public:
          void updateTimer();
          std::time_t  printCurrentTime();
          long long getElaspsedTimeInMicrosecond() const;
          long long getElaspsedTimeInMillisecond() const;
          long long getElaspsedTimeInsecond() const;

          template<typename TimeType> TimeType getElaspsedTime() const;

protected:
          std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};
#endif
