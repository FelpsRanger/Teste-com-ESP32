
---

## 🚀 Como executar

1. Treine um modelo de reconhecimento de voz com TensorFlow ou Teachable Machine
2. Converta o `.tflite` para um array `g_model_data[]` usando a ferramenta [xxd](https://tomeko.net/online_tools/file_to_hex.php?lang=en)
3. Substitua o conteúdo no `tflite_config.h`
4. Conecte seu microfone I²S aos pinos definidos no `config.h`
5. Compile e faça upload para sua placa ESP32-S3 via Arduino IDE
6. Monitore a inferência via Serial Monitor

---

## ⚠️ Sobre o modelo

Se o modelo `g_model_data` for inválido ou muito pequeno, o sistema entra em modo **simulação** com resultados aleatórios.

---

## 📶 Feedback visual

- LED pisca ao detectar comandos válidos
- Pisca rapidamente em caso de erro crítico no `error_loop()`

---

## 📌 Possibilidades futuras

- Suporte a mais comandos personalizados
- Upload de amostras via Wi-Fi
- Dashboard web para visualização dos resultados
- Treinamento incremental embarcado

---


