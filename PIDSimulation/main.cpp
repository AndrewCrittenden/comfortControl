/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * Created on February 20, 2021, 12:19 PM
 */

#include <cstdlib>
#include <iostream>
#include <sys/time.h>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <thread>
#include "ControlAlgorithm.h"

using namespace std;

int tick_get_realtime() {
    struct timeval now;
    gettimeofday(&now,NULL);
    return now.tv_usec/1000+(now.tv_sec-200000000)*1000;
}

int simulatedtime = 0;

int tick_get() {  // REQUIRED
    return simulatedtime;
}

inline float normDist(int input, float sigma, float middle) {
    return exp(pow((input-middle)/sigma,2)/-2)/(sigma*sqrt(2*3.14159265359));
}

inline float timeWeightedAverage(float array[], int length, int start) {
    float to_return = 0;
    int center = length/2;
    for (int i = 0; i < length; i++) {
        to_return += array[(i+start)%length]*normDist(i,length/2,length/2-0.5);
    }
}

int main() {
    const float air_specific_heat = 1210; // in units of J*m^-3*K^-1
    float controlled_air_volume = 0.015; // I don't have the actual length/width/height handy right now
    const float max_cool = -2500;
    const float max_heat = 2500;
    const int out_array_size = 20*10;
    float output_array[out_array_size] = {0};
    int out_array_index = 0;
    const float timescaler = 100;
    int oldTime = tick_get_realtime();
    int simulationresolution = 100;
    bool fileOutput = true;
    string fileName = "tempSimulation.csv";
    
    int lastrun = 0;
    ControlAlgorithm controller(74*controlled_air_volume, 1.1111*controlled_air_volume, 740*controlled_air_volume, max_cool, max_heat, 15000); // REQUIRED
    controller.setInitial(18.3333,22.2222222222); // REQUIRED
    int second = tick_get()/1000;
    ofstream outfile;
    if (fileOutput) {
        outfile.open(fileName);
    }
    thread controlLoop(&ControlAlgorithm::beginAlgorithmLoop,&controller); // REQUIRED
    while (simulatedtime<2000000) {
        if (tick_get()-lastrun>=simulationresolution) {
            float exterior_offset = cos(tick_get()/86400000.0)*-1000;
            float interior_offset = timeWeightedAverage(output_array,out_array_size,out_array_index);
            controller.setCurrentTemperature(((interior_offset+exterior_offset)*(tick_get()-lastrun)/1000/(air_specific_heat*controlled_air_volume)) + controller.getCurrentTemperature());
            output_array[out_array_index] = controller.getOutput();
            out_array_index = (out_array_index == out_array_size-1)?(0):(out_array_index+1);
            if (second!=(tick_get()/500)) {
                second = tick_get()/500;
                cout << "\r" << "Input: " << fixed << setprecision(6) << controller.getCurrentTemperature() << " Output: " << controller.getOutput() << " Time: " << tick_get();
                cout << "              ";
                if (fileOutput) {
                    outfile << controller.getCurrentTemperature() << "," << interior_offset << "," << controller.getOutput() << endl;
                }
                cout.flush();
            }
            lastrun = tick_get();
        }
        int newtime = tick_get_realtime();
        simulatedtime += (((newtime-oldTime)*timescaler>simulationresolution)?simulationresolution:((newtime-oldTime)*timescaler));
        oldTime = newtime;
    }
    if (fileOutput) {
        outfile.close();
    }
    controller.exitFlag = true; // REQUIRED
    controlLoop.join(); // REQUIRED
    return 0;
}



