#pragma once
#include <stdint.h>
#include <stddef.h>

// Detecta atividade de voz no buffer de áudio
bool detect_voice(const int16_t* audio_buffer, size_t buffer_size, float threshold);

// Pré-processa o áudio: aplica janela de Hamming e normaliza
void preprocess_audio(const int16_t* audio_buffer, size_t buffer_size, float* output_features, size_t