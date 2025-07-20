/*
 * TinyML Voice Recognition - Versão Otimizada para Economia de Energia
 * Sistema de reconhecimento de voz com gestão inteligente de energia
 */

#include <WiFi.h>
#include <driver/i2s.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include <esp_pm.h>
#include <ArduinoJson.h>
#include "config.h"
#include "audio_processor.h"
#include "ml_processor.h"
#include "power_manager.h"

// Estados de energia
enum PowerState {
  POWER_ACTIVE,           // Processamento ativo
  POWER_LISTENING,        // Escutando por comandos
  POWER_LIGHT_SLEEP,      // Sleep leve
  POWER_DEEP_SLEEP        // Sleep profundo
};

// Configurações de energia
struct PowerConfig {
  uint32_t active_timeout_ms = 5000;        // Timeout para modo ativo
  uint32_t listening_timeout_ms = 30000;    // Timeout para modo listening
  uint32_t deep_sleep_duration_s = 60;      // Duração do deep sleep
  uint32_t vad_check_interval_ms = 50;      // Intervalo de checagem VAD
  float vad_sensitivity = 0.0005f;          // Sensibilidade VAD reduzida
  uint8_t cpu_freq_mhz = 80;                // CPU freq reduzida
} power_config;

// Variáveis globais de energia
PowerState current_power_state = POWER_LISTENING;
unsigned long last_voice_activity = 0;
unsigned long last_user_interaction = 0;
unsigned long last_vad_check = 0;
bool voice_detected_flag = false;
uint32_t wake_count = 0;

// Buffers otimizados
int16_t* audio_buffer_small;  // Buffer menor para VAD
int16_t* audio_buffer_full;   // Buffer completo para inferência
float* preprocessed_audio;
bool buffers_allocated = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("🚀 Inicializando Sistema de Baixo Consumo...");
  
  // Configurar gerenciamento de energia avançado
  setup_power_management();
  
  // Configurar pinos
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Configurar wake-up sources
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // Botão
  esp_sleep_enable_timer_wakeup(power_config.deep_sleep_duration_s * 1000000ULL);
  
  // Alocar apenas buffers essenciais inicialmente
  allocate_minimal_buffers();
  
  if (!setup_i2s_low_power()) {
    Serial.println("❌ Falha na configuração do I2S");
    error_blink();
    return;
  }
  
  Serial.println("✅ Sistema inicializado - Modo Economia Ativa!");
  print_power_info();
  
  last_voice_activity = millis();
  last_user_interaction = millis();
}

void loop() {
  unsigned long current_time = millis();
  
  // Verificar botão (sempre prioritário)
  bool button_pressed = digitalRead(BUTTON_PIN) == LOW;
  if (button_pressed) {
    handle_user_interaction();
    return;
  }
  
  // Máquina de estados de energia
  switch (current_power_state) {
    case POWER_ACTIVE:
      handle_active_mode();
      break;
      
    case POWER_LISTENING:
      handle_listening_mode();
      break;
      
    case POWER_LIGHT_SLEEP:
      handle_light_sleep_mode();
      break;
      
    case POWER_DEEP_SLEEP:
      handle_deep_sleep_mode();
      break;
  }
  
  // Verificar transições de estado
  check_power_state_transitions();
  
  // Relatório periódico (apenas em modo ativo)
  static unsigned long last_report = 0;
  if (current_power_state == POWER_ACTIVE && 
      current_time - last_report > 30000) {
    print_power_report();
    last_report = current_time;
  }
}

void setup_power_management() {
  // Configurar CPU frequency dinâmica
  esp_pm_config_esp32_t pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = power_config.cpu_freq_mhz,
    .light_sleep_enable = true
  };
  esp_pm_configure(&pm_config);
  
  // Reduzir CPU para economia inicial
  setCpuFrequencyMhz(power_config.cpu_freq_mhz);
  
  // Desabilitar WiFi e Bluetooth
  WiFi.mode(WIFI_OFF);
  btStop();
  
  Serial.printf("🔋 CPU reduzida para %d MHz\n", power_config.cpu_freq_mhz);
}

void allocate_minimal_buffers() {
  // Buffer pequeno para VAD (apenas 256 samples)
  audio_buffer_small = (int16_t*)malloc(256 * sizeof(int16_t));
  
  if (!audio_buffer_small) {
    Serial.println("❌ Falha na alocação de buffer mínimo!");
    error_blink();
    return;
  }
  
  Serial.println("💾 Buffers mínimos alocados");
}

