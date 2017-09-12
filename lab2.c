#include <msp430.h>

int main(void) {
	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
	P1DIR |= BIT0;                            // Set P1.0 to output direction
	P1REN |= BIT7;                            // Enable P1.4 internal resistance
	P1OUT |= BIT7;                            // Set P1.4 as pull-Up resistance
	P1IES |= BIT7;                            // P1.4 Hi/Lo edge
	P1IFG &= ~BIT7;                           // P1.4 IFG cleared
	P1IE |= BIT7;                             // P1.4 interrupt enabled

	__bis_SR_register(LPM4_bits + GIE);
	// Enter LPM4 w/interrupt
	__no_operation();                         // For debugger
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	P1OUT ^= BIT0;                            // P1.0 = toggle
	P1IFG &= ~BIT7;                          // P1.4 IFG cleared
}

