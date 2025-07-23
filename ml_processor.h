/*
 * ml_processor.h - Header para processador ML
 */

#ifndef ML_PROCESSOR_H
#define ML_PROCESSOR_H

#include <Arduino.h>

// Verificar se TensorFlow Lite está disponível
#ifdef ESP32
  // Definir se TF Lite está disponível baseado na biblioteca instalada
  #define TFLITE_AVAILABLE 1
#else
  #define TFLITE_AVAILABLE 0
#endif

#if TFLITE_AVAILABLE
  #include "tensorflow/lite/c/common.h"
  
  // Forward declarations para evitar includes pesados no header
  namespace tflite {
    class MicroInterpreter;
  }
#endif

class MLProcessor {
public:
    MLProcessor();
    ~MLProcessor();
    
    // Inicializar o processador ML
    bool begin();
    
    // Executar predição
    // Retorna: classe predita (-1 se erro)
    int predict(const float* input_data, size_t length);
    
    // Obter confiança da última predição
    float get_confidence() const;
    
    // Verificar se ML está disponível
    bool is_available() const;
    
    // Imprimir informações do modelo
    void print_model_info() const;

private:
#if TFLITE_AVAILABLE
    // Ponteiros para componentes TF Lite
    tflite::MicroInterpreter* interpreter;
    
    // Tensores de entrada e saída
    TfLiteTensor* input_tensor;
    TfLiteTensor* output_tensor;
    
    // Arena de memória para tensores
    uint8_t* tensor_arena;
    
    // Dados do modelo (deve ser definido externamente)
    extern const unsigned char model_data[];
    extern const int model_data_len;
#endif
    
    // Última confiança calculada
    float last_confidence;
};

#endif // ML_PROCESSOR_H
