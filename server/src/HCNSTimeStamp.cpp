#include<HCNSTimeStamp.h>

HCNSTimeStamp::HCNSTimeStamp()
          :m_start(std::chrono::high_resolution_clock::now())
{
          
}

HCNSTimeStamp::~HCNSTimeStamp()
{

}

std::time_t HCNSTimeStamp::printCurrentTime()
{
          auto _currTime = std::chrono::system_clock::now();              //get current time from stl
          return  std::chrono::system_clock::to_time_t(_currTime);
}

void HCNSTimeStamp::updateTimer()
{
          this->m_start = std::chrono::high_resolution_clock::now();  //get current time
}

/*------------------------------------------------------------------------------------------------------
* calculate the time interval between current time and the last time( this->m_start) according to the timetype
* @function: void getElaspsedTime
* @retvalue: double
*------------------------------------------------------------------------------------------------------*/
template<typename TimeType>
TimeType HCNSTimeStamp::getElaspsedTime() const
{
          auto _time_interval = std::chrono::high_resolution_clock::now() - this->m_start;
          return std::chrono::duration_cast<TimeType>(_time_interval);
}

long long HCNSTimeStamp::getElaspsedTimeInMicrosecond() const
{
          return this->getElaspsedTime<std::chrono::microseconds>().count();
}

long long HCNSTimeStamp::getElaspsedTimeInMillisecond() const
{
          return this->getElaspsedTime<std::chrono::milliseconds>().count();
}

long long HCNSTimeStamp::getElaspsedTimeInsecond() const
{
          return this->getElaspsedTime<std::chrono::seconds>().count();
}
