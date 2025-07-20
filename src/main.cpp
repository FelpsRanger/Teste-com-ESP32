/*
 * TinyML ESP32 - PlatformIO Implementation
 * Reconhecimento de comandos de voz otimizado para PlatformIO
 * Compatível com TensorFlow Lite Micro
 */

#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2s.h>
#include <esp_timer.h>
#include <esp_system.h>
#include <esp_log.h>

// Configurações do projeto
#include "config.h"

// TensorFlow Lite Micro (se disponível)
#ifdef ARDUINO_TENSORFLOW_LITE
    #include "TensorFlowLite_ESP32.h"
    #include "tensorflow/lite/micro/all_ops_resolver.h"
    #include "tensorflow/lite/micro/micro_error_reporter.h"
    #include "tensorflow/lite/micro/micro_interpreter.h"
    #include "tensorflow/lite/schema/schema_generated.h"
    #define TFLITE_AVAILABLE 1
#else
    #define TFLITE_AVAILABLE 0
    #warning "TensorFlow Lite não disponível - usando simulação"
#endif

// ================== DADOS DO MODELO (TEMPORÁRIO) ==================
// Substitua por seu modelo real treinado
const unsigned char model_data[] = {
    0x18, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33,
    // ... adicione os bytes do seu modelo aqui
    0x00, 0x00, 0x00, 0x00  // Modelo vazio para compilação
};
const int model_data_len = sizeof(model_data);

// ================== ESTRUTURAS DE DADOS ==================
typedef struct {
    uint32_t inference_time_us;
    float power_consumption_mw;
    float confidence;
    int predicted_class;
    bool voice_detected;
    uint32_t total_inferences;
} system_metrics_t;

// ================== CLASSE PRINCIPAL ==================
class TinyMLVoiceRecognition {
private:
    // Buffers de áudio
    int16_t* audio_buffer;
    float* preprocessed_audio;
    
    // Métricas do sistema
    system_metrics_t metrics;
    PowerMode current_power_mode;
    unsigned long last_activity_time;
    
    // TensorFlow Lite (se disponível)
    #if TFLITE_AVAILABLE
    tflite::MicroErrorReporter error_reporter;
    tflite::AllOpsResolver resolver;
    const tflite::Model* model;
    tflite::MicroInterpreter* interpreter;
    TfLiteTensor* input_tensor;
    TfLiteTensor* output_tensor;
    uint8_t* tensor_arena;
    #endif
    
public:
    TinyMLVoiceRecognition() {
        // Alocar buffers dinamicamente
        audio_buffer = (int16_t*)heap_caps_malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t), MALLOC_CAP_DMA);
        preprocessed_audio = (float*)malloc(INPUT_FEATURES * sizeof(float));
        
        #if TFLITE_AVAILABLE
        tensor_arena = (uint8_t*)heap_caps_malloc(TENSOR_ARENA_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        model = nullptr;
        interpreter = nullptr;
        input_tensor = nullptr;
        output_tensor = nullptr;
        #endif
        
        // Inicializar métricas
        memset(&metrics, 0, sizeof(system_metrics_t));
        current_power_mode = POWER_BALANCED;
        last_activity_time = millis();
    }
    
    ~TinyMLVoiceRecognition() {
        free(audio_buffer);
        free(preprocessed_audio);
        #if TFLITE_AVAILABLE
        free(tensor_arena);
        delete interpreter;
        #endif
    }
    
    bool initialize() {
        Serial.begin(SERIAL_BAUD_RATE);
        delay(1000);
        
        DEBUG_PRINTLN("🚀 Inicializando TinyML Voice Recognition...");
        
        // Verificar memória disponível
        DEBUG_PRINTF("💾 RAM livre: %d KB\n", ESP.getFreeHeap() / 1024);
        DEBUG_PRINTF("💾 PSRAM livre: %d KB\n", ESP.getFreePsram() / 1024);
        
        // Configurar pinos
        pinMode(LED_PIN, OUTPUT);
        pinMode(BUTTON_PIN, INPUT_PULLUP);
        
        // Inicializar I2S
        if (!setup_i2s()) {
            DEBUG_PRINTLN("❌ Falha na configuração do I2S");
            return false;
        }
        
        // Inicializar TensorFlow Lite
        #if TFLITE_AVAILABLE
        if (!setup_tensorflow()) {
            DEBUG_PRINTLN("❌ Falha na configuração do TensorFlow Lite");
            return false;
        }
        #else
        DEBUG_PRINTLN("⚠️ Executando em modo simulação (sem TensorFlow Lite)");
        #endif
        
        // Configurar modo de energia
        set_power_mode(POWER_BALANCED);
        
        DEBUG_PRINTLN("✅ Sistema inicializado com sucesso!");
        print_system_info();
        
        return true;
    }
    
    void run() {
        unsigned long current_time = millis();
        
        // Verificar gerenciamento de energia
        check_power_management(current_time);
        
        // Capturar e processar áudio
        if (capture_audio()) {
            if (preprocess_audio()) {
                if (run_inference()) {
                    process_results();
                    metrics.total_inferences++;
                    last_activity_time = current_time;
                }
            }
        }
        
        // Relatório periódico
        static unsigned long last_report = 0;
        if (current_time - last_report > 10000) { // 10 segundos
            print_performance_report();
            last_report = current_time;
        }
        
        delay(INFERENCE_FREQUENCY_MS);
    }
    
