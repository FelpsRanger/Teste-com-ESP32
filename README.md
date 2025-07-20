# 🎤 ESP32 TinyML Voice Recognition

Sistema inteligente de reconhecimento de comandos de voz otimizado para microcontroladores ESP32, utilizando TensorFlow Lite Micro e técnicas avançadas de gerenciamento de energia.

## 🌟 **Características Principais**

- 🧠 **TensorFlow Lite Micro** integrado para inferência em tempo real
- ⚡ **Gerenciamento de energia adaptativo** com 4 modos de operação
- 🎵 **Processamento de áudio avançado** com VAD e janela de Hamming
- 📊 **Monitoramento de performance** em tempo real
- 🔧 **Arquitetura modular** e facilmente extensível
- 💾 **Uso otimizado de memória** (PSRAM + DMA)
- 🔄 **Fallback inteligente** para desenvolvimento sem modelo

## 📋 **Especificações Técnicas**

| Componente | Especificação |
|------------|---------------|
| **Microcontrolador** | ESP32 (240MHz, 4MB Flash) |
| **Framework** | ESP-IDF via PlatformIO |
| **Áudio** | 16kHz, 16-bit, Mono (I2S) |
| **Modelo ML** | TensorFlow Lite Micro |
| **Classes** | Silêncio, Desconhecido, Sim, Não |
| **Latência** | < 100ms por inferência |
| **Consumo** | 0.1mW (sleep) - 200mW (ativo) |

## 🏗️ **Arquitetura do Sistema**

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Microfone     │───▶│  Processamento  │───▶│   Inferência    │
│     (I2S)       │    │     de Áudio    │    │   TensorFlow    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                              │                        │
                              ▼                        ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ Gerenciamento   │◄───│  Detectores de  │◄───│   Resultados    │
│  de Energia     │    │    Atividade    │    │ & Confiança     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 🚀 **Quick Start**

### **1. Requisitos**
```bash
# Instalar PlatformIO
pip install platformio

# Clonar repositório
git clone https://github.com/seu-usuario/Teste-com-ESP32.git
cd Teste-com-ESP32
```

### **2. Hardware Necessário**
- ESP32 DevKit (recomendado: ESP32-WROVER com PSRAM)
- Microfone I2S (ex: INMP441)
- LED indicador (opcional)
- Botão de controle (opcional)

### **3. Conexões**

| ESP32 Pin | Componente | Função |
|-----------|------------|---------|
| GPIO 15   | I2S WS     | Word Select |
| GPIO 2    | I2S SCK    | Serial Clock |
| GPIO 4    | I2S SD     | Serial Data |
| GPIO 2    | LED        | Status Visual |
| GPIO 0    | Botão      | Controle Manual |

### **4. Configuração**
```cpp
// include/config.h - Ajuste conforme seu hardware
#define I2S_WS_PIN 15
#define I2S_SCK_PIN 2
#define I2S_SD_PIN 4
#define AUDIO_SAMPLE_RATE 16000
```

### **5. Compilar e Upload**
```bash
# Compilar
pio run

# Upload para ESP32
pio run --target upload

# Monitor serial
pio device monitor
```

## 🔧 **Configuração Avançada**

### **Modos de Energia**

O sistema possui 4 modos adaptativos de energia:

```cpp
enum PowerMode {
    POWER_ACTIVE,      // CPU 240MHz - 200mW - Máxima performance
    POWER_BALANCED,    // CPU 160MHz - 120mW - Balanceado
    POWER_ECONOMY,     // CPU 80MHz  - 60mW  - Economia
    POWER_DEEP_SLEEP   // Sleep      - 0.1mW - Máxima economia
};
```

### **Configurações do Modelo ML**
```cpp
#define TENSOR_ARENA_SIZE (80 * 1024)  // 80KB para tensores
#define INPUT_FEATURES 1024            // Features de entrada
#define OUTPUT_CLASSES 4               // Classes de saída
#define CONFIDENCE_THRESHOLD 0.7f      // Limite de confiança
```

### **Parâmetros de Áudio**
```cpp
#define AUDIO_BUFFER_SIZE 1024         // Buffer de captura
#define VOICE_ACTIVITY_THRESHOLD 0.001f // Detecção de voz
#define INFERENCE_FREQUENCY_MS 100     // Frequência de inferência
```

## 📊 **Monitoramento e Métricas**

O sistema fornece métricas detalhadas em tempo real:

```
📊 RELATÓRIO DE PERFORMANCE:
   🔢 Inferências: 1247
   ⏱️ Latência: 45123 μs
   🚀 Throughput: 22.1 inf/s
   ⚡ Potência: 120.0 mW
   💾 RAM livre: 187 KB
   📊 PSRAM livre: 3821 KB
```

