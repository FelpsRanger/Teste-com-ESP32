# TinyML Voice Recognition ESP32

🎤 Sistema de reconhecimento de voz otimizado para ESP32 usando TensorFlow Lite Micro

## 📋 Visão Geral

Este projeto implementa um sistema de reconhecimento de comandos de voz em tempo real no ESP32, utilizando TensorFlow Lite para Microcontroladores (TinyML). O sistema pode reconhecer comandos básicos como "Sim" e "Não", além de detectar silêncio e sons desconhecidos.

### ✨ Características Principais

- **Reconhecimento em Tempo Real**: Processamento de áudio e inferência ML em menos de 100ms
- **Baixo Consumo**: Gerenciamento inteligente de energia com sleep modes
- **Voice Activity Detection (VAD)**: Detecção automática de atividade de voz
- **Otimizado para ESP32**: Uso eficiente de RAM e PSRAM
- **Interface I2S**: Suporte nativo para microfones digitais
- **Feedback Visual**: Indicações LED para comandos reconhecidos

## 🔧 Requisitos de Hardware

### Componentes Necessários

- **ESP32** (qualquer variante com PSRAM recomendado)
- **Microfone I2S** (ex: INMP441, SPH0645)
- **LED** (para feedback visual)
- **Botão** (para ativação manual)
- **Resistor Pull-up** 10kΩ (para o botão)

### Pinagem Padrão

| Componente | Pino ESP32 | Descrição |
|------------|------------|-----------|
| I2S WS (LRCK) | GPIO 15 | Word Select |
| I2S SCK (BCLK) | GPIO 2 | Bit Clock |
| I2S SD (DOUT) | GPIO 4 | Serial Data |
| LED | GPIO 2 | Indicador visual |
| Botão | GPIO 0 | Ativação manual |

## 📦 Dependências

### Arduino IDE

```cpp
// Bibliotecas necessárias (instalar via Gerenciador de Bibliotecas)
- ArduinoJson
- ESP32 Board Package (versão 2.0+)
```

### TensorFlow Lite Micro

O código inclui verificações automáticas para TensorFlow Lite. Se não estiver disponível, o sistema funcionará em modo de simulação para testes.

## 🚀 Instalação e Configuração

### 1. Preparação do Ambiente

```bash
# Clone o repositório
git clone https://github.com/seu-usuario/TinyML_Voice_ESP32.git
cd TinyML_Voice_ESP32

# Abra o projeto no Arduino IDE
arduino TinyML_Voice_ESP32.ino
```

### 2. Configuração das Bibliotecas

1. Instale o **ESP32 Board Package** (versão 2.0 ou superior)
2. Instale a biblioteca **ArduinoJson**
3. Configure a **Partition Scheme** para "Huge APP (3MB No OTA/1MB SPIFFS)"

### 3. Configuração de Hardware

Conecte os componentes conforme a pinagem especificada. Modifique os pinos em `config.h` se necessário:

```cpp
// config.h
#define I2S_WS_PIN 15    // Word Select
#define I2S_SCK_PIN 2    // Bit Clock  
#define I2S_SD_PIN 4     // Serial Data
#define LED_PIN 2        // LED indicador
#define BUTTON_PIN 0     // Botão de ativação
```

### 4. Upload e Teste

1. Selecione sua placa ESP32
2. Configure **CPU Frequency** para 240MHz (recomendado)
3. Configure **PSRAM** como "Enabled" se disponível
4. Faça o upload do código

## 📊 Classes de Reconhecimento

O sistema reconhece 4 classes diferentes:

| Classe | ID | Descrição | Emoji |
|--------|----|-----------| ------|
| Silêncio | 0 | Ausência de áudio significativo | 🔇 |
| Desconhecido | 1 | Sons não reconhecidos | ❓ |
| Sim | 2 | Comando "Sim" | ✅ |
| Não | 3 | Comando "Não" | ❌ |

## ⚙️ Configurações Avançadas

### Parâmetros de Áudio

```cpp
// config.h - Configurações de áudio
#define AUDIO_SAMPLE_RATE 16000        // Taxa de amostragem
#define AUDIO_BUFFER_SIZE 1024         // Tamanho do buffer
#define INPUT_FEATURES 1024            // Características de entrada
#define CONFIDENCE_THRESHOLD 0.7f      // Limiar de confiança
#define VOICE_ACTIVITY_THRESHOLD 0.001f // Limiar VAD
```

