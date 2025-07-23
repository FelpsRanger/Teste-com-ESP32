/*
 * audio_processor_improved.cpp - Implementação completa do processador de áudio
 */

#include "audio_processor.h"
#include "config.h"
#include <driver/i2s.h>

AudioProcessor::AudioProcessor() : 
    initialized(false),
    mfcc_processor(nullptr) {
}

AudioProcessor::~AudioProcessor() {
    if (mfcc_processor) {
        delete mfcc_processor;
    }
}

bool AudioProcessor::begin() {
    Serial.println("🎵 Inicializando Audio Processor...");
    
    // Configurar I2S
    if (!setup_i2s()) {
        Serial.println("❌ Falha ao configurar I2S");
        return false;
    }
    
    // Inicializar processador MFCC
    mfcc_processor = new MFCCProcessor();
    if (!mfcc_processor || !mfcc_processor->begin()) {
        Serial.println("❌ Falha ao inicializar MFCC");
        return false;
    }
    
    initialized = true;
    Serial.println("✅ Audio Processor inicializado!");
    return true;
}

bool AudioProcessor::setup_i2s() {
    // Configuração I2S para microfone digital (INMP441)
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = AUDIO_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // INMP441 usa 32-bit
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };

    // Instalar driver I2S
    esp_err_t result = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        Serial.printf("❌ Erro ao instalar I2S: %d\n", result);
        return false;
    }

    // Configurar pinos
    result = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (result != ESP_OK) {
        Serial.printf("❌ Erro ao configurar pinos I2S: %d\n", result);
        return false;
    }

    // Limpar buffer I2S
    i2s_zero_dma_buffer(I2S_NUM_0);
    
    return true;
}

bool AudioProcessor::captureAudio(int16_t* buffer, size_t buffer_size) {
    if (!initialized || !buffer) {
        return false;
    }
    
    // Buffer temporário para dados I2S de 32-bit
    int32_t* i2s_buffer = (int32_t*)malloc(buffer_size * sizeof(int32_t));
    if (!i2s_buffer) {
        Serial.println("❌ Erro ao alocar buffer I2S");
        return false;
    }
    
    size_t bytes_read = 0;
    esp_err_t result = i2s_read(I2S_NUM_0, 
                               i2s_buffer, 
                               buffer_size * sizeof(int32_t),
                               &bytes_read, 
                               pdMS_TO_TICKS(100));
    
    if (result != ESP_OK || bytes_read == 0) {
        free(i2s_buffer);
        return false;
    }
    
    // Converter de 32-bit para 16-bit
    size_t samples = bytes_read / sizeof(int32_t);
    for (size_t i = 0; i < samples && i < buffer_size; i++) {
        // INMP441 retorna dados nos 24 bits superiores
        buffer[i] = (int16_t)(i2s_buffer[i] >> 16);
    }
    
    free(i2s_buffer);
    return true;
}

bool AudioProcessor::preprocess(const int16_t* input, float* output, size_t length) {
    if (!initialized || !input || !output || !mfcc_processor) {
        return false;
    }
    
    return mfcc_processor->extract_mfcc(input, length, output);
}

float AudioProcessor::calculate_energy(const int16_t* buffer, size_t length) {
    if (!buffer || length == 0) {
        return 0.0f;
    }
    
    float energy = 0.0f;
    for (size_t i = 0; i < length; i++) {
        float sample = buffer[i] / 32768.0f; // Normalizar para [-1, 1]
        energy += sample * sample;
    }
    
    return energy / length; // RMS energy
}

bool AudioProcessor::detect_voice_activity(const int16_t* buffer, size_t length) {
    float energy = calculate_energy(buffer, length);
    return energy > VOICE_ACTIVITY_THRESHOLD;
}

bool AudioProcessor::apply_noise_gate(int16_t* buffer, size_t length, float threshold) {
    float energy = calculate_energy(buffer, length);
    
    if (energy < threshold) {
        // Silenciar buffer se energia estiver abaixo do threshold
        memset(buffer, 0, length * sizeof(int16_t));
        return false; // Indica que o áudio foi silenciado
    }
    
    return true; // Áudio passou pelo noise gate
}

void AudioProcessor::normalize_audio(int16_t* buffer, size_t length) {
    if (!buffer || length == 0) return;
    
    // Encontrar valor máximo absoluto
    int16_t max_val = 0;
    for (size_t i = 0; i < length; i++) {
        int16_t abs_val = abs(buffer[i]);
        if (abs_val > max_val) {
            max_val = abs_val;
        }
    }
    
    // Evitar divisão por zero
    if (max_val == 0) return;
    
    // Normalizar para usar toda a faixa dinâmica
    float scale = 32767.0f / max_val;
    for (size_t i = 0; i < length; i++) {
        buffer[i] = (int16_t)(buffer[i] * scale);
    }
}

void AudioProcessor::print_stats(const int16_t* buffer, size_t length) {
    if (!buffer || length == 0) return;
    
    float energy = calculate_energy(buffer, length);
    int16_t min_val = buffer[0];
    int16_t max_val = buffer[0];
    
    for (size_t i = 1; i < length; i++) {
        if (buffer[i] < min_val) min_val = buffer[i];
        if (buffer[i] > max_val) max_val = buffer[i];
    }
    
    Serial.printf("📊 Audio Stats: Energy=%.4f, Min=%d, Max=%d, Range=%d\n",
                  energy, min_val, max_val, max_val - min_val);
}
