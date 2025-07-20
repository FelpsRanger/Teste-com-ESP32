#pragma once

// Modos de energia disponíveis
enum PowerMode {
    POWER_ACTIVE,      // CPU 240MHz, WiFi ativo
    POWER_BALANCED,    // CPU 80MHz, WiFi sob demanda
    POWER_ECONOMY,     // CPU 40MHz, WiFi desligado
    POWER_DEEP_SLEEP   // Deep sleep entre inferências
};

// Inicializa o gerenciamento de energia
void power_manager_init();

// Define o modo de energia
void set_power_mode(PowerMode mode);

// Obtém o modo de energia atual
PowerMode get_power_mode();

// Atualiza o gerenciamento de energia conforme atividade
void update_power_management(bool voice_detected, bool button_pressed, unsigned long inactivity_duration);