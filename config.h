#ifndef CONFIG_H
#define CONFIG_H

// Hardware
#define I2S_WS_PIN 15
#define I2S_SCK_PIN 2
#define I2S_SD_PIN 4
#define LED_PIN 2
#define BUTTON_PIN 0

// Áudio
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_BUFFER_SIZE 1024
#define INPUT_FEATURES 1024
#define OUTPUT_CLASSES 4

// ML
#define CONFIDENCE_THRESHOLD 0.7f
#define VOICE_ACTIVITY_THRESHOLD 0.001f
#define INFERENCE_FREQUENCY_MS 100

// Classes
#define CLASS_SILENCE 0
#define CLASS_UNKNOWN 1
#define CLASS_YES 2
#define CLASS_NO 3

#endif