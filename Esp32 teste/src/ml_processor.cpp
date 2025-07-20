#include "ml_processor.h"
#include "model_data.h"
#include <Arduino.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Tamanho da arena de tensores (ajuste conforme necessário)
#define TENSOR_ARENA_SIZE (60 * 1024)
static uint8_t tensor_arena[TENSOR_ARENA_SIZE];

// Ponteiros para componentes do TensorFlow Lite Micro
static tflite::MicroErrorReporter micro_error_reporter;
static tflite::AllOpsResolver resolver;
static const tflite::Model* model = nullptr;
static tflite::MicroInterpreter* interpreter = nullptr;
static TfLiteTensor* input_tensor = nullptr;
static TfLiteTensor* output_tensor = nullptr;

// Inicializa o modelo ML
bool ml_init() {
    model = tflite::GetModel(model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.printf("❌ Versão do modelo incompatível: %d vs %d\n", model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    interpreter = new tflite::MicroInterpreter(model, resolver, tensor_arena, TENSOR_ARENA_SIZE, &micro_error_reporter);
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("❌ Falha na alocação de tensores");
        return false;
    }
    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);
    return true;
}

// Executa inferência
bool ml_infer(const float* input_features, size_t feature_size, float* output_probs, size_t output_size) {
    if (!input_tensor || !output_tensor) return false;
    for (size_t i = 0; i < feature_size; i++) {
        input_tensor->data.f[i] = input_features[i];
    }
    if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("❌ Erro na inferência");
        return false;
    }
    for (size_t i = 0; i < output_size; i++) {
        output_probs[i] = output_tensor->data.f[i];
    }
    return