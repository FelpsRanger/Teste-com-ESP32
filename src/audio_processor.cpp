#include "audio_processor.h"
#include <Arduino.h>
#include <math.h>

// Função simples de VAD (Voice Activity Detection)
bool detect_voice(const int16_t* audio_buffer, size_t buffer_size, float threshold) {
    float energy = 0.0f;
    for (size_t i = 0; i < buffer_size; i++) {
        float sample = audio_buffer[i] / 32768.0f;
        energy += sample * sample;
    }
    energy /= buffer_size;
    return energy > threshold;
}

// Função de pré-processamento: janela de Hamming e normalização
void preprocess_audio(const int16_t* audio_buffer, size_t buffer_size, float* output_features, size_t feature_size) {
    for (size_t i = 0; i < feature_size && i < buffer_size; i++) {
        float hamming = 0.54f - 0.46f * cosf(2.0f * PI * i / (feature_size - 1));
        output_features[i] = (audio_buffer[i] / 32768.0f) * hamming;
    }
    // Preenche o resto com zeros se feature_size > buffer_size
    for (size_t i = buffer_size; i < feature_size; i++) {
        output_features[i] = 0.0f;
    }