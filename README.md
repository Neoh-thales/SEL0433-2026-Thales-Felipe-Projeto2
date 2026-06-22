# Aferidor de Temperatura de Forno Industrial (Baseado em PIC18F4550)

**Disciplina:** SEL0433 - Aplicação de Microprocessadores
 
**Alunos:**
* Thales Vasconcelos Aguiar de Oliveira | NUSP: 15489730 
* Felipe Assis Bernardes Falvo | NUSP: 15004433

## 🚀 1. Visão Geral do Projeto 
Esse projeto baseia-se no desenvolvimento de um sistema embarcado para o monitoramento de tempo e temperatura de um forno industrial. O sistema realiza a leitura contínua de um sensor de temperatura na faixa de 0°C a 100°C e gerencia uma contagem regressiva configurável (10 ou 60 segundos) que impede interrupções externas durante o processo de medição. 

## 🛠️ 2. Arquitetura de Hardware e I/O (Simulador SimulIDE) 
O sistema foi validado no simulador SimulIDE, tomando como referência o Kit EasyPIC v7 com o seguinte mapeamento de periféricos:

* **RB0 (INT0):** Entrada com interrupção externa para o Botão 1. Alterna o modo de contagem entre curta e longa.
* **RB1 (INT1):** Entrada com interrupção externa para o Botão 2. Inicia o ciclo de contagem e medição.
* **AN0 (RA0):** Entrada analógica do canal 0. Conectada ao potenciômetro, simulando o sensor de temperatura LM35.
* **AN3 (RA3) e AN2 (RA2):** Entradas de tensão de referência externa ($V_{ref+}$ ligada a 1V e $V_{ref-}$ ligada ao GND).
* **RD0 a RD5:** Saídas digitais configuradas para o controle e barramento de dados do Display LCD 16x2.
* **RE0:** Saída digital para o acionamento do LED indicador de aquecimento (representando a resistência do forno).

## ⚙️ 3. Firmware 
O sistema foi desenvolvido em linguagem C utilizando o microcontrolador **PIC18F4550** e compilado via **MikroC PRO for PIC**.

### 3.1. Temporização e Bases de Tempo (Timers)
O firmware utiliza dois temporizadores internos baseados em um cristal oscilador de 8 MHz para gerar tempos através de interrupções:
* **Timer0:** Configurado com prescaler 1:64 e recarga inicial (`0x85EE`) para estourar a cada 1 segundo, controlando a contagem regressiva longa de 60s.
* **Timer1:** Configurado com prescaler 1:8 e recarga inicial (`0x0BDC`) para estourar a cada 250ms. O sistema utiliza um acumulador de software que, ao atingir 4 fatias de tempo ($4 \times 250\text{ms} = 1\text{s}$), decrementa o tempo da contagem curta de 10s.

### 3.2. Conversão A/D e Otimização Sem Float
Para ajustar a leitura analógica à sensibilidade do sensor LM35, o registrador `ADCON1` é configurado com o valor `0x3A`. Essa linha de código deve ser executada *após* a inicialização da biblioteca (`ADC_Init()`), corrigindo um problema (visualizado pelos alunos durante o desenvolvimento do firmware) do compilador MikroC, que zera os bits 4 e 5 do registrador, forçando o microcontrolador a adotar as tensões de alimentação internas ($V_{DD}$ de 5V e $V_{SS}$ de 0V) como referências do módulo A/D. 

Desse modo, com a correção, o circuito passa a respeitar as fontes externas de referência de 1V ($V_{REF+}$ em AN3) e GND ($V_{REF-}$ em AN2). É importante destacar que a função convencional `ADC_Read()` possui limitações documentadas na biblioteca ADC do MikroC, visto que ela não foi projetada para trabalhar com fontes externas de tensão de referência, tendendo a redefinir o registrador para utilizar a alimentação padrão do microcontrolador ($V_{DD}$). Por isso, a leitura do potenciômetro é executada através da sub-rotina `ADC_Get_Sample(0)`, que mantém as configurações de Vref externas. 

Ademais, o cálculo final da temperatura foi projetado com matemática de inteiros (`unsigned long`), extraindo centenas, dezenas e unidades para formatar e posicionar os caracteres ASCII no display ("XX.X °C"), eliminando o uso de variáveis do tipo `float`.

