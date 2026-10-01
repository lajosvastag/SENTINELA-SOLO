# Guia de calibração de campo

## Registro recomendado

Para cada ponto de medição, registre data/hora, cultura, profundidade, tipo de solo, leitura do SENTINELA-SOLO e uma referência (massa/umidade de laboratório ou instrumento confiável).

## Umidade

O sensor capacitivo não mede diretamente água em volume: ele fornece uma tensão/contagem ADC relacionada à umidade e ao tipo de solo. Faça pelo menos dois pontos no próprio solo:

- **Ponto seco:** sensor no solo na condição considerada seca.
- **Ponto úmido:** sensor no solo após irrigação, quando atingir a condição considerada úmida.

A porcentagem exibida é uma interpolação linear entre `MOISTURE_ADC_DRY` e `MOISTURE_ADC_WET`. Se o sensor apresentar valor crescente com umidade, inverta os dois valores ou ajuste a fórmula em `src/main.cpp`.

## Temperatura

Compare o DS18B20 com um termômetro de referência em água estabilizada. O valor `-127 °C` indica sensor desconectado e é convertido para `null` no JSON.

## Condutividade

1. Confirme que o módulo/probe aceita 3,3 V e que sua saída não ultrapassa o limite do ADC.
2. Meça uma solução padrão conhecida na mesma temperatura aproximada do uso.
3. Compare com o valor exibido pelo SENTINELA-SOLO.
4. Ajuste `EC_SLOPE` e `EC_OFFSET`.
5. Valide com uma segunda solução ou instrumento independente.

A compensação usada é aproximada:

```text
EC25 ≈ ECmedida / (1 + 0,019 × (temperatura − 25))
```

Para medições agronômicas críticas, use uma placa de condicionamento apropriada e calibração multiponto.
