#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <Arduino.h>

class AudioProcessor {
public:
    AudioProcessor();

    // Inicializa o microfone ou ADC
    void begin();

    // Captura amostras de áudio para o buffer fornecido
    bool captureAudio(int16_t* buffer, size_t buffer_size);

    // Pré-processa o áudio para o modelo (ex: normalização, extração de MFCC)
    void preprocess(const int16_t* input, float* output, size_t length);
};

#endif // AUDIO_PROCESSOR_H