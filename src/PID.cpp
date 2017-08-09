#include "PID.h"
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */

using namespace std;

/*
* TODO: Complete the PID class.
*/

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd)
{
  this->Kp = Kp;
  this->Ki = Ki;
  this->Kd = Kd;

  ResetErrors();

}

void PID::ResetErrors()
{
  p_error = 0.0f;
  i_error = 0.0f;
  d_error = 0.0f;

  previous_time = 0.0f;

  count = 0.0f;
  total_error = 0.0f;
  average_error = 0.0f;

}

void PID::UpdateError(double cte)
{

  double dt = GetDeltaTime();
  dt = 1.0f;

  p_error = cte;

  d_error = (cte - cte_previous)/dt;
  cte_previous = cte;

  i_error += cte*dt;

  value = -Kp*p_error - Ki*i_error - Kd*d_error;

  total_error += (cte*cte);
  count ++;
  average_error = total_error/count;
}

double PID::TotalError() {
}

double PID::GetDeltaTime()
{
  double current_time = clock();
  double dt = (current_time - previous_time)/CLOCKS_PER_SEC;
  previous_time = current_time;

  return dt;
}

