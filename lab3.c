#include <msp430.h>

/*
 * main.c
 */
int lpmFlag = 0;
int volt_freqFlag = 1;

//void setVcoreUp (unsigned int );
//void setVcoreDown (unsigned int );
void setFreq ();

void setVcoreUp (unsigned int level) {
  // Subroutine to change core voltage
  // Open PMM registers for write
  PMMCTL0_H = PMMPW_H;
  // Set SVS/SVM high side new level
  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
  // Set SVM low side to new level
  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
  // Wait till SVM is settled
  while ((PMMIFG & SVSMLDLYIFG) == 0);
  // Clear already set flags
  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
  // Set VCore to new level
  PMMCTL0_L = PMMCOREV0 * level;
  // Wait till new level reached
  if ((PMMIFG & SVMLIFG))
    while ((PMMIFG & SVMLVLRIFG) == 0);
  // Set SVS/SVM low side to new level
  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
  // Lock PMM registers for write access
  PMMCTL0_H = 0x00;
}

void setVcoreDown (unsigned int level){
    PMMCTL0_H = PMMPW_H;

    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;

    while ((PMMIFG & SVSMLDLYIFG) == 0);
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
    PMMCTL0_L = PMMCOREV0 * level;
    PMMCTL0_H = 0x00;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    setVcoreUp (0x01);
    setVcoreUp (0x02);
    setVcoreUp (0x03);
    setFreq();
    //LED config
    P1DIR |= BIT0; //Led 1
    P8DIR |= BIT2; //led 3
    P1DIR |= BIT1; //led 4
    P1DIR |= BIT2; //led 5
    P1DIR |= BIT5; //led 8

    P8OUT |=  BIT2;
    P1OUT &= ~BIT0;
    	//P1OUT |=  BIT0;
    P1OUT |=  BIT1;
    P1OUT &= ~BIT2;
    	//P1OUT |=  BIT2;
    P1OUT &= ~BIT5;
    	 //P1OUT |=  BIT5;

    //Button config
    P2DIR &= ~BIT2; //button S2
    P2REN |= BIT2;
    P2OUT |= BIT2;

    P1DIR &= ~BIT7; // button S1
    P1REN |= BIT7;
    P1OUT |= BIT7;

    //enable interrupt for buttons

    P1IES |= BIT7; 
    P1IFG &= ~BIT7; 
    P1IE |= BIT7; 

    //S2
	
    P2IES |= BIT2;
    P2IFG &= ~BIT2;
    P2IE |= BIT2;
    ////////////////////////
    // timer setup
    TA0CCTL0 = CCIE;	// enable compare/capture interrupt
    TA0CTL = TASSEL__ACLK | ID_1 | MC__UP | TACLR;
    TA0CTL &= ~TAIFG;
    TA0CCR0 = 10000;

    P7SEL |= BIT7;
    P7DIR |= BIT7;
    //__bis_SR_register(LPM0_bits + GIE);
    _enable_interrupt();
    __no_operation();
	return 0;
}


void setFreq (){
    UCSCTL3 |= SELREF__REFOCLK; //set DCO reference = REFo
    UCSCTL4 |= SELM__DCOCLK | SELA__DCOCLK; //set MCLK and ACLK to DCOCLK

    __bis_SR_register(SCG0); // Disable FLL control

    UCSCTL0 |= DCO0;  // set lowest posible DCOx, MODx
    UCSCTL0 |= MOD0;

    UCSCTL1 = DCORSEL_1; //select DCO range 1(was 3) 0.15 - 3.45MGz;

    UCSCTL2 = FLLD_1 + 8;

    __bic_SR_register(SCG0);
}


#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void){
    P2IE &= ~BIT2;
    if(lpmFlag == 1){
        //__bis_SR_register(LPM0_EXIT);  //__bic_SR_register_on_ext(LPM0_bits ); //auoia ec ?a?eia LPM0
    	//__bic_SR_register(LPM0_bits );
    	__bic_SR_register_on_exit(LPM0_bits);
        lpmFlag = 0;
        P8OUT &= ~BIT2;
        P1OUT |= BIT0;
    }else{
        lpmFlag = 1;
        P8OUT |= BIT2;
        P1OUT &= ~BIT0;
        P2IE |= BIT2;
        P2IFG &= ~BIT2; 
        __bis_SR_register(LPM0_bits + GIE);
    }
    //P8OUT ^= BIT2;
    //P1OUT ^= BIT0;
    P2IE |= BIT2; 		// enabling interrupt 2nd button
    P2IFG &= ~BIT2; 	// reseting interrupt 2nd button
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void){
    P1IE &= ~BIT7;
    if(volt_freqFlag == 1){
        setVcoreDown(0x02);
        UCSCTL4 &= ~SELM__DCOCLK;
        UCSCTL4 &= ~SELA__DCOCLK;
        UCSCTL4 = SELM__VLOCLK | SELA__VLOCLK;
        //UCSCTL4 &= ~SELA__DCOCLK;
		//UCSCTL4 = SELA__VLOCLK;
        //UCSCTL3 |= FLLREFDIV__8;
        volt_freqFlag = 0;
        P1OUT &= ~BIT1;
        P1OUT |=  BIT2;
    }else{
        setVcoreUp(0x03);
        //setFreq ();
        UCSCTL4 &= ~SELM__VLOCLK;
        UCSCTL4 &= ~SELA__VLOCLK;
        UCSCTL4 = SELM__DCOCLK | SELA__DCOCLK;
        //UCSCTL4 &= ~SELA__VLOCLK;
		//UCSCTL4 = SELA__DCOCLK;
        UCSCTL3 |= FLLREFDIV__2;
        //UCSCTL3 &= ~FLLREFDIV__8;
        volt_freqFlag = 1;
        P1OUT |=  BIT1;
        P1OUT &= ~BIT2;
    }
    //P1OUT ^= BIT2;
    //P1OUT ^= BIT1;
    P1IE |= BIT7; 		// enabling interrupt 1st button
    P1IFG &= ~BIT7; 	// reseting interrupt 1nd button
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void){
    P1OUT ^= BIT5;
}
