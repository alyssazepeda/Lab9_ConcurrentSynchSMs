/*	Author: Alyssa Zepeda
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #9  Exercise #2
 *	Exercise Description: https://youtu.be/pGN6MV13ITA 
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#endif
#define tasksSize 3

/////////////////////////////////////////////////////////////
//struct
typedef struct task {
        int state;
        unsigned long period;
        unsigned long elapsedTime;
        int (*TickFct)(int);
} task;

task tasks[tasksSize];
//GCD of the periods
const unsigned long tasksPeriod = 100;
//periods of individual tasks
const unsigned long periodThreeLEDs = 300;
const unsigned long periodBlinkingLED = 1000;
const unsigned long periodCombineLEDs = 100;
/////////////////////////////////////////////////////////////
//State Machines
//global variables bc they are used in more than 1 SM
unsigned char threeLEDs = 0x00;
unsigned char blinkingLED = 0x00;

enum TL_States{TL_Start, TL_0, TL_1, TL_2} TL_State;
int ThreeLEDsSM_Tick(int state) {
	switch(state) {
		case TL_Start: state = TL_0; break;
		case TL_0: state = TL_1; break;
		case TL_1: state = TL_2; break;
		case TL_2: state = TL_0; break;
		default: state = TL_Start;
	}
	switch(state) {
		case TL_Start: threeLEDs = 0; break;
		case TL_0: threeLEDs = 0x01; break;
		case TL_1: threeLEDs = 0x02; break;
		case TL_2: threeLEDs = 0x04; break;
		default: break;
	}
	return state;
}

enum BL_States{BL_Start, BL_s1} BL_State;
int BlinkingLEDSM_Tick(int state) {
	switch(state) {
		case BL_Start: state = BL_s1; break;
		case BL_s1: state = BL_s1; break;
		default: state = BL_Start; break;
	}
	switch(state) {
		case BL_Start: blinkingLED = 0; break;
		case BL_s1: blinkingLED = !blinkingLED; break;
		default: break;
	}
	return state;
}

enum C_States{Combine_Start, CombineLEDs} C_State;
int CombineLEDsSM_Tick(int state) {
	switch(state) {
		case Combine_Start: state = CombineLEDs; break;
		case CombineLEDs: state = CombineLEDs; break;
		default: state = Combine_Start; break;
	}
	switch(state) {
		case CombineLEDs: PORTB = ((blinkingLED << 3) | threeLEDs); break;
		default: break;
	}
	return state;
}

////////////////////////////////////////////////////////////////
//timer.h functions that needed to be added in main.c
void TimerISR() {
        unsigned char i;
        for (i = 0; i < tasksSize; ++i) {
                if ( tasks[i].elapsedTime >= tasks[i].period ) {
                        tasks[i].state = tasks[i].TickFct(tasks[i].state);
                        tasks[i].elapsedTime = 0;
                }
                tasks[i].elapsedTime += tasksPeriod;
        }
}
ISR(TIMER1_COMPA_vect) {
    _avr_timer_cntcurr--;
    if (_avr_timer_cntcurr == 0){
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

//////////////////////////////////////////////////////////////////
//main
int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
    /* Insert your solution below */
	unsigned char i = 0;
	tasks[i].state = TL_Start;
	tasks[i].period = periodThreeLEDs;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &ThreeLEDsSM_Tick;
	++i;
	tasks[i].state = BL_Start;
        tasks[i].period = periodBlinkingLED;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &BlinkingLEDSM_Tick;
        ++i;
	tasks[i].state = Combine_Start;
        tasks[i].period = periodCombineLEDs;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &CombineLEDsSM_Tick;
      
	TimerSet(tasksPeriod);
	TimerOn();
    while (1) {}
    return 0;
}
