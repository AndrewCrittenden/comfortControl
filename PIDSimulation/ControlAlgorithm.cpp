/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ControlAlgorithm.cpp
 * Author: Joseph
 * 
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
    pid_limits(&pidctrl, newBounds.first, newBounds.second);
} 
std::pair<float,float> ControlAlgorithm::getOutputBounds() {
    return std::pair<float,float>(pidctrl.omin,pidctrl.omax);
}
void ControlAlgorithm::setParameters(std::tuple<float,float,float> newParams) {
    pid_tune(&pidctrl, std::get<0>(newParams), std::get<1>(newParams), std::get<2>(newParams));
} 
std::tuple<float,float,float> ControlAlgorithm::getParameters() {
    return std::tuple<float,float,float>(pidctrl.Kp,pidctrl.Ki,pidctrl.Kd);
}
void ControlAlgorithm::setMaxOutput(float newMax) {
    pid_limits(&pidctrl, pidctrl.omin, newMax);
} 
float ControlAlgorithm::getMaxOutput() {
    return pidctrl.omax;
}
void ControlAlgorithm::setMinOutput(float newMin) {
    pid_limits(&pidctrl, newMin, pidctrl.omax);
} 
float ControlAlgorithm::getMinOutput() {
    return pidctrl.omin;
}
void ControlAlgorithm::setSetpoint(float newSetpoint) {
    setpoint = newSetpoint;
} 
float ControlAlgorithm::getSetpoint() {
    return setpoint;
}
void ControlAlgorithm::setCurrentTemperature(float newTemperature) {
    input = newTemperature;
} 
float ControlAlgorithm::getCurrentTemperature() {
    return input;
}
void ControlAlgorithm::setSamplePeriod(int newSamplePeriod) {
    pidctrl.sampletime = newSamplePeriod;
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
    if (needsUpdate()) {
        pid_compute(&pidctrl);
    }
    return output;
}
void ControlAlgorithm::setInitial(float newTemperature, float newSetpoint, float newOutput) {
    input = newTemperature;
    output = newOutput;
    setpoint = newSetpoint;
    pidctrl.iterm = output;
    pidctrl.lastin = input;
    if (pidctrl.iterm > pidctrl.omax)
            pidctrl.iterm = pidctrl.omax;
    else if (pidctrl.iterm < pidctrl.omin)
            pidctrl.iterm = pidctrl.omin;
}

