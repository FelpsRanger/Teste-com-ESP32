/*
 * config.h - Configurações do sistema (versão corrigida)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Detectar se TensorFlow Lite está disponível
#ifdef ESP32
  #if __has_include("TensorFlowLite_ESP32.h")
    #define TFLITE_AVAILABLE 1
    #include "TensorFlowLite_ESP32.h"
  #elif __has_include("TensorFlowLite.h")
    #define TFLITE_AVAILABLE 1
    #include "TensorFlowLite.h"
  #elif __has_include("tensorflow/lite/micro/micro_interpreter.h")
    #define TFLITE_AVAILABLE 1
  #else
    #define TFLITE_AVAILABLE 0
  #endif
#else
  #define TFLITE_AVAILABLE 0
#endif

// Configurações de áudio
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_BUFFER_SIZE 1024
#define INPUT_FEATURES 1024
#define OUTPUT_CLASSES 4

// Configurações de processamento
#define MFCC_COEFFICIENTS 13
#define MEL_FILTER_BANK_SIZE 26
#define FFT_SIZE 512
#define HOP_LENGTH 256

// Thresholds
#define CONFIDENCE_THRESHOLD 0.7f
#define VOICE_ACTIVITY_THRESHOLD 0.001f
#define ENERGY_THRESHOLD 0.01f

// Pinos I2S (configuração para INMP441)
#define I2S_SCK_PIN 26    // Bit Clock
#define I2S_WS_PIN 25     // Word Select (L/R)
#define I2S_SD_PIN 33     // Serial Data

// Outros pinos
#define LED_PIN 2
#define BUTTON_PIN 0

// Configurações de energia
#define CPU_FREQ_ACTIVE 240     // MHz no modo ativo
#define CPU_FREQ_LISTENING 80   // MHz no modo escutando
#define CPU_FREQ_SLEEPING 80    // MHz no modo dormindo

// Timeouts (em milissegundos)
#define ACTIVE_TIMEOUT 5000     // 5 segundos
#define LISTENING_TIMEOUT 30000 // 30 segundos
#define SLEEP_DURATION 60       // 60 segundos

// Constantes matemáticas
#ifndef PI
#define PI 3.14159265359f
#endif

#ifndef LOG2
#define LOG2 0.69314718056f
#endif

// Classes de comandos
enum VoiceCommand {
    SILENCE = 0,
    UNKNOWN = 1,
    YES = 2,
    NO = 3
};

// Labels para debug
static const char* VOICE_LABELS[] = {
    "🔇 Silêncio",
    "❓ Desconhecido", 
    "✅ Sim",
    "❌ Não"
};

#endif // CONFIG_H
