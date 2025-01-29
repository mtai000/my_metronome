//
// Created by admin on 2025/1/11.
//
// refer to : https://github.com/sevagh/pitch-detection/blob/master/src/mpm.cpp

#include "MPM.h"
#include <cpu-features.h>
#include <arm_neon.h>

double MPM::getPitchFromShort(short *data, int sampleRate, int buffersize) {
    if (!data || buffersize <= 0) {
        return -1.0;
    }

    std::vector<double> audioBuffer(buffersize);
    for (int i = 0; i < buffersize; ++i) {
        audioBuffer[i] = static_cast<double>(data[i]) / 32768.f;
    }
    return get_pitch(audioBuffer, sampleRate);
}

std::vector<double> MPM::normalized_square_diff_simd(const std::vector<double> &buffer) {
    size_t len = buffer.size();
    std::vector<double> nsdf(len, 0.0);

    size_t aligned_len = (len/4)*4;
    for(int tau = 0; tau < aligned_len; ++tau){
        double acf = 0.0;
        double divisor_m = 0.0;
        for(int i = 0; i + tau < aligned_len; i += 4){
            float32x4_t buffer_vec1 = vld1q_f32(reinterpret_cast<const float *>(&buffer[i]));
            float32x4_t buffer_vec2 = vld1q_f32(reinterpret_cast<const float *>(&buffer[i + tau]));

            float32x4_t acf_vec = vmulq_f32(buffer_vec1,buffer_vec2);
            acf += vaddvq_f32(acf_vec);

            float32x4_t p1 = vmulq_f32(buffer_vec1,buffer_vec1);
            float32x4_t p2 = vmulq_f32(buffer_vec2,buffer_vec2);
            float32x4_t divisor_vec = vaddq_f32(p1,p2);
            divisor_m += vaddvq_f32(divisor_vec);
        }

        for(int i = aligned_len; i+ tau < len; i++){
            acf += buffer[i] * buffer[i+tau];
            double p1 = buffer[i] * buffer[i];
            double p2 = buffer[i+tau] * buffer[i+ tau];
            divisor_m += p1 + p2;
        }
        nsdf[tau] = 2.0*acf/divisor_m;
    }

    return nsdf;
}

std::vector<double> MPM::normalized_square_difference(const std::vector<double> &buffer) {
    size_t len = buffer.size();
    std::vector<double> nsdf(len, 0.0);

    for (int tau = 0; tau < len; ++tau) {
        double acf = 0.0;
        double divisor_m = 0.0;
        for (int i = 0; i + tau < len; ++i) {
            acf += buffer[i] * buffer[i + tau];
            double el1 = buffer[i];
            double p1 = el1 * el1;
            double el2 = buffer[i + tau];
            double p2 = el2 * el2;
            divisor_m += p1 + p2;
        }
        nsdf[tau] = 2.0 * acf / divisor_m;
    }

    return nsdf;
}

std::tuple<double, double>
MPM::parabolic_interpolation(const std::vector<double> &nsdf, size_t tau) {
    double nsdfa = nsdf[tau - 1];
    double nsdfb = nsdf[tau];
    double nsdfc = nsdf[tau + 1];

    double b_val = static_cast<double>(tau);
    double bottom = nsdfc + nsdfa - 2.0 * nsdfb;

    if (bottom == 0.0) {
        return {b_val, bottom};
    } else {
        double delta = nsdfa - nsdfc;
        return {
                b_val + delta / (2.0 * bottom),
                nsdfa - delta * delta / (8.0 * bottom)
        };
    }
}

std::vector<size_t> peak_picking(const std::vector<double> &nsdf) {
    std::vector<size_t> max_position;
    size_t pos = 0;
    size_t cur_max_pos = 0;
    size_t len = nsdf.size();

    while (pos < (len - 1) / 3 && nsdf[pos] > 0.0) {
        pos += 1;
    }
    while (pos < (len - 1) && nsdf[pos] <= 0.0) {
        pos += 1;
    }
    if (pos == 0) {
        pos = 1;
    }

    while (pos < len - 1) {
        if (nsdf[pos] > nsdf[pos - 1] && nsdf[pos] >= nsdf[pos + 1]) {
            if (cur_max_pos == 0) {
                cur_max_pos = pos;
            } else if (nsdf[pos] > nsdf[cur_max_pos]) {
                cur_max_pos = pos;
            }
        }
        pos += 1;
        if (pos < len - 1 && nsdf[pos] <= 0.0) {
            if (cur_max_pos > 0) {
                max_position.push_back(cur_max_pos);
                cur_max_pos = 0;
            }
            while (pos < len - 1 && nsdf[pos] <= 0.0) {
                pos += 1;
            }
        }
    }

    if (cur_max_pos > 0) {
        max_position.push_back((cur_max_pos));
    }
    return max_position;
}


double MPM::get_pitch(const std::vector<double> &buffer, size_t sample_rate) {
    std::vector<double> nsdf;
    uint64_t features = android_getCpuFeatures();
    if (features & ANDROID_CPU_ARM_FEATURE_NEON) {
        nsdf = normalized_square_diff_simd(buffer);
    }
    else {
        nsdf = normalized_square_difference(buffer);
    }
    std::vector<size_t> max_positions = peak_picking(nsdf);
    std::vector<std::pair<double, double>> estimates;

    double highest_amplitude = -std::numeric_limits<double>::infinity();
    for (size_t i: max_positions) {
        highest_amplitude = std::max(highest_amplitude, nsdf[i]);

        if (nsdf[i] > small_cutoff) {
            auto est = parabolic_interpolation(nsdf, i);
            estimates.push_back(est);
            highest_amplitude = std::max(highest_amplitude, std::get<1>(est));
        }
    }

    if (estimates.empty()) {
        return -1;
    }

    double actual_cutoff = cutoff * highest_amplitude;
    double period = 0.0;

    for (const auto &e: estimates) {
        if (std::get<1>(e) >= actual_cutoff) {
            period = std::get<0>(e);
            break;
        }
    }

    double pitch_est = static_cast<double>(sample_rate) / period;
    if (pitch_est > lower_pitch_cutoff) {
        return pitch_est;
    } else {
        return -1;
    }
}