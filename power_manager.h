#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>

class PowerManager {
public:
    PowerManager();

    // Inicializa o gerenciamento de energia
    void begin();

    // Coloca o ESP32 em deep sleep por um tempo em microssegundos
    void enterDeepSleep(uint64_t sleepTimeUs);

    // Coloca o ESP32 em light sleep por um tempo em microssegundos
    void enterLightSleep(uint64_t sleepTimeUs);

    // Configura um pino para acordar o ESP32 (ex: botão ou microfone)
    void setWakeupPin(gpio_num_t pin, int level);

    // Remove todas as fontes de wakeup
    void clearWakeupSources();
};

#endif // POWER_MANAGER_H