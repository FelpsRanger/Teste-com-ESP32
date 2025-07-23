#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include <driver/i2s.h>

// Constantes de configuração
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#define I2S_PORT I2S_NUM_0
#define I2S_WS_PIN 25
#define I2S_SCK_PIN 26
#define I2S_SD_PIN 22
#define BUFFER_SIZE 1024

class AudioProcessor {
private:
    bool initialized;
    void* mfcc_processor;  // Ponteiro para processador MFCC (se necessário)
    
    // Configurações I2S
    i2s_config_t i2s_config;
    i2s_pin_config_t pin_config;
    
    // Métodos privados auxiliares
    bool setup_i2s();
    float calculate_energy(const int16_t* buffer, size_t length);
    bool detect_voice_activity(const int16_t* buffer, size_t length);
    bool apply_noise_gate(int16_t* buffer, size_t length, float threshold);
    void normalize_audio(int16_t* buffer, size_t length);
    void print_stats(const int16_t* buffer, size_t length);

public:
    // Construtor e destrutor
    AudioProcessor();
    ~AudioProcessor();
    
    // Método principal de inicialização
    bool begin();
    
    // Métodos públicos para captura e processamento
    bool captureAudio(int16_t* buffer, size_t buffer_size);
    bool preprocess(const int16_t* input, float* output, size_t length);
    
    // Métodos utilitários
    bool isInitialized() const { return initialized; }
    void stop();
};

#endif // AUDIO_PROCESSOR_H
