#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

// for convenience
using json = nlohmann::json;

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




bool do_twiddle;
int const TWIDDLE_MAX_FRAME_COUNT = 1;
int param_iter;

double p[3] = {0.0f,0.0f,0.0f};
double dp [3] = {1.0f,1.0f,1.0f};

int const TWIDDLE_PARAM_COUNT = 3;
double const TWIDDLE_TOL = 0.2;

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


void Twiddle(PID &pid, uWS::WebSocket<uWS::SERVER>& ws)
{
test();
  if(!do_twiddle)
    return;

  bool is_running = pid.count < TWIDDLE_MAX_FRAME_COUNT;
  if(is_running)
    return;

  double dp_sum = 0.0f;
  for(int i = 0; i < TWIDDLE_PARAM_COUNT; i++)
  {
    dp_sum += dp[i];
  }

  if (dp_sum < TWIDDLE_TOL)
  {
    do_twiddle = false;
    return;
  }

  if (is_init)
  {
    //p[3] = new{0.0f,0.0f,0.0f};
    //dp [3] = new{1.0f,1.0f,1.0f};

    param_iter = 0;
    best_error = 99999;

    is_init = true;
    is_increasing_parameter = false;
    is_decreasing_parameter = false;
  }

  if (!is_decreasing_parameter)
  {
    if (!is_increasing_parameter)
    {
      p[param_iter] += dp[param_iter];
      is_increasing_parameter = true;
      pid.Init(p[0],p[1],p[2]);
      reset_simulator(ws);
    }

    double error = pid.average_error;
    if(error < best_error)
    {
      best_error = error;
      dp[param_iter]*=1.1f;
    }
    else
    {
      p[param_iter] -= 2.0f*dp[param_iter];
      is_decreasing_parameter = true;
      pid.Init(p[0],p[1],p[2]);
      reset_simulator(ws);
    }
  }

  double error = pid.average_error;
  if(error < best_error)
  {
    best_error = error;
    dp[param_iter]*=1.1f;
  }
  else
  {
    p[param_iter] += dp[param_iter];
    dp[param_iter] *= 0.9;
  }

  param_iter = (param_iter+1)%TWIDDLE_PARAM_COUNT;
  is_increasing_parameter = false;
  is_decreasing_parameter = false;
}

int main()
{
  uWS::Hub h;

  PID pidSteer;
  // TODO: Initialize the pid variable.
  pidSteer.Init(1.0f, 0.0f, 0.0f);

  h.onMessage([&pidSteer](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
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
          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */















          pidSteer.UpdateError(cte);
          steer_value = pidSteer.value;
          // DEBUG
          std::cout << "CTE: " << cte << " Steering Value: " << steer_value  << " Count: " << pidSteer.count << std::endl;

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = 0.3;
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
