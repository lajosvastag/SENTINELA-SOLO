# Testes e validação

## Checklist de bancada

- [ ] O firmware compila com `pio run`.
- [ ] O monitor Serial abre a 115200 baud.
- [ ] O JSON contém `moisture`, `temperature_c` e `conductivity`.
- [ ] Desconectar o DS18B20 produz `temperature_c: null` e status de erro.
- [ ] O endpoint `/api/reading` retorna HTTP 200.
- [ ] A leitura de umidade varia ao mudar a condição do solo.
- [ ] A tensão nos GPIO34/GPIO35 não excede 3,3 V.

Os testes de medição devem ser feitos com sensores conectados e instrumentos de referência; não há simulação de ADC neste protótipo.
