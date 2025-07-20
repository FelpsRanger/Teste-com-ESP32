#include "ml_processor.h"
#include "model_data.h"

// Construtor
MLProcessor::MLProcessor() : model(nullptr), interpreter(nullptr), input(nullptr), output(nullptr) {}

// Inicializa o modelo TinyML
bool MLProcessor::begin() {
    model = tflite::GetModel(g_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        return false;
    }

    static tflite::MicroMutableOpResolver<5> resolver;
    resolver.AddConv2D();
    resolver.AddFullyConnected();
    resolver.AddSoftmax();
    resolver.AddReshape();
    resolver.AddDepthwiseConv2D();

    static uint8_t tensor_arena[10 * 1024];
    interpreter = new tflite::MicroInterpreter(model, resolver, tensor_arena, sizeof(tensor_arena), nullptr);

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        return false;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    return true;
}

// Executa inferência no buffer de entrada
int MLProcessor::predict(const float* input_data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        input->data.f[i] = input_data[i];
    }

    if (interpreter->Invoke() != kTfLiteOk) {
        return -1;
    }

    // Encontra o índice da maior probabilidade
    int max_index = 0;
    float max_value = output->data.f[0];
    for (int i = 1; i < output->dims->data[1]; ++i) {
        if (output->data.f[i] > max_value) {
            max_value = output->data.f[i];
            max_index = i;
        }
    }