// Nomes: - Felipe Assis Bernardes Falvo, NUSP: 15004433
//        - Thales Vasconcelos Aguiar de Oliveira, NUSP: 15489730
//
// Projeto 2: Aferidor de temperatura de forno industrial
// Entrega final - seleção da temperatura

// Funcionamento do Código e do Circuito:
// - Botão 1(RB0/INT0): Alterna a seleção entre contagem curta (10s) e contagem longa (60s)
// - Botão 2(RB1/INT1): Inicia a contagem e a leitura de temperatura
// - LED(RE0): Liga quando a temperatura é maior que 50 graus
// - Display LCD mostra a temperatura na linha 1 e a contagem na linha 2
//
// Habilitar na aba Library Managner:
// microkroE -> System Libraries -> ADC/Conversions/C_String/Lcd/Lcd_Constant
//
// Componentes utilizados para o circuito:
// - Microprocessador PIC18F4550
// - Display LCD: Ligado na porta D (D0-D5)
// - Botão 1: Ligado em RB0 (INT0) com resistor pull-down (10k ohms)
// - Botão 2: Ligado em RB1 (INT1) com resistor pull-down (10k ohms)
// - Potenciômetro: Ligado em AN0 (RA0), alimentado com 1V, simulando o sensor LM35
// - Vref+: Ligado em AN3 (RA3) ligado a fonte externa de 1V
// - Vref-: Ligado em AN2 (RA2) ligado ao GND
// - LED: Ligado em RE0 com resistor (330 ohms)

// Para a seleção das entradas, o mesmo foi baseado no manual do EasyPIC
// Como o display LCD funciona com 4 bits, será posto os pinos de dados e controle
// nos pinos da porta D do PIC
sbit LCD_RS at LATD4_bit;
sbit LCD_EN at LATD5_bit;
sbit LCD_D4 at LATD0_bit;
sbit LCD_D5 at LATD1_bit;
sbit LCD_D6 at LATD2_bit;
sbit LCD_D7 at LATD3_bit;

// Configuração da direção dos pinos
// Tem-se 0 para Saída e 1 para Entrada
sbit LCD_RS_Direction at TRISD4_bit;
sbit LCD_EN_Direction at TRISD5_bit;
sbit LCD_D4_Direction at TRISD0_bit;
sbit LCD_D5_Direction at TRISD1_bit;
sbit LCD_D6_Direction at TRISD2_bit;
sbit LCD_D7_Direction at TRISD3_bit;

// Variáveis de controle de tempo e display:

// 1) Vetor Display: vetor string utilizado pela função IntToStr() para armazenar
// a conversão dos números de tempo (int) em texto (char), fazendo com que seja
// possível mostrar os números no display LCD
char vetor_tempo[10];

// 2) Vetor temperatura:Vetor para formatar a temperatura do sensor de maneira
// manual, mostrando "XX.X oC" sem usar ponto flutuante
char vetor_temperatura[17];

// 3) Controle do modo de contagem:
// selecao_modo -> 0 = contagem curta (10s), 1 = contagem longa (60s)
// O botão 1 alterna entre esses modos
int selecao_modo = 0;

// 4) Controle da contagem regressiva:
// tempo_restante -> Guarda o valor da contagem regressiva
int tempo_restante = 0;

// contar_1s_com_250ms: Acumula 4 estouros do Timer1 (4 x 250ms = 1s).
// Necessário porque o limite do Timer1 não alcança 1 segundo de uma única vez
int contar_1s_com_250ms = 0;

// flag_contagem_ligado -> Bit que indica se a contagem está acontecendo,
// não deixando que outro botão interrompa a contagem
bit flag_contagem_ligado;

// flag_modo_timer -> Bit auxiliar para a rotina de interrupção saber
// qual timer deve diminuir o tempo_restante (1 = Timer1, 0 = Timer0)
bit flag_modo_timer;

// 4) Variaveis do ADC e de temperatura
unsigned int valor_adc; // Valor do ADC (0 a 1023)
unsigned long temperatura_decimais; // Temperatura com decimais (exemplo: 253 = 25.3 C)

// CALCULOS DOS TIMERS (Clock de 8 MHz -> Ciclo de maquina: 0,5 us)

