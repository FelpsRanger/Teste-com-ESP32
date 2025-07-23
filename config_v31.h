#pragma once

// LED: usei o GPIO 2, comum em placas ESP32 para LEDs embutidos
#define LED_PIN 2

// Botão: GPIO 0 costuma ser fácil de acessar e já tem suporte a wake-up por padrão
#define BUTTON_PIN 0

// UART para o módulo de reconhecimento de voz
#define VOICE_RX_PIN 16  // RX do ESP conectado ao TX do módulo
#define VOICE_TX_PIN 17  // TX do ESP conectado ao RX do módulo