### Gerenciamento de Energia

O sistema inclui gerenciamento automático de energia:

- **Modo Ativo**: Durante detecção de voz ou uso do botão
- **Light Sleep**: Quando inativo por curtos períodos
- **Deep Sleep**: Para economia máxima de energia (implementação futura)

## 🔍 Monitoramento e Debug

### Monitor Serial

O sistema fornece informações detalhadas via Serial Monitor (115200 baud):

```
🚀 Inicializando TinyML Voice Recognition...
💾 RAM livre: 298 KB
💾 PSRAM livre: 4096 KB
✅ Sistema inicializado com sucesso!

🎯 ✅ Sim (87.3%) - 42851 μs
🎯 ❌ Não (91.2%) - 38942 μs

📊 RELATÓRIO DE PERFORMANCE:
   🔢 Inferências: 156
   💾 RAM livre: 287 KB
   📊 PSRAM livre: 4092 KB
   🧠 CPU: 240 MHz
```

### Indicadores LED

- **LED piscando rápido**: Erro de inicialização
- **LED aceso breve**: Comando reconhecido com confiança

## 🔄 Estrutura do Código

### Arquivos Principais

```
TinyML_Voice_ESP32/
├── TinyML_Voice_ESP32.ino    # Arquivo principal
├── config.h                  # Configurações do sistema
├── audio_processor.h/.cpp    # Processamento de áudio
├── ml_processor.h/.cpp       # Inferência ML
├── power_manager.h           # Gerenciamento de energia
├── model_data.h              # Dados do modelo ML
└── LICENSE                   # Licença MIT
```

### Fluxo de Execução

1. **Inicialização**: Configuração I2S, alocação de buffers, carregamento do modelo
2. **Captura**: Leitura contínua de áudio via I2S
3. **Pré-processamento**: VAD, janelamento Hamming, normalização
4. **Inferência**: Execução do modelo TensorFlow Lite
5. **Pós-processamento**: Interpretação dos resultados e ações

## 🤖 Treinamento de Modelo Personalizado

Para treinar seu próprio modelo:

1. **Coleta de Dados**: Grave samples dos comandos desejados
2. **Pré-processamento**: Normalize e converta para o formato adequado
3. **Treinamento**: Use TensorFlow/Keras para treinar o modelo
4. **Conversão**: Converta para TensorFlow Lite Micro
5. **Integração**: Substitua os dados em `model_data.h`

### Exemplo de Conversão

```python
import tensorflow as tf

# Converter modelo para TensorFlow Lite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

# Gerar arquivo C header
with open('model_data.h', 'w') as f:
    f.write('#include <stdint.h>\n')
    f.write(f'const unsigned char g_model_data[] = {{\n')
    for i, byte in enumerate(tflite_model):
        if i % 16 == 0:
            f.write('\n  ')
        f.write(f'0x{byte:02x}, ')
    f.write('\n};\n')
    f.write(f'const int g_model_data_len = {len(tflite_model)};\n')
```

## 📈 Performance

### Benchmarks Típicos

- **Tempo de Inferência**: 35-50ms
- **Uso de RAM**: ~300KB
- **Uso de PSRAM**: ~4MB
- **Taxa de Reconhecimento**: >90% (comandos treinados)
- **Latência Total**: <100ms

### Otimizações Implementadas

- Uso de PSRAM para buffers grandes
- Janelamento Hamming para redução de ruído
- VAD para economia de processamento
- Alocação dinâmica otimizada de memória

## 🔧 Solução de Problemas

### Problemas Comuns

**Erro de Memória**
```
❌ Falha na alocação de memória!
```
- Solução: Ative PSRAM ou reduza `AUDIO_BUFFER_SIZE`

**Falha I2S**
```
❌ Falha na configuração do I2S
```
- Solução: Verifique conexões do microfone e pinagem

**TensorFlow Lite Não Disponível**
```
⚠️ TensorFlow Lite não disponível - usando simulação
```
- Solução: Use ESP32 board package 2.0+ ou modo simulação para testes

### Debug Avançado

1. **Monitor de Memória**: Acompanhe uso de RAM/PSRAM
2. **Análise de Áudio**: Verifique níveis de energia do VAD
3. **Timing de Inferência**: Monitore tempos de processamento
4. **Taxa de Reconhecimento**: Analise confiança das predições

