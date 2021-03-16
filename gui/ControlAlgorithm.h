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
    ControlAlgorithm(float Kp, float Ki, float Kd, float outputMin, float outputMax, int samplePeriod);
    void setOutputBounds(std::pair<float,float> newBounds);
    std::pair<float,float> getOutputBounds();
    void setParameters(std::tuple<float,float,float> newParams);
    std::tuple<float,float,float> getParameters();
    void setMaxOutput(float newMax);
    float getMaxOutput();
    void setMinOutput(float newMin);
    float getMinOutput();
    void setSetpoint(float newSetpoint);
    float getSetpoint();
    void setCurrentTemperature(float newTemperature);
    float getCurrentTemperature();
    void setSamplePeriod(int newSamplePeriod);
    int getSamplePeriod();
    float getOutput();
    bool needsUpdate();
    float doUpdate();
    void setInitial(float newTemperature, float newSetpoint, float newOutput = 0);
    void beginAlgorithmLoop();
    std::atomic<bool> exitFlag;
private:
    void algorithmLoop();
    float setpoint;
    float input;
    float output;
    struct pid_controller pidctrl;
    std::mutex mtx;
    bool synchronized;
};

#endif /* CONTROLALGORITHM_H */

