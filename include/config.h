#ifndef CONFIG_H
#define CONFIG_H

// ================== CONFIGURAÇÕES DE HARDWARE ==================
#define I2S_WS_PIN 15
#define I2S_SCK_PIN 2
#define I2S_SD_PIN 4
#define LED_PIN 2
#define BUTTON_PIN 0

// ================== CONFIGURAÇÕES DE ÁUDIO ==================
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_BUFFER_SIZE 1024
#define AUDIO_CHANNELS 1
#define AUDIO_BITS_PER_SAMPLE 16

// Configurações de processamento
#define INFERENCE_FREQUENCY_MS 100
#define ENERGY_SAVE_THRESHOLD_MS 5000
#define VOICE_ACTIVITY_THRESHOLD 0.001f

// ================== CONFIGURAÇÕES DO MODELO ML ==================
#define TENSOR_ARENA_SIZE (80 * 1024)  // 80KB para o modelo
#define INPUT_FEATURES 1024
#define OUTPUT_CLASSES 4
#define CONFIDENCE_THRESHOLD 0.7f

// Classes de saída
#define CLASS_SILENCE 0
#define CLASS_UNKNOWN 1
#define CLASS_YES 2
#define CLASS_NO 3

// ================== CONFIGURAÇÕES DE ENERGIA ==================
enum PowerMode {
    POWER_ACTIVE = 0,      // CPU 240MHz, WiFi ativo
    POWER_BALANCED = 1,    // CPU 160MHz, WiFi sob demanda  
    POWER_ECONOMY = 2,     // CPU 80MHz, WiFi desligado
    POWER_DEEP_SLEEP = 3   // Deep sleep entre inferências
};

// Consumos de energia estimados (mW)
#define POWER_ACTIVE_MW 200.0f
#define POWER_BALANCED_MW 120.0f
#define POWER_ECONOMY_MW 60.0f
#define POWER_DEEP_SLEEP_MW 0.1f

// ================== CONFIGURAÇÕES DE DEBUG ==================
#define ENABLE_SERIAL_DEBUG 1
#define ENABLE_PERFORMANCE_LOGGING 1
#define ENABLE_AUDIO_LOGGING 0
#define SERIAL_BAUD_RATE 115200

// ================== CONFIGURAÇÕES DE WIFI ==================
#define WIFI_SSID "SeuWiFi"
#define WIFI_PASSWORD "SuaSenha"
#define ENABLE_WIFI_REPORTING 0

// ================== MACROS ÚTEIS ==================
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Logging condicional
#if ENABLE_SERIAL_DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(fmt, ...)
#endif

#endif // CONFIG_H