void allocate_full_buffers() {
  if (buffers_allocated) return;
  
  // Alocar buffers completos apenas quando necessário
  audio_buffer_full = (int16_t*)ps_malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
  preprocessed_audio = (float*)malloc(INPUT_FEATURES * sizeof(float));
  
  if (!audio_buffer_full || !preprocessed_audio) {
    Serial.println("❌ Falha na alocação de buffers completos!");
    return;
  }
  
  buffers_allocated = true;
  Serial.println("💾 Buffers completos alocados");
}

void deallocate_full_buffers() {
  if (!buffers_allocated) return;
  
  if (audio_buffer_full) {
    free(audio_buffer_full);
    audio_buffer_full = nullptr;
  }
  
  if (preprocessed_audio) {
    free(preprocessed_audio);
    preprocessed_audio = nullptr;
  }
  
  buffers_allocated = false;
  Serial.println("💾 Buffers completos liberados");
}

bool setup_i2s_low_power() {
  // Configuração I2S otimizada para baixo consumo
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = AUDIO_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,  // Reduzido para economia
    .dma_buf_len = 256,  // Buffer menor
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

void handle_active_mode() {
  unsigned long current_time = millis();
  
  // Aumentar CPU para processamento
  if (getCpuFrequencyMhz() != 240) {
    setCpuFrequencyMhz(240);
  }
  
  // Alocar buffers completos se necessário
  allocate_full_buffers();
  
  // Capturar e processar áudio completo
  if (capture_full_audio()) {
    bool voice_detected = preprocess_audio_full();
    
    if (voice_detected) {
      run_inference();
      last_voice_activity = current_time;
    }
  }
  
  delay(power_config.vad_check_interval_ms);
}

void handle_listening_mode() {
  unsigned long current_time = millis();
  
  // CPU em frequência reduzida
  if (getCpuFrequencyMhz() != power_config.cpu_freq_mhz) {
    setCpuFrequencyMhz(power_config.cpu_freq_mhz);
  }
  
  // Liberar buffers completos para economia
  deallocate_full_buffers();
  
  // Verificar VAD apenas periodicamente
  if (current_time - last_vad_check > power_config.vad_check_interval_ms) {
    if (quick_vad_check()) {
      voice_detected_flag = true;
      last_voice_activity = current_time;
    }
    last_vad_check = current_time;
  }
  
  delay(power_config.vad_check_interval_ms);
}

void handle_light_sleep_mode() {
  Serial.println("😴 Entrando em Light Sleep...");
  
  // Preparar para sleep
  deallocate_full_buffers();
  setCpuFrequencyMhz(80);
  
  // Light sleep por período curto
  esp_sleep_enable_timer_wakeup(5000000ULL); // 5 segundos
  esp_light_sleep_start();
  
  wake_count++;
  Serial.printf("⏰ Wake #%lu - Verificando atividade\n", wake_count);
  
  // Verificação rápida após acordar
  if (quick_vad_check() || digitalRead(BUTTON_PIN) == LOW) {
    current_power_state = POWER_ACTIVE;
    last_voice_activity = millis();
  }
}

void handle_deep_sleep_mode() {
  Serial.println("💤 Entrando em Deep Sleep...");
  print_power_report();
  
  // Desabilitar I2S para economia máxima
  i2s_driver_uninstall(I2S_NUM_0);
  
  // Configurar RTC GPIO para wake-up
  rtc_gpio_pullup_en(GPIO_NUM_0);
  rtc_gpio_pulldown_dis(GPIO_NUM_0);
  
  // Deep sleep
  esp_deep_sleep_start();
}

bool quick_vad_check() {
  if (!audio_buffer_small) return false;
  
  size_t bytes_read = 0;
  esp_err_t result = i2s_read(I2S_NUM_0, audio_buffer_small, 
                             256 * sizeof(int16_t), 
                             &bytes_read, pdMS_TO_TICKS(10));
  
  if (result != ESP_OK || bytes_read == 0) return false;
  
  // VAD simples e rápido
  float energy = 0.0f;
  int samples = bytes_read / sizeof(int16_t);
  
  for (int i = 0; i < samples; i++) {
    float sample = audio_buffer_small[i] / 32768.0f;
    energy += sample * sample;
  }
  
  energy = energy / samples;
  return energy > power_config.vad_sensitivity;
}

bool capture_full_audio() {
  if (!audio_buffer_full) return false;
  
  size_t bytes_read = 0;
  esp_err_t result = i2s_read(I2S_NUM_0, audio_buffer_full, 
                             AUDIO_BUFFER_SIZE * sizeof(int16_t), 
                             &bytes_read, pdMS_TO_TICKS(100));
  
  return (result == ESP_OK && bytes_read > 0);
}

bool preprocess_audio_full() {
  if (!preprocessed_audio || !audio_buffer_full) return false;
  
  // Mesmo processamento do código original
  float energy = 0.0f;
  for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
    float sample = audio_buffer_full[i] / 32768.0f;
    energy += sample * sample;
    
    if (i < INPUT_FEATURES) {
      float hamming = 0.54f - 0.46f * cosf(2.0f * PI * i / (INPUT_FEATURES - 1));
      preprocessed_audio[i] = sample * hamming;
    }
  }
  
  energy = energy / AUDIO_BUFFER_SIZE;
  
  for (int i = AUDIO_BUFFER_SIZE; i < INPUT_FEATURES; i++) {
    preprocessed_audio[i] = 0.0f;
  }
  
  return energy > VOICE_ACTIVITY_THRESHOLD;
}

