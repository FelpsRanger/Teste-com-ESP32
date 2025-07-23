/*
 * config.h - Configurações do sistema
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Detectar se TensorFlow Lite está disponível
#ifdef ESP32
  // Tentar diferentes formas de detectar TF Lite
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

// Thresholds
#define CONFIDENCE_THRESHOLD 0.7f
#define VOICE_ACTIVITY_THRESHOLD 0.001f

// Pinos I2S
#define I2S_SCK_PIN 26    // Bit Clock
#define I2S_WS_PIN 25     // Word Select
#define I2S_SD_PIN 33     // Serial Data

// Outros pinos
#define LED_PIN 2
#define BUTTON_PIN 0

// Constantes matemáticas
#ifndef PI
#define PI 3.14159265359f
#endif

// Modelo de dados simulado (substitua pelo seu modelo real)
#if TFLITE_AVAILABLE
// Modelo mínimo válido do TensorFlow Lite (placeholder)
const unsigned char g_model_data[] PROGMEM = {
  0x1c, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33, 0x00, 0x00, 0x12, 0x00,
  0x1c, 0x00, 0x04, 0x00, 0x08, 0x00, 0x0c, 0x00, 0x10, 0x00, 0x14, 0x00,
  0x12, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00
  // ... resto do modelo seria aqui
};
const int g_model_data_len = sizeof(g_model_data);
#endif

#endif // CONFIG_H
