#include<HCNSTimeStamp.h>

HCNSTimeStamp::HCNSTimeStamp()
          :m_start(std::chrono::high_resolution_clock::now())
{
          
}

HCNSTimeStamp::~HCNSTimeStamp()
{

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
long long HCNSTimeStamp::getElaspsedTime() const
{
          auto _time_interval = std::chrono::high_resolution_clock::now() - this->m_start;
          return std::chrono::duration_cast<TimeType>(_time_interval).count();
}