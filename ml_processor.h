#ifndef ML_PROCESSOR_H
#define ML_PROCESSOR_H

#include <Arduino.h>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

class MLProcessor {
public:
    MLProcessor();

    // Inicializa o modelo TinyML
    bool begin();

    // Executa inferência no buffer de entrada
    int predict(const float* input_data, size_t length);

private:
    const tflite::Model* model;
    tflite::MicroInterpreter* interpreter;
    TfLiteTensor* input;
    TfLiteTensor* output;
};

#endif // ML_PROCESSOR_H