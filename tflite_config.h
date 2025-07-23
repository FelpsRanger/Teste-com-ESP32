/*
 * tflite_config.h - Configurações para TensorFlow Lite
 */

#ifndef TFLITE_CONFIG_H
#define TFLITE_CONFIG_H

// Detectar se TensorFlow Lite está disponível
#ifdef ESP32
  // Tentar detectar TF Lite pela existência de headers
  #if __has_include("tensorflow/lite/micro/micro_interpreter.h")
    #define TFLITE_AVAILABLE 1
    
    // Detectar versão e recursos disponíveis
    #if __has_include("tensorflow/lite/micro/micro_mutable_op_resolver.h")
      #define USE_MUTABLE_RESOLVER 1
    #else
      #define USE_MUTABLE_RESOLVER 0
    #endif
    
    #if __has_include("tensorflow/lite/micro/micro_error_reporter.h")
      #define HAS_ERROR_REPORTER 1
    #else
      #define HAS_ERROR_REPORTER 0
    #endif
    
  #else
    #define TFLITE_AVAILABLE 0
    #define USE_MUTABLE_RESOLVER 0
    #define HAS_ERROR_REPORTER 0
  #endif
#else
  #define TFLITE_AVAILABLE 0
  #define USE_MUTABLE_RESOLVER 0
  #define HAS_ERROR_REPORTER 0
#endif

// Configurações de modelo (ajustar conforme seu modelo)
#ifndef INPUT_FEATURES
#define INPUT_FEATURES 1024
#endif

#ifndef OUTPUT_CLASSES
#define OUTPUT_CLASSES 4
#endif

#ifndef CONFIDENCE_THRESHOLD
#define CONFIDENCE_THRESHOLD 0.7f
#endif

#ifndef VOICE_ACTIVITY_THRESHOLD
#define VOICE_ACTIVITY_THRESHOLD 0.001f
#endif

// Configurações de áudio
#ifndef AUDIO_SAMPLE_RATE
#define AUDIO_SAMPLE_RATE 16000
#endif

#ifndef AUDIO_BUFFER_SIZE
#define AUDIO_BUFFER_SIZE 1024
#endif

// Pinos I2S (ajustar conforme seu hardware)
#ifndef I2S_SCK_PIN
#define I2S_SCK_PIN 26
#endif

#ifndef I2S_WS_PIN
#define I2S_WS_PIN 25
#endif

#ifndef I2S_SD_PIN
#define I2S_SD_PIN 33
#endif

// Outros pinos
#ifndef LED_PIN
#define LED_PIN 2
#endif

#ifndef BUTTON_PIN
#define BUTTON_PIN 0
#endif

// Dados do modelo (você precisa incluir seu modelo aqui)
#if TFLITE_AVAILABLE
// Exemplo de declaração - substitua pelos seus dados de modelo
extern const unsigned char model_data[] = {
    // Coloque aqui os bytes do seu modelo TensorFlow Lite
    // Por exemplo, exportado como array C do xxd ou similar
    0x1c, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33  // Exemplo fictício
    // ... resto dos dados do modelo
};
extern const int model_data_len = sizeof(model_data);
#endif

#endif // TFLITE_CONFIG_H
