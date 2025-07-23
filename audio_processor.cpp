#include "audio_processor.h"

// Construtor
AudioProcessor::AudioProcessor() : 
    initialized(false),
    mfcc_processor(nullptr) {
}

// Destrutor
AudioProcessor::~AudioProcessor() {
    if (initialized) {
        stop();
    }
    if (mfcc_processor) {
        // Liberar recursos MFCC se necessário
        mfcc_processor = nullptr;
    }
}

// Inicialização principal
bool AudioProcessor::begin() {
    if (initialized) {
        return true;  // Já inicializado
    }
    
    Serial.println("Initializing AudioProcessor...");
    
    // Configurar I2S
    if (!setup_i2s()) {
        Serial.println("Failed to setup I2S");
        return false;
    }
    
    // Inicializar processador MFCC se necessário
    // mfcc_processor = new MFCCProcessor(); // Exemplo
    
    initialized = true;
    Serial.println("AudioProcessor initialized successfully");
    return true;
}

// Configuração I2S
bool AudioProcessor::setup_i2s() {
    // Configuração I2S
    i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    // Configuração de pinos
    pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };
    
    // Instalar driver I2S
    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Failed to install I2S driver: %s\n", esp_err_to_name(err));
        return false;
    }
    
    // Definir pinos I2S
    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Failed to set I2S pins: %s\n", esp_err_to_name(err));
        i2s_driver_uninstall(I2S_PORT);
        return false;
    }
    
    // Iniciar I2S
    err = i2s_start(I2S_PORT);
    if (err != ESP_OK) {
        Serial.printf("Failed to start I2S: %s\n", esp_err_to_name(err));
        i2s_driver_uninstall(I2S_PORT);
        return false;
    }
    
    Serial.println("I2S configured successfully");
    return true;
}

// Captura de áudio
bool AudioProcessor::captureAudio(int16_t* buffer, size_t buffer_size) {
    if (!initialized || !buffer) {
        return false;
    }
    
    size_t bytes_read = 0;
    esp_err_t err = i2s_read(I2S_PORT, buffer, buffer_size * sizeof(int16_t), 
                            &bytes_read, portMAX_DELAY);
    
    if (err != ESP_OK) {
        Serial.printf("I2S read error: %s\n", esp_err_to_name(err));
        return false;
    }
    
    size_t samples_read = bytes_read / sizeof(int16_t);
    
    // Aplicar processamento básico
    if (samples_read > 0) {
        // Detecção de atividade de voz
        bool voice_detected = detect_voice_activity(buffer, samples_read);
        
        if (voice_detected) {
            // Aplicar gate de ruído
            apply_noise_gate(buffer, samples_read, 500.0f);
            
            // Normalizar áudio
            normalize_audio(buffer, samples_read);
            
            // Imprimir estatísticas (opcional)
            print_stats(buffer, samples_read);
        }
    }
    
    return samples_read > 0;
}

// Pré-processamento
bool AudioProcessor::preprocess(const int16_t* input, float* output, size_t length) {
    if (!input || !output || length == 0) {
        return false;
    }
    
    // Converter int16_t para float normalizado (-1.0 a 1.0)
    for (size_t i = 0; i < length; i++) {
        output[i] = static_cast<float>(input[i]) / 32768.0f;
        
        // Aplicar janela (opcional - Hamming window)
        // float window_val = 0.54f - 0.46f * cos(2.0f * PI * i / (length - 1));
        // output[i] *= window_val;
    }
    
    return true;
}

// Calcular energia do sinal
float AudioProcessor::calculate_energy(const int16_t* buffer, size_t length) {
    if (!buffer || length == 0) return 0.0f;
    
    float energy = 0.0f;
    for (size_t i = 0; i < length; i++) {
        float sample = static_cast<float>(buffer[i]);
        energy += sample * sample;
    }
    
    return energy / length;
}

// Detecção de atividade de voz
bool AudioProcessor::detect_voice_activity(const int16_t* buffer, size_t length) {
    float energy = calculate_energy(buffer, length);
    const float VAD_THRESHOLD = 10000000.0f; // Ajustar conforme necessário
    
    return energy > VAD_THRESHOLD;
}

// Aplicar gate de ruído
bool AudioProcessor::apply_noise_gate(int16_t* buffer, size_t length, float threshold) {
    if (!buffer || length == 0) return false;
    
    for (size_t i = 0; i < length; i++) {
        if (abs(buffer[i]) < threshold) {
            buffer[i] = 0;
        }
    }
    
    return true;
}

// Normalizar áudio
void AudioProcessor::normalize_audio(int16_t* buffer, size_t length) {
    if (!buffer || length == 0) return;
    
    // Encontrar valor máximo
    int16_t max_val = 0;
    for (size_t i = 0; i < length; i++) {
        int16_t abs_val = abs(buffer[i]);
        if (abs_val > max_val) {
            max_val = abs_val;
        }
    }
    
    // Normalizar se necessário
    if (max_val > 0 && max_val < 32000) {
        float scale = 32000.0f / max_val;
        for (size_t i = 0; i < length; i++) {
            buffer[i] = static_cast<int16_t>(buffer[i] * scale);
        }
    }
}

// Imprimir estatísticas
void AudioProcessor::print_stats(const int16_t* buffer, size_t length) {
    if (!buffer || length == 0) return;
    
    float energy = calculate_energy(buffer, length);
    int16_t max_val = 0, min_val = 0;
    
    for (size_t i = 0; i < length; i++) {
        if (buffer[i] > max_val) max_val = buffer[i];
        if (buffer[i] < min_val) min_val = buffer[i];
    }
    
    Serial.printf("Audio Stats - Energy: %.2f, Max: %d, Min: %d\n", 
                  energy, max_val, min_val);
}

// Parar processamento
void AudioProcessor::stop() {
    if (initialized) {
        i2s_stop(I2S_PORT);
        i2s_driver_uninstall(I2S_PORT);
        initialized = false;
        Serial.println("AudioProcessor stopped");
    }
}
