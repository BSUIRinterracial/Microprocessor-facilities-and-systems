/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 * 
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//   MSP430F552x Demo - ADC12, Sample A0, Set P1.0 if A0 > 0.5*AVcc
//
//   Description: A single sample is made on A0 with reference to AVcc.
//   Software sets ADC12SC to start sample and conversion - ADC12SC
//   automatically cleared at EOC. ADC12 internal oscillator times sample (16x)
//   and conversion. In Mainloop MSP430 waits in LPM0 to save power until ADC12
//   conversion complete, ADC12_ISR will force exit from LPM0 in Mainloop on
//   reti. If A0 > 0.5*AVcc, P1.0 set, else reset.
//
//                MSP430F552x
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//     Vin -->|P6.0/CB0/A0  P1.0|--> LED
//
//   Bhargavi Nisarga
//   Texas Instruments Inc.
//   April 2009
//   Built with CCSv4 and IAR Embedded Workbench Version: 4.21
//******************************************************************************

#include <msp430.h>

#define DELTA 6250;

/*  Глобальные переменные  */
volatile short counter = 0;
volatile short limit = 1;



/*  Объявление функций  */

void main(void) {
        WDTCTL = WDTPW + WDTHOLD;    // отключаем сторожевой таймер

		P1SEL &= ~BIT0;

		P1REN |= BIT7;
		P2REN |= BIT2;

		P1OUT |= BIT7;
		P2OUT |= BIT2;

		P1DIR |= BIT0;                          // P1.0 output 	LED1
		P1DIR &= ~BIT7;							// P1.7 input   S1
		P2DIR &= ~BIT2;							// P2.2 input   S2


        P2IE |= BIT2;
        P2IES |= BIT2;
        P2IFG &= ~BIT2;

        P1IE |= BIT7;
		P1IES |= BIT7;
		P1IFG &= ~BIT7;

        TA2CCTL0 = CCIE; // Enable counter interrupt on counter compare register 0
        TA2CTL = TASSEL__SMCLK +ID__2 + MC__CONTINOUS; // Use the SMCLK to clock the counter, SMCLK/8, count up mode 8tick/s
        
        __bis_SR_register(LPM0_bits + GIE);

        __no_operation();

        for(;;) { // wait for interrupt
        }
}


#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer_A (void){
	counter ++;
	if (counter == 8) {
		counter = 0;
		P1OUT |= BIT0;
	}
	else if (counter == limit) {
		P1OUT &= ~BIT0;
	}
}

#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void){
    if (limit < 7) limit++;
    P2IFG &= ~BIT2;
}


#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void){
    if (limit > 1) limit--;
    P1IFG &= ~BIT7;
}
