# 🎙️ TinyML Voice Recognition com ESP32-S3

<div align="center">

![ESP32-S3](https://img.shields.io/badge/ESP32--S3-Espressif-red?style=for-the-badge&logo=espressif)
![TensorFlow](https://img.shields.io/badge/TensorFlow-Lite-orange?style=for-the-badge&logo=tensorflow)
![Arduino](https://img.shields.io/badge/Arduino-IDE-blue?style=for-the-badge&logo=arduino)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**Sistema de reconhecimento de comandos de voz embarcado usando TinyML no ESP32-S3**

*Detecção inteligente de atividade vocal • Gerenciamento dinâmico de energia • Inferência em tempo real*

</div>

---

## 📋 Índice

- [Visão Geral](#-visão-geral)
- [Características](#-características)
- [Hardware Necessário](#-hardware-necessário)
- [Instalação](#-instalação)
- [Configuração](#-configuração)
- [Como Usar](#-como-usar)
- [Treinamento do Modelo](#-treinamento-do-modelo)
- [Arquitetura do Sistema](#-arquitetura-do-sistema)
- [API e Configurações](#-api-e-configurações)
- [Troubleshooting](#-troubleshooting)
- [Contribuição](#-contribuição)
- [Licença](#-licença)

---

## 🎯 Visão Geral

Este projeto implementa um sistema completo de reconhecimento de voz embarcado utilizando **TinyML** no microcontrolador **ESP32-S3**. O sistema é capaz de reconhecer comandos de voz como "Sim", "Não" e detectar silêncio/ruído, tudo isso executando localmente sem necessidade de conexão com a internet.

### 🎨 Demo

```
🚀 TinyML Voice - Starting...
🔋 CPU: 80 MHz
✅ System ready!
🔄 ESCUTANDO → ATIVO
🎯 ✅ Sim (87.3%) - 1240 μs
🔄 ATIVO → ESCUTANDO
💤 Sleep mode...
```

---

## ✨ Características

### 🧠 **Inteligência Artificial**
- ✅ Inferência com TensorFlow Lite Micro
- 🎯 Classificação de 4 comandos: Silêncio, Desconhecido, Sim, Não
- 🔄 Fallback inteligente com modo simulação
- 📊 Confiança de predição configurável

### 🔊 **Processamento de Áudio**
- 🎤 Captura via microfone digital I²S (INMP441)
- 🧮 Processamento com janela Hamming e VAD
- 📈 Detecção de atividade vocal em tempo real
- 🔇 Noise gate inteligente

### 🔋 **Gerenciamento de Energia**
- ⚡ 3 modos: ATIVO (240MHz), ESCUTANDO (80MHz), DORMINDO
- 💤 Light sleep automático com wake-on-sound
- 🔄 Transições inteligentes baseadas em atividade
- 📱 Despertar por botão ou detecção de voz

### 💾 **Otimizações de Memória**
- 🧩 Alocação dinâmica de buffers por demanda
- 📦 Liberação automática de memória em modo baixo consumo
- 🎯 Arena de tensores otimizada (60KB)

---

## 🛠 Hardware Necessário

### **Componentes Principais**
| Componente | Especificação | Função |
|------------|---------------|---------|
| **Microcontrolador** | ESP32-S3 DevKit | Processamento principal |
| **Microfone** | INMP441 Digital I²S | Captura de áudio |
| **LED** | LED integrado (GPIO 2) | Feedback visual |
| **Botão** | Push button (GPIO 0) | Wake-up manual |

### **Conexões I²S**

```
ESP32-S3    ←→    INMP441
──────────────────────────
GPIO 26     ←→    SCK (Serial Clock)
GPIO 25     ←→    WS  (Word Select)  
GPIO 33     ←→    SD  (Serial Data)
3.3V        ←→    VDD
GND         ←→    GND & L/R
```

### **Esquema de Ligação**

```
                    ┌─────────────┐
                    │   ESP32-S3  │
                    │             │
    ┌─INMP441───────┤ GPIO 26 SCK │
    │ SCK     WS────┤ GPIO 25 WS  │
    │ SD      SD────┤ GPIO 33 SD  │
    │ VDD     3V3───┤ 3.3V        │
    │ GND     GND───┤ GND         │
    │ L/R     GND───┤ GND         │
    └───────────────┤             │
    ┌─Button────────┤ GPIO 0      │
    │               │             │
    └─GND───────────┤ GND         │
                    │             │
    ┌─LED───────────┤ GPIO 2      │
    └─GND───────────┤ GND         │
                    └─────────────┘
```

---

## 📦 Instalação

### **1. Pré-requisitos**

- **Arduino IDE** 2.0+
- **ESP32 Board Package** 2.0.11+
- **Bibliotecas:**
  ```
  - ESP32 Core Libraries (incluído)
  - TensorFlow Lite Micro (opcional)
  ```

### **2. Download do Projeto**

```bash
git clone https://github.com/seu-usuario/TinyML-Voice-ESP32.git
cd TinyML-Voice-ESP32
```

### **3. Configuração do Arduino IDE**

1. **Instalar ESP32 Boards:**
   - File → Preferences
   - Additional Board Manager URLs: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - Tools → Board → Boards Manager → Pesquisar "ESP32" → Install

2. **Selecionar Placa:**
   - Tools → Board → ESP32 Arduino → ESP32S3 Dev Module

3. **Configurações da Placa:**
   ```
   CPU Frequency: 240MHz
   Flash Mode: QIO
   Flash Size: 4MB
   Partition Scheme: Default 4MB
   PSRAM: Enabled
   ```

---

## ⚙️ Configuração

### **1. Configurações Básicas (`config.h`)**

```cpp
// Configurações de áudio
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_BUFFER_SIZE 1024
#define INPUT_FEATURES 1024
#define OUTPUT_CLASSES 4

// Thresholds
#define CONFIDENCE_THRESHOLD 0.7f
#define VOICE_ACTIVITY_THRESHOLD 0.001f

// Pinos I²S
#define I2S_SCK_PIN 26
#define I2S_WS_PIN 25  
#define I2S_SD_PIN 33
```

### **2. Configurações de Energia**

```cpp
// Frequências CPU (MHz)
#define CPU_FREQ_ACTIVE 240
#define CPU_FREQ_LISTENING 80
#define CPU_FREQ_SLEEPING 80

// Timeouts (milissegundos)
#define ACTIVE_TIMEOUT 5000
#define LISTENING_TIMEOUT 30000
#define SLEEP_DURATION 60
```

---

## 🚀 Como Usar

### **1. Upload do Código**

1. Abra `TinyML_Voice_ESP32.ino` no Arduino IDE
2. Conecte o ESP32-S3 via USB
3. Selecione a porta correta em Tools → Port
4. Clique em Upload (Ctrl+U)

### **2. Monitor Serial**

```bash
# Abra o Serial Monitor (Ctrl+Shift+M)
# Baud Rate: 115200
```

### **3. Operação**

**Estados do Sistema:**
- 🟢 **ATIVO**: Processamento completo, CPU 240MHz
- 🟡 **ESCUTANDO**: VAD ativo, CPU 80MHz  
- 🔴 **DORMINDO**: Light sleep, consumo mínimo

**Interação:**
- **Falar comandos**: "Sim" ou "Não" próximo ao microfone
- **Botão**: Pressionar GPIO 0 para forçar modo ATIVO
- **LED**: Pisca ao detectar comandos válidos

### **4. Exemplo de Saída**

```
🚀 TinyML Voice - Starting...
🔋 CPU: 80 MHz
🧠 Inicializando ML Processor...
✅ TensorFlow Lite inicializado!
   📥 Input: [1024] float32 (4096 bytes)
   📤 Output: [4] float32 (16 bytes)
   💾 Arena usada: 45 KB
✅ System ready!

🔄 ESCUTANDO → ATIVO
🎯 ✅ Sim (87.3%) - 1240 μs
🔄 ATIVO → ESCUTANDO
🎯 ❌ Não (82.1%) - 1156 μs
💤 Sleep mode...
```

---

## 🧠 Treinamento do Modelo

### **Opção 1: Teachable Machine (Recomendado)**

1. **Acesse:** [teachablemachine.withgoogle.com](https://teachablemachine.withgoogle.com/)
2. **Escolha:** Audio Project
3. **Classes:** Crie 4 classes:
   - `Silêncio` (ambiente silencioso)
   - `Desconhecido` (ruídos diversos)
   - `Sim` (grave sua voz falando "sim" ~10 vezes)
   - `Não` (grave sua voz falando "não" ~10 vezes)
4. **Treinar:** Clique em "Train Model"
5. **Exportar:** Export Model → TensorFlow Lite → Download

### **Opção 2: TensorFlow Manual**

```python
import tensorflow as tf

# Carregar dados de áudio
# Preprocessar com MFCC
# Treinar modelo CNN/RNN
# Converter para TensorFlow Lite

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

# Salvar modelo
with open('voice_model.tflite', 'wb') as f:
    f.write(tflite_model)
```

### **3. Conversão para Array C**

```bash
# Usar ferramenta xxd
xxd -i voice_model.tflite > model_data.cpp

# Ou usar ferramenta online:
# https://tomeko.net/online_tools/file_to_hex.php
```

### **4. Integração no Código**

```cpp
// Substituir em tflite_config.h
const unsigned char g_model_data[] PROGMEM = {
  0x1c, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33,
  // ... seus dados do modelo aqui
};
const int g_model_data_len = sizeof(g_model_data);
```

---

## 🏗 Arquitetura do Sistema

### **Diagrama de Estados**

```
    ┌─────────────┐    Voz detectada    ┌─────────────┐
    │  ESCUTANDO  │────────────────────→│   ATIVO     │
    │   (80MHz)   │                     │  (240MHz)   │
    └─────────────┘                     └─────────────┘
           │ ▲                                  │
           │ │ Wake-up                          │ Timeout (5s)
           │ │                                  │
           ▼ │                                  ▼
    ┌─────────────┐                     ┌─────────────┐
    │  DORMINDO   │                     │  ESCUTANDO  │
    │   Light     │◄────Timeout (30s)───│    VAD      │
    │   Sleep     │                     │  Ativo      │
    └─────────────┘                     └─────────────┘
```

### **Fluxo de Processamento**

```
📡 I²S Audio → 🔍 VAD → 🧮 Preprocessing → 🧠 TF Lite → 📊 Classification
     ↓              ↓         ↓              ↓           ↓
  INMP441      Energy    Hamming/MFCC    Inference   Sim/Não/...
   16kHz      Analysis     Features      ~1-2ms     Confidence
```

### **Estrutura de Arquivos**

```
TinyML_Voice_ESP32/
├── 📄 TinyML_Voice_ESP32.ino    # Código principal
├── 📄 config.h                  # Configurações
├── 📄 audio_processor.h/.cpp    # Processamento áudio
├── 📄 ml_processor.h/.cpp       # Machine Learning
├── 📄 tflite_config.h          # Modelo TensorFlow
├── 📄 mfcc_processor.h         # Extração MFCC
├── 📄 power_manager.h          # Gerenciamento energia
├── 📄 model_data.h             # Dados do modelo
├── 📄 README.md                # Documentação
└── 📄 LICENSE                  # Licença MIT
```

---

## 🔧 API e Configurações

### **MLProcessor**

```cpp
MLProcessor ml_processor;

// Inicializar
bool success = ml_processor.begin();

// Executar predição
int result = ml_processor.predict(features, INPUT_FEATURES);
float confidence = ml_processor.get_confidence();

// Verificar disponibilidade
bool available = ml_processor.is_available();
```

### **AudioProcessor**

```cpp
AudioProcessor audio;

// Capturar áudio
bool captured = audio.captureAudio(buffer, AUDIO_BUFFER_SIZE);

// Preprocessar
bool processed = audio.preprocess(input, output, length);

// Detectar voz
bool voice = audio.detect_voice_activity(buffer, length);
```

### **Configurações Avançadas**

```cpp
// Ajustar sensibilidade VAD
config.vad_threshold = 0.002f;  // Mais sensível
config.vad_threshold = 0.0005f; // Menos sensível

// Timeouts personalizados
config.active_timeout = 10000;   // 10 segundos ativo
config.listen_timeout = 60000;   // 1 minuto escutando

// Frequências CPU
config.low_cpu_freq = 40;        // 40MHz economia extrema
config.low_cpu_freq = 160;       // 160MHz performance
```

---

## 🐛 Troubleshooting

### **Problemas Comuns**

#### **❌ "Error: I2S setup failed"**
```cpp
// Verificar conexões I²S
// Testar com outros pinos GPIO
#define I2S_SCK_PIN 14  // Tentar GPIO 14
#define I2S_WS_PIN 15   // Tentar GPIO 15
#define I2S_SD_PIN 32   // Tentar GPIO 32
```

#### **❌ "ML unavailable - simulation mode"**
```cpp
// Modelo não carregado corretamente
// 1. Verificar g_model_data em tflite_config.h
// 2. Confirmar tamanho > 32 bytes
// 3. Verificar magic number TFL3
```

#### **❌ Áudio não detectado**
```cpp
// Ajustar threshold VAD
#define VOICE_ACTIVITY_THRESHOLD 0.0005f  // Mais sensível

// Verificar microfone
// Testar com Serial Monitor para ver energia
```

#### **⚠️ Alto consumo de energia**
```cpp
// Verificar se está entrando em sleep
// Ajustar timeouts
config.active_timeout = 3000;    // Reduzir tempo ativo
config.listen_timeout = 15000;   // Reduzir tempo escutando
```

### **Debug e Monitoramento**

```cpp
// Habilitar debug detalhado
#define DEBUG_VERBOSE 1

// Monitor energia áudio
audio.print_stats(buffer, length);

// Informações do modelo
ml_processor.print_model_info();

// Estado atual do sistema
Serial.printf("Estado: %s, CPU: %dMHz\n", 
              state_name, getCpuFrequencyMhz());
```

### **Teste de Hardware**

```cpp
// Teste de microfone I²S
void test_microphone() {
    int16_t test_buffer[256];
    if (audio.captureAudio(test_buffer, 256)) {
        float energy = audio.calculate_energy(test_buffer, 256);
        Serial.printf("✅ Microfone OK - Energia: %.4f\n", energy);
    } else {
        Serial.println("❌ Microfone falhou");
    }
}
```

---

## 🤝 Contribuição

Contribuições são bem-vindas! Siga estes passos:

### **Como Contribuir**

1. **Fork** o repositório
2. **Crie** uma branch (`git checkout -b feature/nova-funcionalidade`)
3. **Commit** suas mudanças (`git commit -am 'Adiciona nova funcionalidade'`)
4. **Push** para a branch (`git push origin feature/nova-funcionalidade`)
5. **Abra** um Pull Request

### **Áreas para Contribuição**

- 🧠 **Modelos ML**: Novos modelos para mais comandos
- 🔊 **Processamento**: Implementação MFCC completa
- 🔋 **Energia**: Otimizações de consumo
- 📱 **Interface**: Dashboard web ou app mobile
- 🌐 **Conectividade**: Integração Wi-Fi/Bluetooth
- 📚 **Documentação**: Tutoriais e guias

### **Guidelines**

- Código bem documentado
- Testes em hardware real
- Compatibilidade com ESP32-S3
- Seguir estilo do projeto

---

## 📈 Roadmap

### **v2.0 - Próximas Features**
- [ ] 🎯 Implementação MFCC completa
- [ ] 📱 Dashboard web para monitoramento
- [ ] 🌐 Upload de modelos via Wi-Fi
- [ ] 🔄 Treinamento incremental
- [ ] 🎵 Suporte a mais comandos (0-9, cores, etc.)

### **v3.0 - Features Avançadas**
- [ ] 🗣️ Múltiplos idiomas (PT/EN/ES)
- [ ] 👥 Reconhecimento de speaker
- [ ] 🎛️ Calibração automática
- [ ] 📊 Analytics de uso
- [ ] 🔒 Criptografia do modelo

---

## 📊 Performance

### **Benchmarks ESP32-S3**

| Métrica | Valor |
|---------|-------|
| **Latência Inferência** | ~1-2ms |
| **Consumo ATIVO** | ~150mA @ 240MHz |
| **Consumo ESCUTANDO** | ~80mA @ 80MHz |
| **Consumo DORMINDO** | ~5mA (light sleep) |
| **Acurácia** | 85-95% (modelo bem treinado) |
| **Taxa Amostragem** | 16kHz |
| **Memória Usada** | ~100KB RAM, ~45KB Arena |

---

## 📄 Licença

Este projeto está licenciado sob a **MIT License** - veja o arquivo [LICENSE](LICENSE) para detalhes.

```
MIT License - Copyright (c) 2025 Felipe Rangel

✅ Uso comercial permitido
✅ Modificação permitida  
✅ Distribuição permitida
✅ Uso privado permitido
❌ Sem garantia
❌ Sem responsabilidade
```

---

## 🙏 Agradecimentos

- **Espressif** pelo incrível ESP32-S3
- **TensorFlow Team** pelo TF Lite Micro
- **Arduino Community** pelo suporte e bibliotecas
- **Google** pelo Teachable Machine
- **Comunidade Brasileira de IoT** 🇧🇷

---
