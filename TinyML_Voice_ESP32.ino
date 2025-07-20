/*
 * TinyML Voice Recognition - Arduino IDE Version
 * Reconhecimento de comandos de voz para ESP32
 */

#include <WiFi.h>
#include <driver/i2s.h>
#include <ArduinoJson.h>
#include "config.h"
#include "audio_processor.h"
#include "ml_processor.h"
#include "power_manager.h"

// Verificar se TensorFlow Lite está disponível
#ifdef ARDUINO_ARCH_ESP32
  #include "tensorflow/lite/micro/all_ops_resolver.h"
  #include "tensorflow/lite/micro/micro_error_reporter.h"
  #include "tensorflow/lite/micro/micro_interpreter.h"
  #include "tensorflow/lite/schema/schema_generated.h"
  #define TFLITE_AVAILABLE 1
#else
  #define TFLITE_AVAILABLE 0
  #warning "TensorFlow Lite não disponível - usando simulação"
#endif

// Buffers globais
int16_t* audio_buffer;
float* preprocessed_audio;
uint32_t inference_count = 0;
unsigned long last_activity = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("🚀 Inicializando TinyML Voice Recognition...");
  
  // Verificar memória
  Serial.printf("💾 RAM livre: %d KB\n", ESP.getFreeHeap() / 1024);
  Serial.printf("💾 PSRAM livre: %d KB\n", ESP.getFreePsram() / 1024);
  
  // Configurar pinos
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Alocar buffers
  audio_buffer = (int16_t*)ps_malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
  preprocessed_audio = (float*)malloc(INPUT_FEATURES * sizeof(float));
  
  if (!audio_buffer || !preprocessed_audio) {
    Serial.println("❌ Falha na alocação de memória!");
    error_blink();
    return;
  }
  
  // Inicializar componentes
  if (!setup_i2s()) {
    Serial.println("❌ Falha na configuração do I2S");
    error_blink();
    return;
  }
  
  #if TFLITE_AVAILABLE
  if (!ml_init()) {
    Serial.println("❌ Falha na inicialização do ML");
    error_blink();
    return;
  }
  #endif
  
  power_manager_init();
  
  Serial.println("✅ Sistema inicializado com sucesso!");
  print_system_info();
  
  last_activity = millis();
}

void loop() {
  unsigned long current_time = millis();
  
  // Verificar botão
  bool button_pressed = digitalRead(BUTTON_PIN) == LOW;
  
  // Capturar áudio
  if (capture_audio()) {
    bool voice_detected = preprocess_audio();
    
    if (voice_detected || button_pressed) {
      run_inference();
      inference_count++;
      last_activity = current_time;
    }
    
    // Atualizar gerenciamento de energia
    update_power_management(voice_detected, button_pressed, 
                           current_time - last_activity);
  }
  
  // Relatório periódico
  static unsigned long last_report = 0;
  if (current_time - last_report > 10000) {
    print_performance_report();
    last_report = current_time;
  }
  
  delay(INFERENCE_FREQUENCY_MS);
}

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
  if (result != ESP_OK) return false;
  
  result = i2s_set_pin(I2S_NUM_0, &pin_config);
  return result == ESP_OK;
}

bool capture_audio() {
  if (!audio_buffer) return false;
  
  size_t bytes_read = 0;
  esp_err_t result = i2s_read(I2S_NUM_0, audio_buffer, 
                             AUDIO_BUFFER_SIZE * sizeof(int16_t), 
                             &bytes_read, pdMS_TO_TICKS(100));
  
  return (result == ESP_OK && bytes_read > 0);
}

bool preprocess_audio() {
  if (!preprocessed_audio) return false;
  
  // VAD (Voice Activity Detection)
  float energy = 0.0f;
  for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
    float sample = audio_buffer[i] / 32768.0f;
    energy += sample * sample;
    
    // Aplicar janela de Hamming
    if (i < INPUT_FEATURES) {
      float hamming = 0.54f - 0.46f * cosf(2.0f * PI * i / (INPUT_FEATURES - 1));
      preprocessed_audio[i] = sample * hamming;
    }
  }
  
  energy = energy / AUDIO_BUFFER_SIZE;
  bool voice_detected = energy > VOICE_ACTIVITY_THRESHOLD;
  
  // Preencher resto com zeros
  for (int i = AUDIO_BUFFER_SIZE; i < INPUT_FEATURES; i++) {
    preprocessed_audio[i] = 0.0f;
  }
  
  return voice_detected;
}

void run_inference() {
  unsigned long start_time = micros();
  
  #if TFLITE_AVAILABLE
  float output_probs[OUTPUT_CLASSES];
  
  if (ml_infer(preprocessed_audio, INPUT_FEATURES, output_probs, OUTPUT_CLASSES)) {
    // Encontrar classe com maior probabilidade
    int predicted_class = 0;
    float max_prob = output_probs[0];
    
    for (int i = 1; i < OUTPUT_CLASSES; i++) {
      if (output_probs[i] > max_prob) {
        max_prob = output_probs[i];
        predicted_class = i;
      }
    }
    
    process_result(predicted_class, max_prob, micros() - start_time);
  }
  #else
  // Simulação para teste
  int fake_class = random(0, OUTPUT_CLASSES);
  float fake_confidence = 0.75f + random(0, 25) / 100.0f;
  process_result(fake_class, fake_confidence, micros() - start_time);
  #endif
}

void process_result(int predicted_class, float confidence, unsigned long inference_time) {
  const char* labels[] = {"🔇 Silêncio", "❓ Desconhecido", "✅ Sim", "❌ Não"};
  
  if (confidence > CONFIDENCE_THRESHOLD) {
    Serial.printf("🎯 %s (%.1f%%) - %lu μs\n", 
                  labels[predicted_class], confidence * 100, inference_time);
    
    // Indicação visual
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }
}

void print_performance_report() {
  Serial.println("\n📊 RELATÓRIO DE PERFORMANCE:");
  Serial.printf("   🔢 Inferências: %lu\n", inference_count);
  Serial.printf("   💾 RAM livre: %d KB\n", ESP.getFreeHeap() / 1024);
  Serial.printf("   📊 PSRAM livre: %d KB\n", ESP.getFreePsram() / 1024);
  Serial.printf("   🧠 CPU: %d MHz\n", getCpuFrequencyMhz());
}

void print_system_info() {
  Serial.println("\n🔧 INFORMAÇÕES DO SISTEMA:");
  Serial.printf("   📱 Chip: %s rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("   🧠 CPU: %d MHz\n", getCpuFrequencyMhz());
  Serial.printf("   💾 RAM total: %d KB\n", ESP.getHeapSize() / 1024);
  Serial.printf("   💾 PSRAM total: %d KB\n", ESP.getPsramSize() / 1024);
  Serial.printf("   🎤 Audio: %d Hz\n", AUDIO_SAMPLE_RATE);
}

void error_blink() {
  while(1) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}