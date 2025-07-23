#include <Arduino.h>
#include "ml_processor.h"
#include "config.h"

MLProcessor::MLProcessor()
#if TFLITE_AVAILABLE
    : interpreter(nullptr),
      input_tensor(nullptr),
      output_tensor(nullptr)
#endif
{
    last_confidence = 0.0f;
    ml_initialized = false;
}

bool MLProcessor::begin() {
    Serial.println("🧠 Inicializando ML Processor...");

#if TFLITE_AVAILABLE
    const tflite::Model* model = tflite::GetModel(g_model_data);

    // Substituindo AllOpsResolver por MicroMutableOpResolver
    static tflite::MicroMutableOpResolver<4> resolver;
    resolver.AddFullyConnected();
    resolver.AddSoftmax();
    resolver.AddReshape();
    resolver.AddConv2D();

    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize);

    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("❌ Falha ao alocar tensores");
        ml_initialized = false;
        return false;
    }

    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);

    ml_initialized = true;
    Serial.println("✅ TensorFlow Lite inicializado");
    return true;
#else
    Serial.println("⚠️ TF Lite não disponível");
    ml_initialized = false;
    return false;
#endif
}

int MLProcessor::predict(const float* input_data, size_t length) {
    if (!input_data || length == 0) {
        Serial.println("❌ Dados de entrada inválidos");
        return -1;
    }

#if TFLITE_AVAILABLE
    if (ml_initialized && interpreter && input_tensor && output_tensor) {
        float* input_buffer = input_tensor->data.f;
        size_t input_size = input_tensor->bytes / sizeof(float);
        size_t copy_size = std::min(static_cast<int>(length), static_cast<int>(input_size));

        for (size_t i = 0; i < copy_size; i++) {
            input_buffer[i] = constrain(input_data[i], -1.0f, 1.0f);
        }
        for (size_t i = copy_size; i < input_size; i++) {
            input_buffer[i] = 0.0f;
        }

        if (interpreter->Invoke() != kTfLiteOk) {
            Serial.println("❌ Falha na inferência");
            int result;
            simulate_prediction(result, last_confidence);
            return result;
        }

        float* output_data = output_tensor->data.f;
        int output_size = std::min(static_cast<int>(output_tensor->bytes / sizeof(float)), OUTPUT_CLASSES);

        int predicted_class = 0;
        float max_prob = output_data[0];
        for (int i = 1; i < output_size; i++) {
            if (output_data[i] > max_prob) {
                max_prob = output_data[i];
                predicted_class = i;
            }
        }

        last_confidence = max_prob;
        return predicted_class;
    }
#endif

    int result;
    simulate_prediction(result, last_confidence);
    return result;
}

void MLProcessor::simulate_prediction(int& result, float& confidence) {
    static unsigned long last_time = 0;
    static int last_result = 0;
    unsigned long now = millis();

    if (now - last_time > random(2000, 5000)) {
        last_result = random(0, OUTPUT_CLASSES);
        last_time = now;
    }

    result = last_result;
    confidence = 0.65f + (random(0, 35) / 100.0f);
}

float MLProcessor::get_confidence() const {
    return last_confidence;
}

bool MLProcessor::is_available() const {
#if TFLITE_AVAILABLE
    return ml_initialized;
#else
    return false;
#endif
}

void MLProcessor::print_model_info() const {
    Serial.println("\n🧠 ML INFO:");
#if TFLITE_AVAILABLE
    if (ml_initialized && input_tensor && output_tensor) {
        Serial.printf("   ✅ Status: Ativo\n");
        Serial.printf("   📥 Input size: %d bytes\n", input_tensor->bytes);
        Serial.printf("   📤 Output size: %d bytes\n", output_tensor->bytes);
    } else {
        Serial.println("   ⚠️ Status: Simulação");
    }
#else
    Serial.println("   ⚠️ TF Lite não disponível");
#endif
    Serial.printf("   🎲 Confiança atual: %.1f%%\n", last_confidence * 100);
}
