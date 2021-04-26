/* 
 * Created on March 15, 2021, 8:56 PM
 */

#ifndef CONTROLALGORITHM_H
#define CONTROLALGORITHM_H
#include <utility>
#include <tuple>
#include <atomic>
#include <mutex>
#include "PID.h"

class ControlAlgorithm {
public:
    // constructor which sets the P, I, and D coefficients, the output min and max, and the sample period in ms
    ControlAlgorithm(float Kp, float Ki, float Kd, float outputMin, float outputMax, int samplePeriod);
    // sets the output bounds, taking a pair as input with the first value as the minimum and the second as the maximum
    void setOutputBounds(std::pair<float,float> newBounds);
    // gets the output bounds, returns a pair as output with the first value as the minimum and the second value as the maximum
    std::pair<float,float> getOutputBounds();
    // sets the PID parameters, taking a tuple as input, with the first value as Kp, the second as Ki, and the last as Kd
    void setParameters(std::tuple<float,float,float> newParams);
    // gets the PID parameters, returns a tuple as output, with the first value as Kp, the second as Ki, and the last as Kd
    std::tuple<float,float,float> getParameters();
    // sets the PID maximum independently of the minimum, only input is the new maximum as a float
    void setMaxOutput(float newMax);
    // gets the PID maximum as a float
    float getMaxOutput();
    // sets the PID minimum independently of the maximum, only input is the new minimum as a float
    void setMinOutput(float newMin);
    // gets the PID minimum as a float
    float getMinOutput();
    // sets the PID setpoint, only input is the new setpoint as a float
    void setSetpoint(float newSetpoint);
    // gets the PID setpoint as a float
    float getSetpoint();
    // sets the input temperature to the PID, only input is the new temperature as a float
    void setCurrentTemperature(float newTemperature);
    // gets the PID input temperature value
    float getCurrentTemperature();
    // sets the PID sample period, only input is the new sample period in ms as an int
    void setSamplePeriod(int newSamplePeriod);
    // gets the PID sample period in ms as an int
    int getSamplePeriod();
    // gets the PID output as a float
    float getOutput();
    // returns whether the PID needs to do an update based on the sample period as a boolean
    bool needsUpdate();
    // checks if an update is needed via needsUpdate, and does an update if it is needed
    // returns the new output level as a float
    float doUpdate();
    // forces a new update to happen, ignoring the sample period
    // returns the output level as a float
    float forceUpdate();
    // sets the initial input temperature, setpoint, and output values
    // should be used to if a reset to controller's memory is desired
    void setInitial(float newTemperature, float newSetpoint, float newOutput = 0);
    // can be called to have the PID controller automatically update itself on a regular basis
    void beginAlgorithmLoop();
    // can be set by a thread to indicate that the algorithmLoop should end
    std::atomic<bool> exitFlag;
private:
    // the loop which actually runs the PID controller algorithm when beginAlgorithmLoop is called
    void algorithmLoop();
    // the setpoint for the PID controller
    // stored here because the c-struct declares it as a pointer, presumably for efficiency
    float setpoint;
    // the input for the PID controller
    // stored here because the c-struct declares it as a pointer, presumably for efficiency
    float input;
    // the output for the PID controller
    // stored here because the c-struct declares it as a pointer, presumably for efficiency
    float output;
    // the pid_controller c-struct which is wrapped around by this class
    struct pid_controller pidctrl;
    // the mutex used to synchronize thread operations if the multi-threaded algorithm is used
    std::mutex mtx;
    // the boolean which indicates that the mutex should be used to synchronize function calls
    // only set if beginAlgorithmLoop is called
    bool synchronized;
};

#endif /* CONTROLALGORITHM_H */

