/*
 * ml_processor.h - Header corrigido para processador ML
 */

#ifndef ML_PROCESSOR_H
#define ML_PROCESSOR_H

#include <Arduino.h>
#include "tflite_config.h"

// Forward declarations condicionais
#if TFLITE_AVAILABLE
  // Incluir apenas os headers necessários que existem
  #if __has_include("tensorflow/lite/micro/micro_interpreter.h")
    #include "tensorflow/lite/micro/micro_interpreter.h"
    #include "tensorflow/lite/micro/all_ops_resolver.h"
    #include "tensorflow/lite/schema/schema_generated.h"
    
  #elif __has_include("TensorFlowLite_ESP32.h")
    #include "TensorFlowLite_ESP32.h"
  #elif __has_include("TensorFlowLite.h")
    #include "TensorFlowLite.h"
  #endif
  
  // Forward declaration para evitar problemas de compilação
  namespace tflite {
    class MicroInterpreter;
  }
#endif

class MLProcessor {
public:
    MLProcessor();
    
    // Inicializar o processador ML
    bool begin();
    
    // Executar predição
    // Retorna: classe predita (0-3) ou -1 se erro
    int predict(const float* input_data, size_t length);
    
    // Obter confiança da última predição
    float get_confidence() const;
    
    // Verificar se ML está disponível
    bool is_available() const;
    
    // Imprimir informações do modelo
    void print_model_info() const;

private:
#if TFLITE_AVAILABLE
    // Componentes TensorFlow Lite
    tflite::MicroInterpreter* interpreter;
    TfLiteTensor* input_tensor;
    TfLiteTensor* output_tensor;
    
    // Arena de memória estática
    static constexpr size_t kTensorArenaSize = 60 * 1024;
    alignas(16) uint8_t tensor_arena[kTensorArenaSize];
#endif
    
    // Estado interno
    float last_confidence;
    bool ml_initialized;
    
    // Métodos auxiliares
    void simulate_prediction(int& result, float& confidence);
};

#endif // ML_PROCESSOR_H