void handle_user_interaction() {
  Serial.println("👆 Interação do usuário detectada!");
  
  current_power_state = POWER_ACTIVE;
  last_user_interaction = millis();
  last_voice_activity = millis();
  voice_detected_flag = false;
  
  // Feedback imediato
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
}

void check_power_state_transitions() {
  unsigned long current_time = millis();
  unsigned long idle_time = current_time - last_voice_activity;
  unsigned long user_idle_time = current_time - last_user_interaction;
  
  switch (current_power_state) {
    case POWER_ACTIVE:
      if (idle_time > power_config.active_timeout_ms) {
        current_power_state = POWER_LISTENING;
        Serial.println("🔄 Transição: ATIVO → ESCUTANDO");
      }
      break;
      
    case POWER_LISTENING:
      if (voice_detected_flag) {
        current_power_state = POWER_ACTIVE;
        voice_detected_flag = false;
        Serial.println("🔄 Transição: ESCUTANDO → ATIVO");
      } else if (idle_time > power_config.listening_timeout_ms) {
        current_power_state = POWER_LIGHT_SLEEP;
        Serial.println("🔄 Transição: ESCUTANDO → LIGHT SLEEP");
      }
      break;
      
    case POWER_LIGHT_SLEEP:
      // Após alguns ciclos de light sleep, ir para deep sleep
      if (wake_count > 5 && user_idle_time > 300000) { // 5 min sem interação
        current_power_state = POWER_DEEP_SLEEP;
        Serial.println("🔄 Transição: LIGHT SLEEP → DEEP SLEEP");
      } else if (voice_detected_flag) {
        current_power_state = POWER_ACTIVE;
        voice_detected_flag = false;
        wake_count = 0;
        Serial.println("🔄 Transição: LIGHT SLEEP → ATIVO");
      } else {
        current_power_state = POWER_LISTENING;
        Serial.println("🔄 Transição: LIGHT SLEEP → ESCUTANDO");
      }
      break;
  }
}

void run_inference() {
  // Mesmo código de inferência do original
  unsigned long start_time = micros();
  
  #if TFLITE_AVAILABLE
  // ... código ML original ...
  #else
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
    
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }
}

void print_power_report() {
  const char* state_names[] = {"ATIVO", "ESCUTANDO", "LIGHT SLEEP", "DEEP SLEEP"};
  
  Serial.println("\n🔋 RELATÓRIO DE ENERGIA:");
  Serial.printf("   🔋 Estado: %s\n", state_names[current_power_state]);
  Serial.printf("   🧠 CPU: %d MHz\n", getCpuFrequencyMhz());
  Serial.printf("   💾 RAM livre: %d KB\n", ESP.getFreeHeap() / 1024);
  Serial.printf("   ⏰ Wakes: %lu\n", wake_count);
  Serial.printf("   🎤 Última atividade: %lu ms atrás\n", 
                millis() - last_voice_activity);
  Serial.printf("   💾 Buffers completos: %s\n", 
                buffers_allocated ? "Alocados" : "Liberados");
}

void print_power_info() {
  Serial.println("\n🔋 CONFIGURAÇÃO DE ENERGIA:");
  Serial.printf("   ⏱️  Timeout ativo: %lu ms\n", power_config.active_timeout_ms);
  Serial.printf("   👂 Timeout escuta: %lu ms\n", power_config.listening_timeout_ms);
  Serial.printf("   💤 Deep sleep: %lu s\n", power_config.deep_sleep_duration_s);
  Serial.printf("   🎤 VAD interval: %lu ms\n", power_config.vad_check_interval_ms);
  Serial.printf("   🎛️  Sensibilidade: %.4f\n", power_config.vad_sensitivity);
}

void error_blink() {
  while(1) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}
