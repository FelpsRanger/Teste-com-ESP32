/*
 * TinyML Voice Recognition - Compact & Optimized Version
 * Voice recognition system with smart power management
 */

#include <WiFi.h>
#include <driver/i2s.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include <esp_pm.h>
#include "config.h"
#include "audio_processor.h"
#include "ml_processor.h"
#include "tflite_config.h"

// Prototypes
void error_loop();
bool setup_i2s();
void setup_power();
void handle_button();
void handle_active();
void handle_listening();
void handle_sleep();
void check_transitions();
bool quick_vad();
bool capture_audio();
bool preprocess_audio();
void ensure_full_buffers();
void free_full_buffers();
void run_inference();
void process_result(int predicted_class, float confidence, unsigned long time_us);

// Power states
enum PowerMode { ACTIVE, LISTENING, SLEEPING };

// Global configuration
struct Config {
  uint32_t active_timeout = 5000;
  uint32_t listen_timeout = 30000;
  uint32_t sleep_duration = 60;
  uint32_t vad_interval = 100;
  float vad_threshold = 0.001f;
  uint8_t low_cpu_freq = 80;
} config;

// Global variables
PowerMode power_mode = LISTENING;
unsigned long last_activity = 0;
bool voice_detected = false;

int16_t* audio_small = nullptr;
int16_t* audio_full = nullptr;
float* features = nullptr;
bool full_buffers_ready = false;

MLProcessor ml_processor;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("🚀 TinyML Voice - Starting...");

  setup_power();

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
  esp_sleep_enable_timer_wakeup(config.sleep_duration * 1000000ULL);

  audio_small = (int16_t*)malloc(256 * sizeof(int16_t));
  if (!audio_small) {
    Serial.println("❌ Error: Minimum buffer");
    error_loop();
  }

  if (!setup_i2s()) {
    Serial.println("❌ Error: I2S setup failed");
    error_loop();
  }

  if (!ml_processor.begin()) {
    Serial.println("⚠️ ML unavailable - simulation mode");
  }

  Serial.println("✅ System ready!");
  last_activity = millis();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    handle_button();
    return;
  }

  switch (power_mode) {
    case ACTIVE: handle_active(); break;
    case LISTENING: handle_listening(); break;
    case SLEEPING: handle_sleep(); break;
  }

  check_transitions();
  delay(config.vad_interval);
}
void handle_active() {
  // Colocar CPU na frequência máxima
  if (getCpuFrequencyMhz() != 240) {
    setCpuFrequencyMhz(240);
  }

  // Alocar buffers completos
  ensure_full_buffers();

  // Capturar e processar áudio
  if (capture_audio() && preprocess_audio()) {
    run_inference();
    last_activity = millis();
  }
}

void setup_power() {
  esp_pm_config_esp32s3_t pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = config.low_cpu_freq,
    .light_sleep_enable = true
  };

  esp_pm_configure(&pm_config);
  setCpuFrequencyMhz(config.low_cpu_freq);
  WiFi.mode(WIFI_OFF);
  btStop();

  Serial.printf("🔋 CPU: %d MHz\n", config.low_cpu_freq);
}

bool setup_i2s() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = AUDIO_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
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

  return (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) == ESP_OK &&
          i2s_set_pin(I2S_NUM_0, &pin_config) == ESP_OK);
}
void handle_listening() {
  if (getCpuFrequencyMhz() != config.low_cpu_freq) {
    setCpuFrequencyMhz(config.low_cpu_freq);
  }
  free_full_buffers();
  if (quick_vad()) {
    voice_detected = true;
    last_activity = millis();
  }
}

void handle_sleep() {
  Serial.println("💤 Sleep mode...");
  free_full_buffers();
  setCpuFrequencyMhz(80);
  esp_sleep_enable_timer_wakeup(5000000ULL); // 5s
  esp_light_sleep_start();

  if (quick_vad() || digitalRead(BUTTON_PIN) == LOW) {
    power_mode = ACTIVE;
    last_activity = millis();
  }
}

