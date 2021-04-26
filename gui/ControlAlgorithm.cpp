/*
 * Created on March 15, 2021, 8:56 PM
 */

#include "ControlAlgorithm.h"

// constructor which sets the P, I, and D coefficients, the output min and max, and the sample period in ms
ControlAlgorithm::ControlAlgorithm(float Kp, float Ki, float Kd, float outputMin, float outputMax, int samplePeriod) {
    progid_t pid = pid_create(&pidctrl, &input, &output, &setpoint, Kp, Ki, Kd);
    pid_limits(pid, outputMin, outputMax);
    pid_auto(pid);
    pid_sample(pid, samplePeriod);
}
// sets the output bounds, taking a pair as input with the first value as the minimum and the second as the maximum
void ControlAlgorithm::setOutputBounds(std::pair<float,float> newBounds) {
    while(synchronized&&!mtx.try_lock());
    pid_limits(&pidctrl, newBounds.first, newBounds.second);
    if (synchronized) mtx.unlock();
}
// gets the output bounds, returns a pair as output with the first value as the minimum and the second value as the maximum
std::pair<float,float> ControlAlgorithm::getOutputBounds() {
    return std::pair<float,float>(pidctrl.omin,pidctrl.omax);
}
// sets the PID parameters, taking a tuple as input, with the first value as Kp, the second as Ki, and the last as Kd
void ControlAlgorithm::setParameters(std::tuple<float,float,float> newParams) {
    while(synchronized&&!mtx.try_lock());
    pid_tune(&pidctrl, std::get<0>(newParams), std::get<1>(newParams), std::get<2>(newParams));
    if (synchronized) mtx.unlock();
}
// gets the PID parameters, returns a tuple as output, with the first value as Kp, the second as Ki, and the last as Kd
std::tuple<float,float,float> ControlAlgorithm::getParameters() {
    return std::tuple<float,float,float>(pidctrl.Kp,pidctrl.Ki,pidctrl.Kd);
}
// sets the PID maximum independently of the minimum, only input is the new maximum as a float
void ControlAlgorithm::setMaxOutput(float newMax) {
    while(synchronized&&!mtx.try_lock());
    pid_limits(&pidctrl, pidctrl.omin, newMax);
    if (synchronized) mtx.unlock();
}
// gets the PID maximum as a float
float ControlAlgorithm::getMaxOutput() {
    return pidctrl.omax;
}
// sets the PID minimum independently of the maximum, only input is the new minimum as a float
void ControlAlgorithm::setMinOutput(float newMin) {
    while(synchronized&&!mtx.try_lock());
    pid_limits(&pidctrl, newMin, pidctrl.omax);
    if (synchronized) mtx.unlock();
}
// gets the PID minimum as a float
float ControlAlgorithm::getMinOutput() {
    return pidctrl.omin;
}
// sets the PID setpoint, only input is the new setpoint as a float
void ControlAlgorithm::setSetpoint(float newSetpoint) {
    while(synchronized&&!mtx.try_lock());
    setpoint = newSetpoint;
    if (synchronized) mtx.unlock();
}
// gets the PID setpoint as a float
float ControlAlgorithm::getSetpoint() {
    return setpoint;
}
// sets the input temperature to the PID, only input is the new temperature as a float
void ControlAlgorithm::setCurrentTemperature(float newTemperature) {
    while(synchronized&&!mtx.try_lock());
    input = newTemperature;
    if (synchronized) mtx.unlock();
}
// gets the PID input temperature value
float ControlAlgorithm::getCurrentTemperature() {
    return input;
}
// sets the PID sample period, only input is the new sample period in ms as an int
void ControlAlgorithm::setSamplePeriod(int newSamplePeriod) {
    while(synchronized&&!mtx.try_lock());
    pid_sample(&pidctrl,newSamplePeriod);
    if (synchronized) mtx.unlock();
}
// gets the PID sample period in ms as an int
int ControlAlgorithm::getSamplePeriod() {
    return pidctrl.sampletime;
}
// gets the PID output as a float
float ControlAlgorithm::getOutput() {
    return output;
}
// returns whether the PID needs to do an update based on the sample period as a boolean
bool ControlAlgorithm::needsUpdate() {
    return pid_need_compute(&pidctrl);
}
// checks if an update is needed via needsUpdate, and does an update if it is needed
// returns the new output level as a float
float ControlAlgorithm::doUpdate() {
    printf("PID doUpdate");
    if (needsUpdate()) {
        while(synchronized&&!mtx.try_lock());
        pid_compute(&pidctrl);
        if (synchronized) mtx.unlock();
    }
    return output;
}
// forces a new update to happen, ignoring the sample period
// returns the output level as a float
float ControlAlgorithm::forceUpdate() {
    while(synchronized&&!mtx.try_lock());
    pid_compute(&pidctrl);
    if (synchronized) mtx.unlock();
    return output;
}
// sets the initial input temperature, setpoint, and output values
// should be used to if a reset to controller's memory is desired
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
// can be called to have the PID controller automatically update itself on a regular basis
void ControlAlgorithm::beginAlgorithmLoop() {
    synchronized = true;
    algorithmLoop();
}
// the loop which actually runs the PID controller algorithm when beginAlgorithmLoop is called
void ControlAlgorithm::algorithmLoop() {
    while (!exitFlag) {
        doUpdate();
    }
}

