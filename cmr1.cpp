/******************************************************************
 * 
 * Comment block for the radar
 * 
 *****************************************************************/


void initRadar(){
    //init IIO
    //
    //calibrate fast lock by iterating
    //through freqs, locking pll, and 
    //saving to fast lock profile.
    //
    //generate chirp in array
    //
    //create tx buffer with cyclic=true
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
}