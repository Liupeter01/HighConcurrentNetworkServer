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
          
          template<typename TimeType>
          long long getElaspsedTime() const;

          long long getElaspsedTimeInMicrosecond() const;
          long long getElaspsedTimeInMillisecond() const;
        
protected:
          std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};
#endif