### 3.3. Controle de Fluxo, Alarme e Interface
O controle do sistema baseia-se em colocar os eventos críticos de entrada em uma interrupção assíncrona, enquanto o loop principal funciona em primeiro plano atualizando os periféricos de saída com base em variáveis de estado (*flags*).

* **Tratamento de Interrupções e Debounce:** O Botão 1 (alternância de Modo) está relacionado à interrupção externa **INT0** (`RB0`) e o Botão 2 (início da contagem) à **INT1** (`RB1`). Dentro da interrupção (`void Interrupt()`), o efeito *bouncing* é resolvido através de um atraso controlado de 20 ms (`Delay_ms(20)`) seguido pela checagem do nível lógico alto do pino, validando o acionamento somente na borda de subida.
* **Trava de Estado:** Utilizou-se uma lógica através do bit `flag_contagem_ligado`, na qual, assim que a contagem é iniciada pelo Botão 2, essa flag é colocada em `1`. Enquanto estiver ativa, a interrupção ignora qualquer nova requisição de mudança de modo ou reinicialização dos botões, garantindo que a medição da temperatura do forno seja feita de forma segura até o fim do tempo programado.
* **Loop Principal:** O laço de repetição `while(1)` divide o comportamento da interface em duas partes:
  1. *Modo Configuração (`flag_contagem_ligado == 0`):* O display mostra o modo selecionado para a próxima operação ("Modo curto 10s" ou "Modo longo 60s") e solicita a interação do operador. Nessa parte, o **Botão 1** (conectado à entrada `RB0/INT0` e posicionado mais embaixo no hardware) é o responsável por alternar entre as opções de tempo. O **Botão 2** (conectado à entrada `RB1/INT1` e localizado mais acima no circuito) é o responsável por iniciar a aferição. O LED de aquecimento é mantido desligado.
  2. *Modo Medição (`flag_contagem_ligado == 1`):* O firmware começa a ler o potenciômetro. A linha 1 do LCD é atualizada com a temperatura convertida e a linha 2 exibe o tempo restante em contagem regressiva decrementado pelos estouros dos timers.
* **Lógica do Alarme Térmico:** No modo de medição, o sistema verifica continuamente a variável `temperatura_decimais`. Caso o valor ultrapasse o limiar de `500` (que representa 50.0°C em escala de inteiros), o pino `RE0` é acionado via registrador `LATE.B0`, ligando o LED indicador de potência da resistência do forno. Se o valor estiver abaixo, a saída é desativada.
* **Estabilização do Display:** Foi inserido um atraso de 200 ms (`Delay_ms(200)`). Esse delay atua com o objetivo de controlar a taxa de atualização do Display LCD, impedindo oscilações visuais na tela causadas por leituras rápidas na memória.
  
## 📊 4. Discussão dos Resultados e Conclusão
O desenvolvimento desse projeto indicou a eficiência do uso de interrupções em conjunto com o controle de temporizadores no PIC18F4550. Ademais, a arquitetura orientada a interrupções garantiu que as contagens do **Timer0** e **Timer1** se mantivessem exatas, prevenindo atrasos gerados pela atualização do display no loop principal.

Dos resultados obtidos, destacam-se três pontos principais:

* **Precisão Analógica:** O principal obstáculo técnico do projeto (sobreposição das configurações de tensão de referência pela biblioteca do MikroC) foi resolvido com sucesso através da configuração manual do registrador `ADCON1`. Isso permitiu que a conversão A/D operasse corretamente.
* **Eficiência Computacional:** A decisão de formatar a temperatura no display através de divisões matemáticas utilizando inteiros (`unsigned long`) provou ser eficiente. A eliminação do uso de bibliotecas de ponto flutuante (`float`) resultou em um firmware mais leve, refletindo em um baixo consumo de memória ROM e RAM do microcontrolador na compilação.
* **Confiabilidade:** A implementação da lógica de estado `flag_contagem_ligado` junto com o tratamento de *debounce* provou a eficiência da interface. Dessa forma, o sistema atendeu aos requisitos mencionados pelo professor, garantindo que o ciclo de aquecimento do forno fosse protegido contra ruídos dos botões ou interferências acidentais.

## 💻 5. Como Simular 

<img src="https://github.com/user-attachments/assets/3d1eefab-e678-4757-bfe1-aff7a91d2ae5" width="70%" alt="simulador" />

