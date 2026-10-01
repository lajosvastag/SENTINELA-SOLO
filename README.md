# SENTINELA-SOLO

> Monitoramento de umidade, temperatura e condutividade elétrica do solo com ESP32.

**Autoria:** Luís Brito<br>
**Copyright:** Curié Edge<br>
**Versão do firmware:** 1.1.0<br>
**Licença:** MIT

O **SENTINELA-SOLO** é uma base de prototipagem para aquisição de dados do solo com um ESP32 DevKit. O dispositivo lê um sensor capacitivo de umidade, um sensor de temperatura DS18B20 e um módulo analógico de condutividade elétrica (EC). As leituras são disponibilizadas pela porta Serial em JSON e por uma API HTTP local.

> **Importante:** as leituras de umidade e condutividade dependem do sensor, do solo, do circuito de condicionamento e da calibração. Este projeto não substitui análise laboratorial ou instrumentação agronômica certificada.

## Sumário

- [O que o projeto faz](#o-que-o-projeto-faz)
- [Arquitetura](#arquitetura)
- [Materiais](#materiais)
- [Ligações](#ligações)
- [Instalação e primeiro uso](#instalação-e-primeiro-uso)
- [Configuração](#configuração)
- [Wi-Fi e API HTTP](#wi-fi-e-api-http)
- [Formato das leituras](#formato-das-leituras)
- [Calibração](#calibração)
- [Segurança elétrica e de rede](#segurança-elétrica-e-de-rede)
- [Testes e CI](#testes-e-ci)
- [Estrutura do repositório](#estrutura-do-repositório)
- [Solução de problemas](#solução-de-problemas)
- [Próximos passos](#próximos-passos)
- [Licença e autoria](#licença-e-autoria)

## O que o projeto faz

A cada intervalo configurado, o ESP32 faz uma média de amostras dos dois canais ADC, converte o canal de umidade para uma porcentagem interpolada entre os pontos seco e úmido, solicita a temperatura ao DS18B20, estima a condutividade em mS/cm com compensação aproximada para 25 °C e publica a leitura em JSON pela Serial e em `GET /api/reading`.

O dispositivo sinaliza estados básicos como `ok`, `soil_dry`, `temperature_out_of_range` e `temperature_sensor_error`.

## Arquitetura

```text
Sensor capacitivo de umidade ── ADC GPIO34 ┐
                                           ├── ESP32 ── Serial JSON
Módulo analógico de EC ─────── ADC GPIO35 ┘       └── API HTTP local
DS18B20 ────────────────────── 1-Wire GPIO4
```

Os GPIO34 e GPIO35 pertencem ao ADC1 e são somente entrada. Essa escolha evita o conflito comum entre ADC2 e o Wi-Fi do ESP32.

## Materiais

- ESP32 DevKit V1 ou placa compatível.
- Sensor capacitivo analógico de umidade do solo.
- DS18B20, preferencialmente encapsulado para uso externo.
- Resistor de 4,7 kΩ entre DATA e 3V3 do DS18B20.
- Probe e módulo analógico de condutividade compatível com 3,3 V.
- Fonte 5 V com corrente adequada para o ESP32 e os sensores.
- Cabos, protoboard ou placa de circuito e proteção contra umidade.

## Ligações

| Componente | ESP32 | Observação |
|---|---:|---|
| Saída analógica do sensor de umidade | GPIO34 | ADC1; entrada somente |
| Saída analógica do módulo EC | GPIO35 | ADC1; entrada somente |
| DATA do DS18B20 | GPIO4 | Pull-up externo de 4,7 kΩ para 3V3 |
| VCC dos sensores | 3V3 | Confirme a especificação de cada módulo |
| GND dos sensores | GND | Terra comum |

Nunca conecte um sinal acima de 3,3 V a um ADC do ESP32. Alguns módulos de umidade ou EC são alimentados em 5 V e podem exigir divisor resistivo ou condicionamento adicional na saída.

## Instalação e primeiro uso

### PlatformIO

1. Instale o [PlatformIO](https://platformio.org/) no VS Code ou o PlatformIO Core.
2. Clone este repositório e abra a pasta:

```bash
git clone https://github.com/lajosvastag/SENTINELA-SOLO.git
cd SENTINELA-SOLO
```

3. Edite `include/config.h` antes de gravar o firmware.
4. Conecte o ESP32 por USB.
5. Compile, grave e abra o monitor Serial:

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

O primeiro build baixa automaticamente as bibliotecas declaradas em `platformio.ini`.

### Arduino IDE

O caminho recomendado é o PlatformIO, pois ele fixa a placa e as dependências. Para usar Arduino IDE, instale as bibliotecas OneWire e DallasTemperature, copie `src/main.cpp` para um sketch, disponibilize `include/config.h` no projeto e selecione uma placa ESP32 Dev Module.

## Configuração

Todas as configurações do dispositivo ficam em `include/config.h`.

| Constante | Função | Padrão |
|---|---|---|
| `WIFI_SSID` / `WIFI_PASSWORD` | Rede Wi-Fi existente | vazio |
| `AP_SSID` / `AP_PASSWORD` | Rede criada pelo ESP32 quando não há Wi-Fi | `SENTINELA-SOLO` / `sentinela123` |
| `SAMPLE_INTERVAL_MS` | Intervalo entre leituras | 5000 ms |
| `ANALOG_SAMPLES` | Amostras usadas na média ADC | 8 |
| `MOISTURE_ADC_DRY` | Referência do solo seco | 3000 |
| `MOISTURE_ADC_WET` | Referência do solo úmido | 1300 |
| `EC_SLOPE` / `EC_OFFSET` | Ajuste da conversão de EC | 1.00 / 0.00 |
| `MOISTURE_LOW_PERCENT` | Limite para estado `soil_dry` | 30% |
| `TEMPERATURE_LOW_C` / `TEMPERATURE_HIGH_C` | Faixa de alerta térmico | 10 / 40 °C |

Não publique senhas reais no GitHub. Para instalações em campo, altere a senha do modo AP e considere manter credenciais em um arquivo local ignorado pelo Git.

## Wi-Fi e API HTTP

Quando `WIFI_SSID` está preenchido, o firmware tenta conectar-se por até 15 segundos. Se a rede não for configurada ou falhar, o ESP32 cria um ponto de acesso:

- **SSID:** `SENTINELA-SOLO`
- **Senha inicial:** `sentinela123`
- **Endereço padrão do AP:** `192.168.4.1`

| Rota | Resposta |
|---|---|
| `GET /` | Página simples de identificação |
| `GET /api/reading` | Leitura atual em JSON |
| `GET /api/health` | Estado mínimo para monitoramento |

A API envia `Cache-Control: no-store` e `Access-Control-Allow-Origin: *`, facilitando a integração com um painel local. Ela não possui autenticação; não exponha o ESP32 diretamente na Internet.

Teste pelo terminal:

```bash
curl http://192.168.4.1/api/reading
curl http://192.168.4.1/api/health
```

## Formato das leituras

Exemplo de `GET /api/reading` ou de uma linha enviada pela Serial:

```json
{
  "device": "SENTINELA-SOLO",
  "firmware": "1.1.0",
  "uptime_ms": 125000,
  "sampled_at_ms": 125000,
  "status": "ok",
  "moisture": {"raw": 1875, "percent": 59.2},
  "temperature_c": 24.75,
  "conductivity": {"raw": 1240, "ms_cm": 0.84}
}
```

Quando o DS18B20 está ausente ou desconectado, `temperature_c` é `null`, `status` é `temperature_sensor_error` e a EC continua sendo reportada sem compensação térmica.

## Calibração

### Umidade

O sensor capacitivo fornece uma leitura ADC dependente do tipo de solo e da profundidade. Meça e anote `moisture.raw` no solo considerado seco e no solo considerado úmido. Depois, ajuste `MOISTURE_ADC_DRY` e `MOISTURE_ADC_WET`. A porcentagem é uma interpolação linear limitada entre 0 e 100%.

Se o seu sensor produzir um valor maior quando está úmido, ajuste os dois valores ou a função `mapMoisturePercent()` em `src/main.cpp`.

### Temperatura

Compare o DS18B20 com um termômetro de referência em água estabilizada. O firmware considera inválidos valores fora de -55 a 125 °C e converte a desconexão para `null`.

### Condutividade elétrica

A EC depende do probe, da geometria, da solução, da salinidade, do circuito analógico e da temperatura. Use uma solução padrão e um instrumento de referência para ajustar `EC_SLOPE` e `EC_OFFSET`. A compensação implementada é aproximada e referenciada a 25 °C; valide-a em mais de um ponto antes de usar os dados para decisões agronômicas.

O guia complementar está em [`docs/GUIA-CALIBRACAO.md`](docs/GUIA-CALIBRACAO.md).

## Segurança elétrica e de rede

- Nunca exceda 3,3 V nas entradas ADC.
- Use fonte regulada e terra comum.
- Proteja conectores e emendas contra água e corrosão.
- Evite alimentar probes de EC continuamente quando isso acelerar corrosão; uma revisão futura pode chavear a alimentação por MOSFET.
- Troque a senha do AP antes de instalar o equipamento em local acessível.
- Mantenha a API em rede local e não faça port forwarding para a Internet.
- Desconecte a alimentação antes de alterar a fiação.

## Testes e CI

O build oficial pode ser executado com:

```bash
pio run
```

O workflow `.github/workflows/build.yml` executa essa compilação em cada push e pull request. O arquivo `test/README.md` traz o checklist de bancada. A compilação valida a integração do código, mas não substitui testes com sensores fisicamente conectados.

## Estrutura do repositório

```text
SENTINELA-SOLO/
├── .github/workflows/build.yml  # compilação no GitHub Actions
├── docs/GUIA-CALIBRACAO.md      # calibração de campo
├── include/config.h             # pinos, rede e parâmetros
├── src/main.cpp                 # firmware
├── test/README.md               # checklist de testes
├── .gitignore                   # artefatos locais ignorados
├── CONTRIBUTING.md
├── LICENSE
├── platformio.ini
└── README.md
```

## Solução de problemas

**O build falha por dependência:** execute `pio pkg update` e tente `pio run` novamente.<br>
**A temperatura aparece como `null`:** verifique DATA, GND, 3V3 e o resistor de 4,7 kΩ.<br>
**A umidade fica invertida:** recalibre os valores seco/úmido ou revise `mapMoisturePercent()`.<br>
**O ESP32 não conecta ao Wi-Fi:** confirme SSID e senha; o AP de fallback deve aparecer após 15 segundos.<br>
**A EC fica instável:** verifique o condicionamento analógico, a alimentação, o aterramento e a calibração com solução padrão.<br>
**O endpoint não abre:** confirme o IP mostrado na Serial e que o computador está na mesma rede do ESP32.

## Próximos passos

Entre as evoluções naturais estão armazenamento em cartão SD, MQTT/HTTPS com autenticação, dashboard, alimentação solar, chaveamento do probe de EC e calibração por curva específica de cada solo.

## Licença e autoria

Este projeto é distribuído sob a [Licença MIT](LICENSE).

Copyright (c) 2026 **Curié Edge**<br>
Autoria: **Luís Brito**
