# Estação Solarimétrica – Raspberry Pi 4 + I2C

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/-Raspberry_Pi-C51A4A?style=for-the-badge&logo=Raspberry-Pi)
![Linux](https://img.shields.io/badge/Linux-%23000000.svg?style=for-the-badge&logo=linux&logoColor=white)
![GitHub](https://img.shields.io/badge/github-%23121011.svg?style=for-the-badge&logo=github&logoColor=white)

## Descrição do Projeto

Este projeto tem como objetivo utilizar uma Raspberry Pi 4, acessando seu barramento I2C para ler valores de sensores meteorológicos e radiométricos, simulando uma estação solarimétrica. A aplicação coleta dados como temperatura, umidade, pressão atmosférica e iluminância/irradiância, consolidando medições em tempo real para análise e registro.

## Visão Geral

- Arquivo principal: `main.c` — realiza a inicialização do I2C, leitura dos sensores e exibição dos dados no console.
- Interface: barramento I2C nativo da Raspberry Pi 4 (`/dev/i2c-1`).
- Sensores utilizados:
  - MPU6050 (temperatura interna do IMU)
  - AHT20 (temperatura e umidade)
  - BH1750 (iluminância em lux)
- Sistema de saída: impressão formatada no console (sem CSV).

Observação: a lista de sensores é configurável; ajuste endereços I2C, constantes de conversão e frequência de amostragem conforme seu hardware.

## Hardware (Conexões Sugeridas)

- Raspberry Pi 4 (Raspbian/Raspberry Pi OS)
- I2C da Raspberry Pi 4:
  - SDA: GPIO2 — pino físico 3
  - SCL: GPIO3 — pino físico 5
  - 3V3: pino físico 1
  - GND: pino físico 6
- Sensores I2C compatíveis (alimentados em 3.3V). Muitos módulos já possuem resistores de pull‑up; evite duplicar.

Endereços I2C padrão (ajustáveis conforme hardware):
- MPU6050: `0x68` (AD0 em nível baixo)
- AHT20: `0x38`
- BH1750: `0x23` (ADDR em nível baixo)

## Preparação do Sistema

1. Habilite o I2C:

```bash
sudo raspi-config
# Interface Options -> I2C -> Enable
```

2. Instale ferramentas e headers de compilação:

```bash
sudo apt update
sudo apt install -y i2c-tools build-essential
```

3. Verifique se o barramento está ativo e os dispositivos são detectados:

```bash
i2cdetect -y 1
```

## Como Compilar e Executar

Compilação simples (ajuste flags se seu código usar bibliotecas específicas):

```bash
gcc main.c -o solarimetria -lm
./solarimetria
```

Notas:
- O código usa a interface `i2c-dev` padrão do Linux; não requer bibliotecas externas.
- Caso opte por bibliotecas como `pigpio` ou `wiringPi` (opcional), ajuste as flags de compilação.

## Estrutura do Repositório

- `main.c` — lógica principal de leitura via I2C e formatação dos dados.
- `README.md` — este documento.

## Funcionamento

- Inicializa o barramento I2C (`/dev/i2c-1`).
- Configura e lê os sensores em intervalos regulares:
  - MPU6050: leitura de temperatura interna via registradores `0x41/0x42` e conversão `T(°C) = raw/340 + 36.53`.
  - AHT20: comando de medição `0xAC 0x33 0x00`, espera ~80 ms e leitura de 6 bytes; conversão para umidade relativa (%) e temperatura (°C) conforme datasheet.
  - BH1750: modo High-Resolution (`0x10`), leitura de 2 bytes e conversão para lux (`lux = raw/1.2`).
- Exibe medições com formatação de moldura no console (caracteres box‑drawing).

## Personalização

- Adicione/remova sensores conforme necessidade e disponibilidade.
- Ajuste endereços I2C, coeficientes de conversão e taxa de amostragem.
- Integre saída com bancos de dados (InfluxDB), MQTT ou dashboards (Grafana) para monitoramento contínuo.

## Referências Úteis

- Documentação oficial do I2C no Linux (`/usr/include/linux/i2c-dev.h`).
- `i2c-tools`: utilitários para diagnóstico e identificação de dispositivos.
- Fichas técnicas dos sensores (BME280, BH1750, TSL2561, ADS1115) para endereços e fórmulas de conversão.