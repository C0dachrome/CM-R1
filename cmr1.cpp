/******************************************************************
 * 
 * Comment block for the radar
 * 
 *****************************************************************/
#include <iostream>
#include <vector>
#include <complex>
#include <cstdint>
#include <cmath>

double sampleRate = 10e6;
double duration = 0.001;
double bandwidth = 50e6;
double startFreq = 0;


//array for lock frequencies
long long int freqs[3] = { 5.735e9, 5.775e9, 5.815e9 };

long int numSamps = static_cast<int>(sampleRate * duration); //arbitrary number for now

//tx buffer is int16_t, we convert to IQ later
std::vector<int16_t> txBuffer;

double chirpRate = bandwidth / duration;

// //start with chirp as floats then we convert to int16_t later
// std::vector<std::complex<float>> chirpFloat(numSamps);

void initRadar(){
    //init IIO
    
    //calibrate fast lock by iterating
    //through freqs, locking pll, and 
    //saving to fast lock profile.
    for(auto& freq : freqs){
        //just a cout for debugging
        std::cout << "locking PLL to " << freq << "hz";
        //lock PLL to freq
        //initialize_fastlock_save
        
    }
    
    //generate chirp in array
    
    //create tx buffer with cyclic=true
    // txBuffer.reserve(numSamps * 2);
    //
    //push the buffer

    //load phase offsets into an array

    return;
}

int main(){
    //for freq in freqs
    //  recall fast lock profile [freq]
    //  refill RX buffers
    //  remove DC offset
    //  correct for cable offset via the array
    //  apply window
    //  calculate IF
    //  run FFT on IF
    //  align hop phase

    //for each fft peak
    //  find rx phase diff
    //  convert to angle
    //  format packet and push via udp
    initRadar();
}