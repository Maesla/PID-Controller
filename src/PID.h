#ifndef PID_H
#define PID_H

class PID {
public:
  /*
  * Errors
  */
  double p_error;
  double i_error;
  double d_error;
  double total_error;
  double average_error;

  /*
  * Coefficients
  */ 
  double Kp;
  double Ki;
  double Kd;

  double cte_previous;
  double value;

  /*
  * Constructor
  */
  PID();

  /*
  * Destructor.
  */
  virtual ~PID();

  /*
  * Initialize PID.
  */
  void Init(double Kp, double Ki, double Kd);
  void ResetErrors();

  /*
  * Update the PID error variables given cross track error.
  */
  void UpdateError(double cte);

  /*
  * Calculate the total PID error.
  */
  double TotalError();

  int count;

private:
  double previous_time;
  double GetDeltaTime();
};

#endif /* PID_H */
