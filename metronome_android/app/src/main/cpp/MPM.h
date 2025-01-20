//
// Created by admin on 2025/1/11.
//

#ifndef METRONOME_ANDROID_MPM_H
#define METRONOME_ANDROID_MPM_H

#include <vector>
#include <cmath>

class MPM {
public :
    double getPitchFromShort(short *data, int sampleRate, int buffersize);

private:
    std::vector<double> normalized_square_difference(const std::vector<double> &buffer);

    std::tuple<double, double> parabolic_interpolation(const std::vector<double> &nsdf, size_t tau);

    double get_pitch(const std::vector<double> &buffer, size_t sample_rate);

    const double cutoff = 0.93;
    const double lower_pitch_cutoff = 40.0;
    const double small_cutoff = 0.5;
};

#endif