private:
    bool setup_i2s() {
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = AUDIO_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 4,
            .dma_buf_len = 512,
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
        
        esp_err_t result = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
        if (result != ESP_OK) {
            DEBUG_PRINTF("❌ Erro i2s_driver_install: %d\n", result);
            return false;
        }
        
        result = i2s_set_pin(I2S_NUM_0, &pin_config);
        if (result != ESP_OK) {
            DEBUG_PRINTF("❌ Erro i2s_set_pin: %d\n", result);
            return false;
        }
        
        DEBUG_PRINTF("✅ I2S configurado: %d Hz\n", AUDIO_SAMPLE_RATE);
        return true;
    }
    
    #if TFLITE_AVAILABLE
    bool setup_tensorflow() {
        // Verificar se temos tensor arena
        if (!tensor_arena) {
            DEBUG_PRINTLN("❌ Falha na alocação da tensor arena");
            return false;
        }
        
        // Carregar modelo
        model = tflite::GetModel(model_data);
        if (model->version() != TFLITE_SCHEMA_VERSION) {
            DEBUG_PRINTF("❌ Versão incompatível: %d vs %d\n", 
                        model->version(), TFLITE_SCHEMA_VERSION);
            return false;
        }
        
        // Criar interpretador
        interpreter = new tflite::MicroInterpreter(
            model, resolver, tensor_arena, TENSOR_ARENA_SIZE, &error_reporter);
        
        // Alocar tensores
        TfLiteStatus allocate_status = interpreter->AllocateTensors();
        if (allocate_status != kTfLiteOk) {
            DEBUG_PRINTLN("❌ Falha na alocação de tensores");
            return false;
        }
        
        // Obter ponteiros
        input_tensor = interpreter->input(0);
        output_tensor = interpreter->output(0);
        
        DEBUG_PRINTLN("✅ TensorFlow Lite inicializado");
        DEBUG_PRINTF("   Input: [%d, %d]\n", 
                    input_tensor->dims->data[0], input_tensor->dims->data[1]);
        DEBUG_PRINTF("   Output: [%d, %d]\n", 
                    output_tensor->dims->data[0], output_tensor->dims->data[1]);
        
        return true;
    }
    #endif
    
    void set_power_mode(PowerMode mode) {
        current_power_mode = mode;
        
        switch (mode) {
            case POWER_ACTIVE:
                setCpuFrequencyMhz(240);
                metrics.power_consumption_mw = POWER_ACTIVE_MW;
                DEBUG_PRINTLN("🔥 Modo ATIVO: 240MHz");
                break;
                
            case POWER_BALANCED:
                setCpuFrequencyMhz(160);
                metrics.power_consumption_mw = POWER_BALANCED_MW;
                DEBUG_PRINTLN("⚖️ Modo BALANCEADO: 160MHz");
                break;
                
            case POWER_ECONOMY:
                setCpuFrequencyMhz(80);
                metrics.power_consumption_mw = POWER_ECONOMY_MW;
                DEBUG_PRINTLN("🌱 Modo ECONOMIA: 80MHz");
                break;
                
            case POWER_DEEP_SLEEP:
                DEBUG_PRINTLN("😴 Deep sleep...");
                esp_deep_sleep(1000000); // 1 segundo
                break;
        }
        
        digitalWrite(LED_PIN, mode == POWER_ACTIVE ? HIGH : LOW);
    }
    
    void check_power_management(unsigned long current_time) {
        unsigned long idle_time = current_time - last_activity_time;
        
        if (digitalRead(BUTTON_PIN) == LOW) {
            // Botão pressionado - modo ativo
            if (current_power_mode != POWER_ACTIVE) {
                set_power_mode(POWER_ACTIVE);
            }
            last_activity_time = current_time;
        } else if (idle_time > ENERGY_SAVE_THRESHOLD_MS) {
            // Sem atividade - modo economia
            if (current_power_mode != POWER_ECONOMY) {
                set_power_mode(POWER_ECONOMY);
            }
        } else if (metrics.voice_detected) {
            // Voz detectada - modo balanceado
            if (current_power_mode == POWER_ECONOMY) {
                set_power_mode(POWER_BALANCED);
            }
        }
    }
    
    bool capture_audio() {
        if (!audio_buffer) return false;
        
        size_t bytes_read = 0;
        esp_err_t result = i2s_read(I2S_NUM_0, audio_buffer, 
                                   AUDIO_BUFFER_SIZE * sizeof(int16_t), 
                                   &bytes_read, pdMS_TO_TICKS(100));
        
        if (result != ESP_OK || bytes_read == 0) {
            return false;
        }
        
        // Preencher resto com zeros se necessário
        int samples_read = bytes_read / sizeof(int16_t);
        if (samples_read < AUDIO_BUFFER_SIZE) {
            memset(&audio_buffer[samples_read], 0, 
                   (AUDIO_BUFFER_SIZE - samples_read) * sizeof(int16_t));
        }
        
        return true;
    }
    
    bool preprocess_audio() {
        if (!preprocessed_audio) return false;
        
        // Detecção de atividade de voz (VAD)
        float energy = 0.0f;
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
            float sample = audio_buffer[i] / 32768.0f;
            energy += sample * sample;
            
            // Aplicar janela de Hamming e normalizar
            if (i < INPUT_FEATURES) {
                float hamming = 0.54f - 0.46f * cosf(2.0f * PI * i / (INPUT_FEATURES - 1));
                preprocessed_audio[i] = sample * hamming;
            }
        }
        
        energy = energy / AUDIO_BUFFER_SIZE;
        metrics.voice_detected = energy > VOICE_ACTIVITY_THRESHOLD;
        
        // Preencher resto com zeros
        for (int i = AUDIO_BUFFER_SIZE; i < INPUT_FEATURES; i++) {
            preprocessed_audio[i] = 0.0f;
        }
        
        return metrics.voice_detected;
    }
    
    bool run_inference() {
        unsigned long start_time = micros();
        
        #if TFLITE_AVAILABLE
        // Copiar dados para tensor
        for (int i = 0; i < INPUT_FEATURES; i++) {
            input_tensor->data.f[i] = preprocessed_audio[i];
        }
        
        // Executar inferência
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            DEBUG_PRINTLN("❌ Erro na inferência TensorFlow Lite");
            return false;
        }
        
        #else
        // Simulação de inferência
        delay(1); // Simular tempo de processamento
        
        // Gerar resultado fake baseado na energia
        float energy = 0;
        for (int i = 0; i < MIN(100, INPUT_FEATURES); i++) {
            energy += preprocessed_audio[i] * preprocessed_audio[i];
        }
        
        if (energy < 0.0001f) {
            metrics.predicted_class = CLASS_SILENCE;
            metrics.confidence = 0.9f;
        } else {
            metrics.predicted_class = (int)(energy * 10000) % 3 + 1;
            metrics.confidence = 0.75f;
        }
        #endif
        
        metrics.inference_time_us = micros() - start_time;
        return true;
    }
    
    void process_results() {
        const char* class_labels[] = {"🔇 Silêncio", "❓ Desconhecido", "✅ Sim", "❌ Não"};
        
        #if TFLITE_AVAILABLE
        // Processar saída do TensorFlow Lite
        float max_probability = output_tensor->data.f[0];
        int predicted_class = 0;
        
        for (int i = 1; i < OUTPUT_CLASSES; i++) {
            if (output_tensor->data.f[i] > max_probability) {
                max_probability = output_tensor->data.f[i];
                predicted_class = i;
            }
        }
        
        metrics.predicted_class = predicted_class;
        metrics.confidence = max_probability;
        #endif
        
        // Mostrar resultado se confiança alta
        if (metrics.confidence > CONFIDENCE_THRESHOLD) {
            DEBUG_PRINTF("🎯 %s (%.1f%%) - %lu μs\n", 
                        class_labels[metrics.predicted_class], 
                        metrics.confidence * 100, 
                        metrics.inference_time_us);
            
            // Piscar LED
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
        }
    }
    
    void print_performance_report() {
        if (metrics.total_inferences == 0) return;
        
        float throughput = 1000000.0f / metrics.inference_time_us;
        
        DEBUG_PRINTLN("\n📊 RELATÓRIO DE PERFORMANCE:");
        DEBUG_PRINTF("   🔢 Inferências: %lu\n", metrics.total_inferences);
        DEBUG_PRINTF("   ⏱️ Latência: %lu μs\n", metrics.inference_time_us);
        DEBUG_PRINTF("   🚀 Throughput: %.1f inf/s\n", throughput);
        DEBUG_PRINTF("   ⚡ Potência: %.1f mW\n", metrics.power_consumption_mw);
        DEBUG_PRINTF("   💾 RAM livre: %d KB\n", ESP.getFreeHeap() / 1024);
        DEBUG_PRINTF("   📊 PSRAM livre: %d KB\n", ESP.getFreePsram() / 1024);
    }
    
    void print_system_info() {
        DEBUG_PRINTLN("\n🔧 INFORMAÇÕES DO SISTEMA:");
        DEBUG_PRINTF("   📱 Chip: %s rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
        DEBUG_PRINTF("   🧠 CPU: %d MHz\n", getCpuFrequencyMhz());
        DEBUG_PRINTF("   💾 RAM total: %d KB\n", ESP.getHeapSize() / 1024);
        DEBUG_PRINTF("   💾 PSRAM total: %d KB\n", ESP.getPsramSize() / 1024);
        DEBUG_PRINTF("   🎤 Audio: %d Hz, %d bits\n", AUDIO_SAMPLE_RATE, AUDIO_BITS_PER_SAMPLE);
        DEBUG_PRINTF("   🧮 Tensor Arena: %d KB\n", TENSOR_ARENA_SIZE / 1024);
        DEBUG_PRINTF("   📡 TensorFlow Lite: %s\n", TFLITE_AVAILABLE ? "✅ Ativo" : "❌ Simulação");
        DEBUG_PRINTLN();
    }
};

// ================== INSTÂNCIA GLOBAL ==================
TinyMLVoiceRecognition* voice_recognition = nullptr;

// ================== SETUP E LOOP ==================
void setup() {
    // Criar instância
    voice_recognition = new TinyMLVoiceRecognition();
    
    if (!voice_recognition->initialize()) {
        Serial.println("❌ Falha na inicialização do sistema!");
        
        // Piscar LED de erro
        while(1) {
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(200);
        }
    }
}

void loop() {
    if (voice_recognition) {
        voice_recognition->run();
    }
}