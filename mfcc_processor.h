/*
 * mfcc_processor.h - Extração de características MFCC para reconhecimento de voz
 */

#ifndef MFCC_PROCESSOR_H
#define MFCC_PROCESSOR_H

#include <Arduino.h>
#include <math.h>
#include "config.h"

class MFCCProcessor {
public:
    MFCCProcessor();
    ~MFCCProcessor();
    
    // Inicializar o processador MFCC
    bool begin();
    
    // Extrair features MFCC do áudio
    bool extract_mfcc(const int16_t* audio_data, size_t length, float* mfcc_output);
    
    // Verificar se está inicializado
    bool is_ready() const { return initialized; }
    
    // Aplicar janela Hamming
    void apply_hamming_window(const int16_t* input, float* output, size_t length);
    
    // Calcular FFT (versão simplificada)
    void compute_fft(const float* input, float* magnitude, size_t length);
    
    // Aplicar filtros Mel
    void apply_mel_filters(const float* fft_magnitude, float* mel_output);
    
    // Calcular DCT para obter coeficientes MFCC
    void compute_dct(const float* mel_input, float* mfcc_output);
    
private:
    bool initialized;
    
    // Buffers internos
    float* hamming_window;
    float* fft_buffer;
    float* mel_filters;
    float* mel_output;
    
    // Parâmetros do banco de filtros Mel
    static constexpr float MEL_LOW_FREQ = 80.0f;
    static constexpr float MEL_HIGH_FREQ = 8000.0f;
    
    // Funções auxiliares
    float hz_to_mel(float hz);
    float mel_to_hz(float mel);
    void create_mel_filter_bank();
    void create_hamming_window();
    
    // FFT simplificada para uso embarcado
    void fft_radix2(float* real, float* imag, int n);
};

// Implementação inline para funções pequenas
inline float MFCCProcessor::hz_to_mel(float hz) {
    return 2595.0f * log10f(1.0f + hz / 700.0f);
}

inline float MFCCProcessor::mel_to_hz(float mel) {
    return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f);
}

#endif // MFCC_PROCESSOR_H
