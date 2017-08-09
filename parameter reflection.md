
# Chosen parameters #

In order to completed a full lap, I have implemented two PIDs, one for steering and other for speed controlling.

For steering PID, these parameters have been chosen:

| Parameter | Value |
|-----------|-------|
| kp		| 0.124463    |
| ki		| 0.00167277  |
| kd		| 0.18647     |

For speed controlling PID, these parameters have been chosen:

| Parameter | Value |
|-----------|-------|
| kp		| 0.0476519    |
| ki		| -8.561e-05  |
| kd		| 0.201947     |

# Tunning parameters methodology #
At the beginning, I just started with steering PID and I left throttle just to 0.3.
I just started to tune parameters by trial and error. But it didn't work for me because I didn't realize when the changes make the result better or worse.

So I developed the Twiddle algorithm that can be found in **main.cpp**. The tricky part of the algorithm, comparing with the one that it is developed in the lesson, is the lesson one is synchronous and, in the project, the algorithm need to be asynchronous.
For example, in Unity you can use coroutines to yield in some point and then return from this point. In order to achieve this kind of behavior, I have set several flags to continue later the flow when a reset is launched from the Twiddle algorithm.

Once the algorithm was completed and working, I found other issue. I needed to choose the parameters for the Twiddle. I started such as in the lesson, with p[] = {0,0,0} and dp[]={1,1,1}, but the result was pretty awful.
Then, I set up like this, with p[] = {0,0,0} and dp[]={0.1,0.0001,1} and the results were much better.

I started calculating the error in 500 time steps, but I realized with a very few you can see if the parameters are right or not. So I started again only with 50, to iterate faster. When I got a reasonable error, I increased the time steps to 100, but starting with the parameters calculated with 50. Then I repeated the operation with 500.

When I got a good parametrization for steering, I decided to use a PID for throttle. The reason was because I wanted a stable speed, but with a constant throttle, the speed kept increasing.
The methodology was the same. With the last steering PID parametrization, I launched the Twiddle algorithm against the speed controlling PID, with 50, 100 and finally 500 time steps.

The final loop was launching again the twiddle algorithm against the steering PID, with 500 time steps and the throttle controlled by its PID.

# P,I,D effect #
I have written down several configuration at the steering PID while the twiddle algorithm were finding new best parametrization
| kp       | ki           | kd       | error    |
|----------|--------------|----------|----------|
| 0.28     | -1           | 2.05     | 12.69    |
| 0.199    | 0            | 0.21     | 0.9649   |
| 0.178821 | 0            | 0.21     | 0.621226 |
| 0.156623 | 0            | 0.21     | 0.3092   |
| 0.123768 | 0.0009767741 | 0.187862 | 0.171612 |

The first thing that attracted my attention was that the parameter that decreases the most the error is kp. Kd is very useful to complement kp value.
The most tricky one is ki. It could help, but in a very small proportion. It make sense, since it going to apply to error_i, which is increasing in each time step.