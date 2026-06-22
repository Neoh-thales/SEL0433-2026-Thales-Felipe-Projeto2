# SEL0433-2026-Thales-Felipe-Projeto2
Projeto 2 de Aplicação de Microcontroladores
# Projeto 2: Aferidor de Temperatura de Forno Industrial
**Disciplina:** SEL0433 - Aplicação de Microprocessadores  
**Aluno:** Thales Vasconcelos Aguiar de Olivera  
**Nº USP:** 15489730
**Aluno:**  Felipe Assis Falvo
**Nº USP:** 

## 1. Introdução
Este repositório contém a solução final para o Projeto 2 (metodologia PBL), que consiste no desenvolvimento de um dispositivo de aferição de temperatura e tempo para fornos industriais. 

O sistema foi desenvolvido em linguagem C utilizando o microcontrolador **PIC18F4550**, simulado no software **SimulIDE** e compilado via **MikroC PRO for PIC**. O objetivo principal foi integrar múltiplos periféricos modernos da família PIC18, incluindo:
* Módulo ADC de 10 bits (para leitura do sensor de temperatura LM35).
* Timers (TMR0 e TMR1) para geração de bases de tempo precisas.
* Sistema de Interrupções Externas (INT0 e INT1) para controle da interface humana.
* Display LCD 16x2 operando em modo 4 bits.

## 2. Requisitos e Implementação
O funcionamento do equipamento baseia-se em medir a temperatura interna do forno na faixa de 0°C a 100°C e exibir uma contagem regressiva selecionável pelo usuário.

* **Interface e Controle:** Foram utilizados botões configurados com interrupção externa (borda de subida). O primeiro botão alterna o modo de aferição entre Curta Duração (10s) e Longa Duração (60s). O segundo botão inicia ou pausa a contagem regressiva e a medição.
* **Medição Analógica:** A medição do sensor LM35 foi emulada utilizando um potenciômetro no pino AN0. Para garantir precisão com a escala de 10mV/°C do LM35, a tensão de referência do ADC (Vref+) foi configurada para uma fonte externa de 1V no pino AN3.
* **Atuação:** Um LED foi alocado no pino RC0 para representar a resistência do forno, ligando para temperaturas abaixo de 60°C e desligando ao ultrapassar 80°C.

## 3. Breve Discussão dos Resultados
A integração dos subsistemas apresentou desafios arquiteturais que foram contornados com práticas de programação defensiva. 

Destaca-se a realocação do Display LCD para a Porta D, garantindo que os pinos de Interrupção Externa (INT0 e INT1) da Porta B ficassem livres para os botões do usuário. Além disso, a inicialização do conversor A/D exigiu intervenção direta no registrador `ADCON1` logo após a chamada da biblioteca `ADC_Init()` para corrigir o zeramento indesejado dos bits de seleção de Vref.

Por fim, visando otimizar o uso da memória RAM do microcontrolador de 8 bits, o processamento da temperatura foi realizado utilizando matemática de inteiros (multiplicação prévia do valor do ADC), eliminando completamente a necessidade de variáveis do tipo `float` e formatando os caracteres ASCII diretamente para o display. O sistema comportou-se de maneira estável e síncrona na simulação.

## 4. Imagens da Simulação e Compilação
*(Adicione aqui os prints exigidos pelo roteiro)*

**Compilação no MikroC (Sucesso e uso de memória):**
![Print do MikroC](link_para_sua_imagem1.png)

**Execução no SimulIDE (Sistema em funcionamento):**
![Print do SimulIDE](link_para_sua_imagem2.png)
