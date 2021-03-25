/*
 * Created on March 15, 2021, 8:56 PM
 */

#include "ControlAlgorithm.h"

ControlAlgorithm::ControlAlgorithm(float Kp, float Ki, float Kd, float outputMin, float outputMax, int samplePeriod) {
    progid_t pid = pid_create(&pidctrl, &input, &output, &setpoint, Kp, Ki, Kd);
    pid_limits(pid, outputMin, outputMax);
    pid_auto(pid);
    pid_sample(pid, samplePeriod);
}
void ControlAlgorithm::setOutputBounds(std::pair<float,float> newBounds) {
    while(synchronized&&!mtx.try_lock());
    pid_limits(&pidctrl, newBounds.first, newBounds.second);
    if (synchronized) mtx.unlock();
}
std::pair<float,float> ControlAlgorithm::getOutputBounds() {
    return std::pair<float,float>(pidctrl.omin,pidctrl.omax);
}
void ControlAlgorithm::setParameters(std::tuple<float,float,float> newParams) {
    while(synchronized&&!mtx.try_lock());
    pid_tune(&pidctrl, std::get<0>(newParams), std::get<1>(newParams), std::get<2>(newParams));
    if (synchronized) mtx.unlock();
} 
std::tuple<float,float,float> ControlAlgorithm::getParameters() {
    return std::tuple<float,float,float>(pidctrl.Kp,pidctrl.Ki,pidctrl.Kd);
}
void ControlAlgorithm::setMaxOutput(float newMax) {
    while(synchronized&&!mtx.try_lock());
    pid_limits(&pidctrl, pidctrl.omin, newMax);
    if (synchronized) mtx.unlock();
} 
float ControlAlgorithm::getMaxOutput() {
    return pidctrl.omax;
}
void ControlAlgorithm::setMinOutput(float newMin) {
    while(synchronized&&!mtx.try_lock());
    pid_limits(&pidctrl, newMin, pidctrl.omax);
    if (synchronized) mtx.unlock();
} 
float ControlAlgorithm::getMinOutput() {
    return pidctrl.omin;
}
void ControlAlgorithm::setSetpoint(float newSetpoint) {
    while(synchronized&&!mtx.try_lock());
    setpoint = newSetpoint;
    if (synchronized) mtx.unlock();
} 
float ControlAlgorithm::getSetpoint() {
    return setpoint;
}
void ControlAlgorithm::setCurrentTemperature(float newTemperature) {
    while(synchronized&&!mtx.try_lock());
    input = newTemperature;
    if (synchronized) mtx.unlock();
} 
float ControlAlgorithm::getCurrentTemperature() {
    return input;
}
void ControlAlgorithm::setSamplePeriod(int newSamplePeriod) {
    while(synchronized&&!mtx.try_lock());
    pid_sample(&pidctrl,newSamplePeriod);
    if (synchronized) mtx.unlock();
} 
int ControlAlgorithm::getSamplePeriod() {
    return pidctrl.sampletime;
}
float ControlAlgorithm::getOutput() {
    return output;
}
bool ControlAlgorithm::needsUpdate() {
    return pid_need_compute(&pidctrl);
}
float ControlAlgorithm::doUpdate() {
    printf("PID doUpdate");
    if (needsUpdate()) {
        while(synchronized&&!mtx.try_lock());
        pid_compute(&pidctrl);
        if (synchronized) mtx.unlock();
    }
    return output;
}
float ControlAlgorithm::forceUpdate() {
    while(synchronized&&!mtx.try_lock());
    pid_compute(&pidctrl);
    if (synchronized) mtx.unlock();
    return output;
}
void ControlAlgorithm::setInitial(float newTemperature, float newSetpoint, float newOutput) {
    while(synchronized&&!mtx.try_lock());
    input = newTemperature;
    output = newOutput;
    setpoint = newSetpoint;
    pidctrl.iterm = output;
    pidctrl.lastin = input;
    if (pidctrl.iterm > pidctrl.omax)
            pidctrl.iterm = pidctrl.omax;
    else if (pidctrl.iterm < pidctrl.omin)
            pidctrl.iterm = pidctrl.omin;
    if (synchronized) mtx.unlock();
}
void ControlAlgorithm::beginAlgorithmLoop() {
    synchronized = true;
    algorithmLoop();
}
void ControlAlgorithm::algorithmLoop() {
    while (!exitFlag) {
        doUpdate();
    }
}