## 🤖 **Substituindo o Modelo ML**

### **1. Treinar seu Modelo**
```python
# Exemplo usando TensorFlow
import tensorflow as tf

# Seu modelo aqui
model = tf.keras.Sequential([...])

# Converter para TensorFlow Lite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

# Salvar
with open('modelo.tflite', 'wb') as f:
    f.write(tflite_model)
```

### **2. Converter para Array C++**
```bash
# Gerar header file
xxd -i modelo.tflite > data/models/model_data.h

# Editar o arquivo gerado para usar o formato correto
sed -i 's/unsigned char/const unsigned char/' data/models/model_data.h
```

### **3. Atualizar Metadados**
```json
// data/models/model_metadata.json
{
  "model_name": "meu_modelo_voz",
  "version": "2.0",
  "input_shape": [1, 1024],
  "output_classes": ["silencio", "ola", "tchau", "ajuda"],
  "sample_rate": 16000,
  "framework": "TensorFlow Lite Micro"
}
```

## 🛠️ **Desenvolvimento**

### **Estrutura do Projeto**
```
Teste-com-ESP32/
├── src/
│   ├── main.cpp              # Loop principal
│   ├── audio_processor.cpp   # Processamento de áudio
│   ├── ml_processor.cpp      # Inferência ML
│   └── power_manager.h       # Gerenciamento de energia
├── include/
│   └── config.h              # Configurações globais
├── data/models/
│   ├── model_data.h          # Modelo convertido
│   └── model_metadata.json   # Metadados do modelo
├── platformio.ini            # Configurações PlatformIO
└── README.md
```

### **Adicionando Novos Sensores**
```cpp
class SensorManager {
    void addSensor(SensorType type) {
        // Implementar novo sensor
        switch(type) {
            case SENSOR_ACCELEROMETER:
                // Configurar acelerômetro
                break;
            case SENSOR_TEMPERATURE:
                // Configurar sensor de temperatura
                break;
        }
    }
};
```

### **Debug e Logging**
```cpp
// Ativar logging detalhado
#define ENABLE_SERIAL_DEBUG 1
#define ENABLE_PERFORMANCE_LOGGING 1
#define ENABLE_AUDIO_LOGGING 1  // Cuidado: muito verbose

// Usar macros condicionais
DEBUG_PRINTF("🎯 Resultado: %s (%.1f%%)\n", label, confidence);
```

## 📈 **Performance e Otimização**

### **Benchmarks Típicos**
| Metric | ESP32 (240MHz) | ESP32 (160MHz) | ESP32 (80MHz) |
|--------|----------------|----------------|---------------|
| **Inferência** | ~45ms | ~65ms | ~120ms |
| **Throughput** | 22 inf/s | 15 inf/s | 8 inf/s |
| **Consumo** | 200mW | 120mW | 60mW |
| **Bateria** | 5h | 8h | 16h |

### **Dicas de Otimização**
1. **Memória**: Use PSRAM para tensores grandes
2. **CPU**: Ajuste frequência baseada na latência necessária
3. **Áudio**: Reduza buffer_size se latência for crítica
4. **Energia**: Implemente deep sleep entre inferências

## 🔧 **Solução de Problemas**

### **Erro: "Falha na configuração do I2S"**
```cpp
// Verificar conexões e pins
#define I2S_WS_PIN 15   // Correto para seu hardware?
#define I2S_SCK_PIN 2   // Verificar datasheet do microfone
#define I2S_SD_PIN 4    // Testar com outros pins
```

### **Erro: "Tensor arena muito pequena"**
```cpp
// Aumentar arena size
#define TENSOR_ARENA_SIZE (100 * 1024)  // De 80KB para 100KB

// Ou usar PSRAM
uint8_t* tensor_arena = (uint8_t*)heap_caps_malloc(
    TENSOR_ARENA_SIZE, MALLOC_CAP_SPIRAM
);
```

### **Baixa Precisão do Modelo**
1. **Calibrar VAD**: Ajustar `VOICE_ACTIVITY_THRESHOLD`
2. **Coletar mais dados**: Treinar com dataset maior
3. **Verificar pré-processamento**: Janela de Hamming aplicada?
4. **Ajustar confiança**: Diminuir `CONFIDENCE_THRESHOLD`

---

**⭐ Se este projeto foi útil, considere dar uma estrela no GitHub!**

```
🎤 ESP32 + 🧠 TinyML + ⚡ Energia Inteligente = 🚀 Reconhecimento de Voz Eficiente
```