1. Compile o arquivo `codigo_projeto2.c` no **MikroC PRO for PIC** habilitando as bibliotecas no *Library Manager* para gerar o arquivo `.hex`, ou utilize o mesmo fornecido neste repositório (`codigo_projeto2.hex`).
2. Carregue o circuito abrindo o arquivo `circuito_projeto2.simu` no **SimulIDE**.
3. Clique com o botão direito no microcontrolador PIC18F4550, selecione **Load Firmware** e carregue o arquivo `.hex`.
4. Inicie a simulação clicando no botão de **Power** na barra superior.
5. **Teste de Seleção e Execução:** Pressione o Botão 1 (conectado à entrada `RB0/INT0` e posicionado mais embaixo no hardware) para alternar no LCD entre o modo de 10s e 60s. Escolha o modo e aperte o Botão 2 (conectado à entrada `RB1/INT1` e localizado mais acima no circuito) para começar a medição.
6. **Teste do Sensor e Alarme:** Com a contagem ativa, gire o potenciômetro conectado a `AN0`. O display exibirá a variação de temperatura em tempo real. Caso o valor passe dos 50.0°C, o LED (conectado em `RE0`) se acenderá.


## 📸 6. Registros Visuais da Execução

### Compilação no MikroC (Sucesso e Uso de Memória)

<img width="1339" height="431" alt="Mensagem de compilação com sucesso" src="https://github.com/user-attachments/assets/78752a1d-cc72-4054-ab74-f9ac523b0fac" />

*(Passo a passo para habilitar as bibliotecas no Library Manager)*

**Passo 1:** Clique na aba vertical `Library Manager` localizada no canto inferior direito da IDE.

<img width="1000" alt="Aba Library Manager" src="https://github.com/user-attachments/assets/e44c4ef1-5fa5-445b-ad72-4e3e1e2d662a" />

**Passo 2:** Expanda o diretório `System Libraries`.

<img width="460" alt="Expandindo System Libraries" src="https://github.com/user-attachments/assets/12f708ad-59fa-46c6-b438-69335daaf9cb" />

**Passo 3:** Habilite as bibliotecas marcando as caixas `ADC`, `Conversions`, `C_String`, `Lcd` e `Lcd_Constants`.

<img width="460" alt="Bibliotecas Selecionadas" src="https://github.com/user-attachments/assets/65e5eb6d-62e0-44ed-8cc2-d4ad6069ad86" />


### Execução no SimulIDE (Sistema em Funcionamento)

As imagens a seguir demonstram o comportamento do firmware em tempo real, validando a transição entre os estados de configuração e operação do microcontrolador e a resposta dos periféricos.

**1. Seleção de Modo Longo (60s)**
> *Mostra o sistema em Modo Configuração. O display aguarda a ação do usuário, exibindo a pré-seleção de 60 segundos feita através do Botão 1 (`RB0`). Por questões de segurança do projeto, o pino `RE0` mantém o LED desativado.*

<img width="1198" height="581" alt="Seleção de modo longo" src="https://github.com/user-attachments/assets/4207a4d0-9716-47e5-a0d4-f0260c318b90" />

**2. Contagem Ativa com Sensor a 93.3°C (LED Alarme Ligado)**
> *Mostra o Modo de Medição em funcionamento após pressionar o Botão 2 (`RB1`). A interface é travada contra interrupções. A linha inferior mostra o Timer atualizando o tempo restante com precisão, enquanto o ADC converte a tensão do potenciômetro para 93.3°C. Como o valor supera o limiar crítico de 50°C, o sistema aciona o LED em `RE0`.*

<img width="1181" height="566" alt="Medição ativa a 93 graus" src="https://github.com/user-attachments/assets/69661d29-962f-4911-84a7-b7e1d78e80bb" />

**3. Aferição Limite com Sensor a 100.0°C**
> *Comprova a estabilidade da conversão A/D no limite superior exigido pelo projeto. Colocar a tensão de referência externa ($V_{ref+}$ de 1V) no registrador `ADCON1` garante que a conversão exiba 100.0°C, acionando o LED.*

<img width="1188" height="574" alt="Medição no limite de 100 graus" src="https://github.com/user-attachments/assets/ec9653cd-264e-4fa3-9a5f-678c0082c53e" />
