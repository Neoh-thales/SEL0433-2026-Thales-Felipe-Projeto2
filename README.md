# SEL0433-2026-Thales-Felipe-Projeto2
Projeto 2 de Aplicação de Microcontroladores
# Projeto 2: Aferidor de Temperatura de Forno Industrial
**Disciplina:** SEL0433 - Aplicação de Microprocessadores  
**Aluno:** Thales Vasconcelos Aguiar de Olivera  
**Nº USP:** 15489730
**Aluno:**  Felipe Assis Bernardes Falvo
**Nº USP:** 15004433

## 1. Introdução
Este repositório contém a solução final para o Projeto 2 (metodologia PBL), que consiste no desenvolvimento de um dispositivo de aferição de temperatura e tempo para fornos industriais. 

O sistema foi desenvolvido em linguagem C utilizando o microcontrolador **PIC18F4550**, simulado no software **SimulIDE** e compilado via **MikroC PRO for PIC**. O objetivo principal foi integrar múltiplos periféricos modernos da família PIC18, incluindo:
* Módulo ADC de 10 bits (para leitura do sensor de temperatura LM35).
* Timers (TMR0 e TMR1) para geração de bases de tempo precisas.
* Sistema de Interrupções Externas (INT0 e INT1) para controle da interface humana.
* Display LCD 16x2 operando em modo 4 bits.

## 2. Requisitos e Implementação
O funcionamento do equipamento baseia-se em medir a temperatura interna do forno na faixa de 0°C a 100°C e exibir uma contagem regressiva selecionável pelo usuário.

* **Interface e Controle:** Foram utilizados botões configurados com interrupção externa (borda de subida). O primeiro botão (RB0/INT0) alterna o modo de aferição entre Contagem Curta (10s) e Contagem Longa (60s). O segundo botão (RB1/INT1) atua como gatilho de início: uma vez acionado, ele inicia a contagem e a medição, travando o sistema por segurança até que o ciclo de tempo seja concluído.
* **Medição Analógica:** A medição do sensor LM35 foi emulada utilizando um potenciômetro no pino AN0. Para garantir precisão com a escala de 10mV/°C do LM35, a tensão de referência do ADC (Vref+) foi configurada para uma fonte externa de 1V no pino AN3, e a referência negativa (Vref-) atrelada ao GND no pino AN2.
* **Atuação:** Um LED foi alocado no pino RE0 para representar um alarme ou acionamento de resistência. O LED liga automaticamente sempre que a temperatura medida ultrapassa a marca de 50°C.

## 3. Breve Discussão dos Resultados
A integração dos subsistemas apresentou desafios arquiteturais que foram contornados com práticas de programação defensiva, garantindo maior confiabilidade ao circuito. 

Destaca-se a alocação do Display LCD para a Porta D, garantindo que os pinos de Interrupção Externa (INT0 e INT1) da Porta B ficassem inteiramente livres para a interface de botões. Na lógica de controle, implementou-se uma trava de estado (flag de software) no botão de "Start", impedindo interrupções acidentais ou pausas manuais durante um ciclo de medição ativo.

Além disso, a inicialização do conversor A/D exigiu intervenção direta no registrador `ADCON1` (recebendo o valor `0x3A`) após a chamada da biblioteca `ADC_Init()`. Isso assegurou a proteção dos pinos digitais da Porta B e a correta aplicação das tensões de referência externas. Por fim, para otimizar o uso da memória RAM do microcontrolador, o processamento da temperatura foi executado puramente com matemática de inteiros, formatando os caracteres ASCII diretamente no display LCD e eliminando o uso de variáveis do tipo `float`.
## 4. Imagens da Simulação e Compilação


**Compilação no MikroC (Sucesso e uso de memória):**
<img width="1339" height="431" alt="WhatsApp Image 2026-06-21 at 23 12 45" src="https://github.com/user-attachments/assets/78752a1d-cc72-4054-ab74-f9ac523b0fac" />
<img width="1365" height="738" alt="WhatsApp Image 2026-06-21 at 23 14 59" src="https://github.com/user-attachments/assets/027abcc9-dd31-4a6d-a713-db5889ebd3a9" />
<img width="460" height="669" alt="WhatsApp Image 2026-06-21 at 23 14 59(1)" src="https://github.com/user-attachments/assets/150b8b7c-0fb0-45fe-a0c4-19a14166064d" />
<img width="460" height="656" alt="WhatsApp Image 2026-06-21 at 23 14 59(2)" src="https://github.com/user-attachments/assets/821a5097-361c-42a3-bafb-f1b941a5d346" />


**Execução no SimulIDE (Sistema em funcionamento):**
<img width="1189" height="603" alt="WhatsApp Image 2026-06-21 at 23 08 56" src="https://github.com/user-attachments/assets/3d1eefab-e678-4757-bfe1-aff7a91d2ae5" />
<img width="1198" height="581" alt="WhatsApp Image 2026-06-21 at 23 08 56(1)" src="https://github.com/user-attachments/assets/4207a4d0-9716-47e5-a0d4-f0260c318b90" />
<img width="1181" height="566" alt="WhatsApp Image 2026-06-21 at 23 08 56(2)" src="https://github.com/user-attachments/assets/69661d29-962f-4911-84a7-b7e1d78e80bb" />
<img width="1188" height="574" alt="WhatsApp Image 2026-06-21 at 23 08 57" src="https://github.com/user-attachments/assets/ec9653cd-264e-4fa3-9a5f-678c0082c53e" />