// TIMER0 (1 segundo para contagem de 60s):
// Tempo desejado: 1.000.000 us (1s)
// Ciclos = 1.000.000 us / 0.5 us = 2.000.000 ciclos
// Prescaler: 1:64
// Incrementos no timer = 2.000.000 / 64 = 31.250
// Recarga = 65536 - 31250 = 34286 -> 0x85EE

// TIMER1 (250 milissegundos para contagem de 10s):
// Tempo desejado: 250.000 us (250ms)
// Ciclos = 250.000 us / 0.5 us = 500.000 ciclos
// Prescaler: 1:8
// Incrementos no timer = 500.000 / 8 = 62.500
// Recarga = 65536 - 62500 = 3036 -> 0x0BDC

// Interrupção:
void Interrupt() {

    // Botão 1 apertado (Pino RB0/INT0) - Muda o modo de contagem
    if (INTCON.INT0IF == 1) {
        // Tratamento de bouncing
        Delay_ms(20);

        if (PORTB.RB0 == 1) {
            // Só pode mudar o modo se a contagem não estiver acontecendo
            if (flag_contagem_ligado == 0) {
                // Alterna entre modo curto (0) e modo longo (1)
                if (selecao_modo == 0)
                    selecao_modo = 1;
                else
                    selecao_modo = 0;
            }
        }
        // Limpa a flag do botão
        INTCON.INT0IF = 0;
    }

    // Botão 2 apertado (Pino RB1/INT1) - Começa a contagem e a medicao do sensor
    if (INTCON3.INT1IF == 1) {
        // Tratamento de bouncing
        Delay_ms(20);

        if (PORTB.RB1 == 1) {
            // Só inicia se não tiver acontencendo uma contagem
            if (flag_contagem_ligado == 0) {
                flag_contagem_ligado = 1;

                if (selecao_modo == 1) {
                    // Modo longo: 60 segundos usando Timer0
                    tempo_restante = 60;
                    flag_modo_timer = 0;

                    // Desliga Timer1
                    T1CON.TMR1ON = 0;

                    // Pré-carrega o Timer0 para o primeiro ciclo já ter 1s
                    TMR0H = 0x85;
                    TMR0L = 0xEE;

                    // Liga o Timer0
                    T0CON.TMR0ON = 1;
                } else {
                    // Modo curto: 10 segundos usando Timer1
                    tempo_restante = 10;
                    contar_1s_com_250ms = 0;
                    flag_modo_timer = 1;

                    // Desliga Timer0
                    T0CON.TMR0ON = 0;

                    // Pré-carrega o Timer1 para o primeiro ciclo já ter 250ms
                    TMR1H = 0x0B;
                    TMR1L = 0xDC;

                    // Liga o Timer1
                    T1CON.TMR1ON = 1;
                }
            }
        }
        // Limpa a flag do botão
        INTCON3.INT1IF = 0;
    }

    // TEMPORIZADOR 0 - Estouro de 1 segundo (Contagem Longa):

    // Verifica se a flag de interrupção do Timer0 foi ativada pelo hardware
    if (INTCON.TMR0IF == 1) {

        // Recarrega os registradores do Timer0 com o valor inicial calculado (0x85EE)
        // Garante que a próxima contagem de 1 segundo comece certo
        TMR0H = 0x85;
        TMR0L = 0xEE;

        if (flag_contagem_ligado == 1 && flag_modo_timer == 0) {
            // Subtrai 1 do tempo total
            tempo_restante = tempo_restante - 1;

            // Verificar se o cronômetro acabou
            if (tempo_restante <= 0) {
                tempo_restante = 0; // Trava em 0 para não mostrar números negativos na tela
                T0CON.TMR0ON = 0; // Desliga o Timer0
                flag_contagem_ligado = 0; // Abaixa a flag, indicando que a rotina acabou
            }
        }

        // Limpar a flag de interrupção para detectar o próximo estouro de tempo
        INTCON.TMR0IF = 0;
    }

    // TEMPORIZADOR 1 - Estouro de 250 ms (Contagem Curta):

    // Verifica se a flag de interrupção do Timer1 foi ativada
    if (PIR1.TMR1IF == 1) {
        // Recarrega os registradores do Timer1 com o valor inicial (0x0BDC)
        // para manter a precisão de 250ms
        TMR1H = 0x0B;
        TMR1L = 0xDC;

        if (flag_contagem_ligado == 1 && flag_modo_timer == 1) {
            // Soma 1 ao acumulador
            contar_1s_com_250ms = contar_1s_com_250ms + 1;

            // Como o limite do Timer1 é pequeno, verificamos se já
            // acumulamos 4 fatias de 250ms (4 * 250ms = 1000ms = 1 segundo)
            if (contar_1s_com_250ms >= 4) {
                // Zera o acumulador para recomeçar a contagem do próximo segundo
                contar_1s_com_250ms = 0;

                // Subtrai 1 do tempo total restante
                tempo_restante = tempo_restante - 1;

                // Verificar se a contagem curta terminou
                if (tempo_restante <= 0) {
                    tempo_restante = 0; // Trava o valor em zero
                    T1CON.TMR1ON = 0; // Desliga a contagem do Timer1
                    flag_contagem_ligado = 0; // Abaixa a flag de execução
                }
            }
        }

        // Limpa a flag de estouro do Timer1
        PIR1.TMR1IF = 0;
    }
}