void ensure_full_buffers() {
  if (full_buffers_ready) return;
  audio_full = (int16_t*)ps_malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
  features = (float*)malloc(INPUT_FEATURES * sizeof(float));
  if (audio_full && features) {
    full_buffers_ready = true;
    Serial.println("💾 Buffers alocados");
  }
}

void free_full_buffers() {
  if (!full_buffers_ready) return;
  if (audio_full) { free(audio_full); audio_full = nullptr; }
  if (features) { free(features); features = nullptr; }
  full_buffers_ready = false;
}

bool quick_vad() {
  if (!audio_small) return false;
  size_t bytes_read = 0;
  if (i2s_read(I2S_NUM_0, audio_small, 256 * sizeof(int16_t), &bytes_read, pdMS_TO_TICKS(10)) != ESP_OK) return false;

  float energy = 0.0f;
  int samples = bytes_read / sizeof(int16_t);
  for (int i = 0; i < samples; i++) {
    float sample = audio_small[i] / 32768.0f;
    energy += sample * sample;
  }
  return (energy / samples) > config.vad_threshold;
}

bool capture_audio() {
  if (!audio_full) return false;
  size_t bytes_read = 0;
  return (i2s_read(I2S_NUM_0, audio_full, AUDIO_BUFFER_SIZE * sizeof(int16_t),
                   &bytes_read, pdMS_TO_TICKS(100)) == ESP_OK && bytes_read > 0);
}

bool preprocess_audio() {
  if (!features || !audio_full) return false;
  float energy = 0.0f;

  for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
    float sample = audio_full[i] / 32768.0f;
    energy += sample * sample;
    if (i < INPUT_FEATURES) {
      float hamming = 0.54f - 0.46f * cosf(2.0f * PI * i / (INPUT_FEATURES - 1));
      features[i] = sample * hamming;
    }
  }

  for (int i = AUDIO_BUFFER_SIZE; i < INPUT_FEATURES; i++) {
    features[i] = 0.0f;
  }

  return (energy / AUDIO_BUFFER_SIZE) > VOICE_ACTIVITY_THRESHOLD;
}

void run_inference() {
  unsigned long start = micros();
  #ifdef TFLITE_AVAILABLE
    int result = ml_processor.predict(features, INPUT_FEATURES);
    float confidence = ml_processor.get_confidence();
  #else
    int result = random(0, OUTPUT_CLASSES);
    float confidence = 0.75f + random(0, 25) / 100.0f;
  #endif
  process_result(result, confidence, micros() - start);
}

void process_result(int predicted_class, float confidence, unsigned long time_us) {
  const char* labels[] = {"🔇 Silêncio", "❓ Desconhecido", "✅ Sim", "❌ Não"};
  if (confidence > CONFIDENCE_THRESHOLD) {
    Serial.printf("🎯 %s (%.1f%%) - %lu μs\n", labels[predicted_class], confidence * 100, time_us);
    digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW);
  }
}

void handle_button() {
  Serial.println("👆 Botão pressionado!");
  power_mode = ACTIVE;
  last_activity = millis();
  voice_detected = false;
  digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW);
}

void check_transitions() {
  unsigned long idle_time = millis() - last_activity;
  switch (power_mode) {
    case ACTIVE:
      if (idle_time > config.active_timeout) {
        power_mode = LISTENING;
        Serial.println("🔄 ATIVO → ESCUTANDO");
      }
      break;
    case LISTENING:
      if (voice_detected) {
        power_mode = ACTIVE;
        voice_detected = false;
        Serial.println("🔄 ESCUTANDO → ATIVO");
      } else if (idle_time > config.listen_timeout) {
        power_mode = SLEEPING;
        Serial.println("🔄 ESCUTANDO → DORMINDO");
      }
      break;
    case SLEEPING:
      if (voice_detected || digitalRead(BUTTON_PIN) == LOW) {
        power_mode = ACTIVE;
        voice_detected = false;
        Serial.println("🔄 DORMINDO → ATIVO");
      } else {
        power_mode = LISTENING;
        Serial.println("🔄 DORMINDO → ESCUTANDO");
      }
      break;
  }
}

void error_loop() {
  while (true) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}
