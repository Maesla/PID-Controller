#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

// for convenience
using json = nlohmann::json;

using namespace std;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}




bool do_twiddle = false;
int const TWIDDLE_MAX_FRAME_COUNT = 500;
int param_iter;

//Steer
//double p[3] = {0.121459, 0.000969774, 0.18647};
//double dp [3] = {0.00166699, 0.000334758, 0.00111592};

//Speed
double p[3] = {0.0476519, -8.561e-05, 0.201947};
double dp [3] =  {0.00541994, 7.07348e-05, 0.0245334};

int const TWIDDLE_PARAM_COUNT = 3;
double const TWIDDLE_TOL = 0.0001;

double best_error;

//FLAGS
bool is_init = false;
bool is_increasing_parameter = false;
bool is_decreasing_parameter = false;


void reset_simulator(uWS::WebSocket<uWS::SERVER>& ws)
{
  // reset
  std::string msg("42[\"reset\", {}]");
  ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
}


void Twiddle(PID &pid, PID &otherPid, uWS::WebSocket<uWS::SERVER>& ws)
{
  if(!do_twiddle)
    return;

  if (!is_init)
  {
    //p[3] = new{0.0f,0.0f,0.0f};
    //dp [3] = new{1.0f,1.0f,1.0f};

    cout << "Twiddle Starting!" << endl;

    param_iter = 0;
    best_error = 9999999999;

    is_init = true;
    is_increasing_parameter = false;
    is_decreasing_parameter = false;

    pid.Init(p[0],p[1],p[2]);
    otherPid.ResetErrors();
    reset_simulator(ws);
  }

  bool is_running = pid.count < TWIDDLE_MAX_FRAME_COUNT;
  if(is_running)
  {
    cout << "Twiddle Calculating. Value: " << pid.value << " Error:" << pid.average_error << " " << pid.count << "/" << TWIDDLE_MAX_FRAME_COUNT << endl;

    return;
  }

  double dp_sum = 0.0f;
  for(int i = 0; i < TWIDDLE_PARAM_COUNT; i++)
  {
    dp_sum += dp[i];
  }

  if (dp_sum < TWIDDLE_TOL)
  {
    cout << "--------------------->Twiddle Completed!" << endl;
    cout << "--------------------->Best error " << best_error << endl;

    for (int i = 0; i < TWIDDLE_PARAM_COUNT; i++)
    {
      cout << "Param: " << i << " Value: " << p[i] <<endl;
    }
    do_twiddle = false;
    return;
  }


  if (!is_decreasing_parameter)
  {
    if (!is_increasing_parameter)
    {

      p[param_iter] += dp[param_iter];

      cout << "--------------------->Trying Increasing param: " << param_iter << endl;
      cout << "ParamIdex: " << param_iter << " Values: "<< p[0] << " " << p[1] << " " << p[2] << endl;
      cout << "Deltas: "<< dp[0] << " " << dp[1] << " " << dp[2] << endl;
      cout << "--------------------->Best error " << best_error << endl;


      is_increasing_parameter = true;
      pid.Init(p[0],p[1],p[2]);
      otherPid.ResetErrors();
      reset_simulator(ws);
      return;
    }

    double error = pid.average_error;
    if(error < best_error)
    {
      best_error = error;
      dp[param_iter]*=1.1f;

      cout << "--------------------->Twiddle New Best Error Increasing!" << endl;
      cout << "--------------------->Best error " << error << endl;
      cout << "ParamIdex: " << param_iter << " Values: "<< p[0] << " " << p[1] << " " << p[2] << endl;
      cout << "Deltas: "<< dp[0] << " " << dp[1] << " " << dp[2] << endl;


      param_iter = (param_iter+1)%TWIDDLE_PARAM_COUNT;
      is_increasing_parameter = false;
      is_decreasing_parameter = false;

      return;

    }
    else
    {
      p[param_iter] -= 2.0f*dp[param_iter];
      is_decreasing_parameter = true;

      cout << "--------------------->Trying Decreasing param: " << param_iter << endl;
      cout << "ParamIdex: " << param_iter << " Values: "<< p[0] << " " << p[1] << " " << p[2] << endl;
      cout << "Deltas: "<< dp[0] << " " << dp[1] << " " << dp[2] << endl;
      cout << "--------------------->Best error " << best_error << endl;

      pid.Init(p[0],p[1],p[2]);
      otherPid.ResetErrors();
      reset_simulator(ws);

      return;
    }
  }

  double error = pid.average_error;
  if(error < best_error)
  {
    best_error = error;
    dp[param_iter]*=1.1f;

    cout << "--------------------->Twiddle New Best Error Decreasing!" << endl;
    cout << "ParamIdex: " << param_iter << " Values: "<< p[0] << " " << p[1] << " " << p[2] << endl;
    cout << "Deltas: "<< dp[0] << " " << dp[1] << " " << dp[2] << endl;

  }
  else
  {
    p[param_iter] += dp[param_iter];
    dp[param_iter] *= 0.9;

    cout << "--------------------->Trying Decreasing delta param: " << param_iter << endl;
    cout << "ParamIdex: " << param_iter << " Values: "<< p[0] << " " << p[1] << " " << p[2] << endl;
    cout << "Deltas: "<< dp[0] << " " << dp[1] << " " << dp[2] << endl;
  }

  param_iter = (param_iter+1)%TWIDDLE_PARAM_COUNT;
  is_increasing_parameter = false;
  is_decreasing_parameter = false;
}

double clamp(double value, double min, double max)
{
  if (value > max)
    return max;
  else if (value < min)
    return min;

  return value;
}

int main()
{
  uWS::Hub h;

  PID pidSteer;
  PID pidSpeed;
  // TODO: Initialize the pid variable.
  pidSteer.Init(0.124463, 0.00167277, 0.18647);
  pidSpeed.Init(0.0476519, -8.561e-05, 0.201947);

  h.onMessage([&pidSteer, &pidSpeed](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          double speed = std::stod(j[1]["speed"].get<std::string>());
          double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;
          double throttle = 0.1;
          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */
          Twiddle(pidSpeed, pidSteer, ws);

          pidSteer.UpdateError(cte);
          steer_value = pidSteer.value;
          steer_value = clamp(steer_value, -1, 1);

          pidSpeed.UpdateError(speed -7.5f);
          throttle = pidSpeed.value;
          throttle = clamp(throttle, 0, 1);
          //throttle = 0.1f;
          // DEBUG
         // std::cout << "CTE: " << cte << " Steer: " << steer_value  << " Throttle: " << throttle << std::endl;

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = throttle;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
