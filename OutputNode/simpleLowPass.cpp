#include <cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>

using namespace std;

class simpleLowPass {
public:
    // empty constructor to allow delay initialization
    simpleLowPass() {
        this->a = 0;
        this->cutoff = false;
        for (int i = 0; i < arrSize; i++) {
            mem[i] = 0;
        }
    }
    // main constructor: aIn is the alpha constant variable, cutoff controls whether the output will drop to zero when the output crosses 0
    simpleLowPass(double aIn, bool cutoff = false, double initialOutput = 0) {
        this->a = aIn;
        this->cutoff = cutoff;
        for (int i = 0; i < arrSize; i++) {
            mem[i] = initialOutput;
        }
    }
    // this function takes an input (the current filter input) and returns 
    // the output of the filter
    // the output will be zero of cutoff is enabled and the output and the input
    // and the output have different signs (this makes the filter waste less 
    // energy if, say, its driving a Peltier device)
    double filter(double input) {
        /*double output = input - 0.5*xm1;
        xm1 = input;
        return output;*/
        mem[0] = a*input + (1-a)*mem[0];
        if (cutoff && ((input>0)^(mem[0]>0))) {
            return 0;
        }
        return mem[0];
    }
private:
    static const int arrSize = 1;
    double a;
    double mem[arrSize];
    bool cutoff;
};

/*
 * This is a test function for the filter. The default input is a square wave.
 * The output of the filter will be equivalent to an RC circuit. The test data is written
 * to a CSV called testData.csv. The code for file IO was copied from my PID simulation program.
 */

int main() {
    string fileName = "testData.csv";
    const bool fileOutput = true;
    ofstream outfile;
    if (fileOutput) {
        outfile.open(fileName);
    }
    double fc = 0.00333333333; // the desired characteristic frequency goes here
    double alpha = 1/(1+1/(fc*2*3.14159)); // this is the alpha given the desired characteristic frequency AND A SAMPLE FREQUENCY OF 1
    // if the sample frequency is not 1, then the numerator of the fraction (the one with fc in the denominator) should be the sample frequency
    double sqrFreq = 100;
    simpleLowPass filt = simpleLowPass(alpha,true);
    for (int i = 0; i < 1000; i++) { // i = 1 second
        double input;
        if (sin(i/sqrFreq)!=0) {
            input = sin(i/sqrFreq)/abs(sin(i/sqrFreq)); // lazy and inefficient square wave
        }
        else {
            input = 1/2;
        }
        outfile << i << "," << input << "," << filt.filter(input) << endl;
    }
    if (fileOutput) {
        outfile.close();
    }
    return 0;
}
