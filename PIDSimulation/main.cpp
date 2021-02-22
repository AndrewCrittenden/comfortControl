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
#include "PID.h"

using namespace std;

int tick_get_realtime() {
    struct timeval now;
    gettimeofday(&now,NULL);
    return now.tv_usec/1000+(now.tv_sec-200000000)*1000;
}

int simulatedtime = 0;

int tick_get() {
    return simulatedtime;
}

void original_test_loop() {
        struct pid_controller pidctrl;
    float in = 5, out = 5, set = 5;
    int lastrun = 0;
    progid_t pid = pid_create(&pidctrl, &in, &out, &set, 1, 1, 0.1);
    pid_limits(pid, 0, 255);
    pid_auto(pid);
    cout << in << " " << out << endl;
    const int out_array_size = 5;
    float output_array[out_array_size] = {0};
    int out_array_index = 0;
    float out_array_total = 0;
    int second = tick_get()/1000;
    cout << tick_get() << endl;
    while (true) {
        if (pid_need_compute(pid)) {
            lastrun = tick_get();
            // Read inputs
            //cout << "Sin value: " << (sin(tick_get()/5000.0)-0.5) << endl;
            in = 5+(sin(tick_get()/20000.0)-1) + out_array_total / out_array_size;
            out_array_total -= output_array[out_array_index];
            // Compute PID controllers
            pid_compute(pid);
            output_array[out_array_index] = out;
            out_array_total += out;
            if (out_array_index == out_array_size-1) {
                out_array_index = 0;
            }
            else {
                out_array_index++;
            }
            // Adjust actuator
            // set_out(output);
            if (second!=(tick_get()/500)) {
                second = tick_get()/500;
                cout << "\r" << "Input: " << fixed << setprecision(6) << in << " Output: " << out << " Corrected error: " << abs(5-in) << " Uncorrected error: " << abs((sin(tick_get()/20000.0)-1)) << " Time: " << tick_get();
                cout << "              ";
                cout.flush();
            }
        }
    }
}

inline float kelvinToCelsius(float kelvin) {
    return kelvin-273.15;
}

inline float celsiusToKelvin(float celsius) {
    return celsius+273.15;
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
    float length = 3, width = 3, height = 1.5; // units are meters
    const float air_specific_heat = 1210; // in units of J*m^-3*K^-1
    float controlled_air_volume = length*width*height;
    //float outdoor_temperature = 60; // unit is Celsius
    float indoor_temperature = 18.3333; // unit is Celsius
    float desired_temperature = 22.2222222222;
    float power_output = 0; // unit is watts
    const float max_cool = -2500;
    const float max_heat = 2500;
    const int volume_array_size = 5;
    float volume_array[volume_array_size] = {0};
    int volume_array_index = 0;
    float volume_array_total = 0;
    float oldTemperature = indoor_temperature;
    const int out_array_size = 20*10;
    float output_array[out_array_size] = {0};
    int out_array_index = 0;
    const float timescaler = 100;
    int expected_time = 100; // ms
    int oldTime = tick_get_realtime();
    int simulationresolution = 100;
    bool fileOutput = true;
    string fileName = "tempSimulation.csv";
    
    struct pid_controller pidctrl;
    int lastrun = 0;
    progid_t pid = pid_create(&pidctrl, &indoor_temperature, &power_output, &desired_temperature, 1000, 15, 10000);
    pid_limits(pid, max_cool, max_heat);
    pid_auto(pid);
    pid_sample(pid, 15000);
    int second = tick_get()/1000;
    ofstream outfile;
    if (fileOutput) {
        outfile.open(fileName);
    }
    while (simulatedtime<2000000) {
        if (tick_get()-lastrun>=simulationresolution) {
            if (pid_need_compute(pid)) {
                /*if (tick_get()>lastrun&&abs(indoor_temperature-oldTemperature)>0.01) {
                    volume_array_total -= volume_array[volume_array_index];
                    volume_array[volume_array_index] = (power_output*(tick_get()-lastrun)/1000)/((indoor_temperature-oldTemperature)*air_specific_heat); // estimated volume of air
                    volume_array_total += volume_array[volume_array_index];
                    volume_array_index = (volume_array_index == (volume_array_size-1))?(0):(volume_array_index+1);
                    oldTemperature = indoor_temperature;
                    //cout << volume_array[volume_array_index] << " " << (tick_get()-lastrun) << "              ";
                    //pid_tune(pid,500,5,1000000);
                    //pid_tune(pid,air_specific_heat*(volume_array_total/volume_array_size)/100,1.5,-1);
                }*/
                // Compute PID controllers
                pid_compute(pid);
                // Adjust actuator
                // set_out(output);
            }
            float exterior_offset = cos(tick_get()/86400000.0)*-1000;
            float interior_offset = timeWeightedAverage(output_array,out_array_size,out_array_index);
            indoor_temperature += (interior_offset+exterior_offset)*(tick_get()-lastrun)/1000/(air_specific_heat*controlled_air_volume);
            output_array[out_array_index] = power_output;
            out_array_index = (out_array_index == out_array_size-1)?(0):(out_array_index+1);
            if (second!=(tick_get()/500)) {
                second = tick_get()/500;
                cout << "\r" << "Input: " << fixed << setprecision(6) << indoor_temperature << " Output: " << power_output << " Corrected error: " << abs(indoor_temperature-desired_temperature) << " Time: " << tick_get();
                cout << "              ";
                if (fileOutput) {
                    outfile << indoor_temperature << "," << interior_offset << "," << power_output << endl;
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
    return 0;
}



