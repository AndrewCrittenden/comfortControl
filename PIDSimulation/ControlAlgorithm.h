/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ControlAlgorithm.h
 * Author: Joseph
 *
 * Created on March 15, 2021, 8:56 PM
 */

#ifndef CONTROLALGORITHM_H
#define CONTROLALGORITHM_H
#include <utility>
#include <tuple>
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
private:
    float setpoint;
    float input;
    float output;
    struct pid_controller pidctrl;
};

#endif /* CONTROLALGORITHM_H */

