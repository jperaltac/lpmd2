//
//
//

#include <lpmd/timer.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <sys/resource.h>

#include <lpmd/error.h>

using namespace lpmd;

class lpmd::TimerImpl
{
 public:
   struct timeval t0, t1;
   double treal, tuser, tsystem;
};

Timer::Timer() { timpl = new TimerImpl(); }

Timer::~Timer() { delete timpl; }

void Timer::Start() 
{
 timpl->treal = 0.0;
 timpl->tuser = 0.0;
 timpl->tsystem = 0.0;
 gettimeofday(&(timpl->t0), NULL);
}

void Timer::Stop()
{
 gettimeofday(&(timpl->t1), NULL);
 timpl->treal = double(timpl->t1.tv_sec)+(1e-6)*double(timpl->t1.tv_usec)-double(timpl->t0.tv_sec)-(1e-6)*double(timpl->t0.tv_usec);
 struct rusage usg;
 if (getrusage(RUSAGE_SELF, &usg) == 0)
 {
  timpl->tuser = double(usg.ru_utime.tv_sec) + (1e-6)*double(usg.ru_utime.tv_usec);
  timpl->tsystem = double(usg.ru_stime.tv_sec) + (1e-6)*double(usg.ru_stime.tv_usec);
 }
 else
 {
  throw SystemError(std::string("getrusage failed: ") + std::strerror(errno));
 }
}

void Timer::ShowElapsedTimes() const
{
 std::cout << "Elapsed real time: " << timpl->treal << " seconds." << '\n';
 std::cout << "Elapsed user time: " << timpl->tuser << " seconds." << '\n';
 std::cout << "Elapsed system time: " << timpl->tsystem << " seconds." << '\n';
}