// Função principal:
void main() {

    // Configuração dos pinos dos botões como ENTRADA
    TRISB.B0 = 1; // Botão 1 (INT0)
    TRISB.B1 = 1; // Botão 2 (INT1)

    // Configuração do pino do LED como SAÍDA
    TRISE.B0 = 0; // RE0 como saida para o LED
    LATE.B0 = 0;  // LED comeca apagado

    // Inicializa o módulo ADC
    // O conversor ADC deve ser inicializado primeiro pelo MikroC
    // e somente depois o registrador ADCON1 deve ser configurado
    ADC_Init(); // Inicializa o modulo ADC

    // Configura os pinos da porta A usados pelo ADC como ENTRADA
    TRISA.B0 = 1; // AN0 (Potenciometro)
    TRISA.B2 = 1; // AN2 (Vref- ligado ao GND)
    TRISA.B3 = 1; // AN3 (Vref+ ligado a 1V)

    // Configura o ADCON1
    // O valor 0x3A em binario é 0011 1010, que faz o seguinte:
    //
    // 1) Bits 5 e 4 em "11":
    // Informa ao PIC que a tensao maxima (Vref+) é o pino AN3 (onde
    // está a fonte de 1V) e a tensao minima (Vref-) é o do pino AN2
    // (onde está o GND), garantindo que o conversor funcione no
    //  intervalo do sensor LM35
    //
    // 2) Bits 3 a 0 em "1010":
    // Coloca apenas os canais de AN0 ate AN4 em entradas analogicas,
    // garantindo o funcionamento do potenciometro e referencias.
    // Protege a porta B como pinos digitais, evitando que os
    // botoes parem de funcionar
    ADCON1 = 0x3A;

    // T0CON: Registrador de Controle do Timer0
    // Bit 7(0): Inicia com o Timer0 desligado
    // Bit 6(0): Configura como contador de 16 bits
    // Bit 5(0): Usa o clock interno do sistema
    // Bit 4(0): Borda de incremento
    // Bit 3(0): Habilita o uso do Prescaler
    // Bits 2-0(101): Configura o Prescaler para 1:64.
    T0CON = 0b00000101;

    // T1CON: Registrador de Controle do Timer1
    // Bit 7 (1): Habilita leitura/escrita de 16 bits
    // Bit 6 (0): Timer1 não é a principal fonte de clock do sistema
    // Bits 5-4 (11): Configura o Prescaler máximo do Timer1 (1:8)
    // Bit 3 (0): Desliga o oscilador do Timer1
    // Bit 2 (0): Sincronização
    // Bit 1 (0): Usa o clock interno do sistema
    // Bit 0 (0): Inicia com o Timer1 desligado
    T1CON = 0b10110000;

    // Configuração das Interrupções Externas
    // Nível lógico alto indica disparo na borda de subida
    INTCON2.INTEDG0 = 1;
    INTCON2.INTEDG1 = 1;

    INTCON.INT0IE = 1; // Habilita interrupcao externa INT0
    INTCON3.INT1IE = 1;// Habilita interrupcao externa INT1
    INTCON.TMR0IE = 1;// Habilita interrupcao interna do Timer0
    PIE1.TMR1IE = 1; // Habilita interrupcao interna do Timer1

    INTCON.PEIE = 1; // Habilita interrupcoes perifericas
    INTCON.GIE = 1; // Chave geral de uso de interrupcoes

    // Inicialização do LCD
    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);

    while(1) {
        // Configuração para contagem e leitura da temperatura
        if (flag_contagem_ligado == 1) {

            // Leitura do valor do ADC no canal AN0 (potenciometro)
            // Foi usado ADC_Get_Sample() ao invés de ADC_Read()
            // A funcao ADC_Read() apaga as configuracoes no
            // ADCON1 e colocaria a referência em 5V. Ja o ADC_Get_Sample()
            // apenas faz a leitura, respeitando a fonte de 1V (Vref+).
            valor_adc = ADC_Get_Sample(0);

            // Converte o valor do ADC para temperatura para decimais:
            // Com Vref = 1V e LM35 (10mV/grauC):
            // 0V --> 0 grauC,
            // 1V --> 100 grauC
            // ADC 0 --> 0.0 grauC,
            // ADC 1023 --> 100.0 grauC

            // temperatura_decimais = valor_adc * 1000 / 1023
            // Exemplo: valor_adc = 512 -> temperatura_decimais = 500 -> 50.0 grauC
            // Usa unsigned long para evitar overflow
            temperatura_decimais = (unsigned long)valor_adc * 1000 / 1023;

            // formatação a temperatura sem usar float
            vetor_temperatura[0] = (temperatura_decimais / 1000) + '0'; // Centena

            // Se a centena for zero (ex: 50 graus), apaga o zero da esquerda para ficar mais bonito
            if (vetor_temperatura[0] == '0') vetor_temperatura[0] = ' ';
            vetor_temperatura[1] = ((temperatura_decimais / 100)%10) + '0'; // Dezena
            vetor_temperatura[2] = ((temperatura_decimais / 10)%10) + '0'; // Unidade
            vetor_temperatura[3] = '.'; // Ponto
            vetor_temperatura[4] = (temperatura_decimais % 10) + '0'; // Decimal
            vetor_temperatura[5] = ' '; // Espaco
            vetor_temperatura[6] = 223; // Simbolo grau de temperatura
            vetor_temperatura[7] = 'C'; // Celsius
            vetor_temperatura[8] = ' '; // Tirar o que tiver
            vetor_temperatura[9] = '\0'; // Terminar

            // Exibe temperatura na linha 1 do LCD
            Lcd_Out(1, 1, " Graus:");
            Delay_ms(5);
            Lcd_Out(1, 8, vetor_temperatura);

            // Converte o tempo (int) em texto (char) para mostrar no display LCD
            IntToStr(tempo_restante, vetor_tempo);
            Ltrim(vetor_tempo); // Remove os espaços à esquerda

            // Mostra qual modo de contagem no display
            if (flag_modo_timer == 1) {
                Lcd_Out(2, 1, "Tempo:         ");
                Delay_ms(5);
                Lcd_Out(2, 8, vetor_tempo);
            } else {
                Lcd_Out(2, 1, "Tempo:         ");
                Delay_ms(5);
                Lcd_Out(2, 8, vetor_tempo);
            }

            // Controle do LED (RE0):
            // LED acende quando temperatura for superior a 50 graus (500 decimos)
            if (temperatura_decimais > 500) {
                LATE.B0 = 1; // Acende LED
            } else {
                LATE.B0 = 0; // Apaga LED
            }

            Delay_ms(200); // Delay para não bugar a tela
        }
        else {
            // Selecionando o modo de contagem
            if (selecao_modo == 0) {
                Lcd_Out(1, 1, " Modo curto 10s ");
            } else {
                Lcd_Out(1, 1, " Modo longo 60s ");
            }

            Lcd_Out(2, 1, " Aperte botao 1 ");

            // Desliga o LED enquanto não está em alguma contagem
            LATE.B0 = 0;

            Delay_ms(200);
        }
    }
}