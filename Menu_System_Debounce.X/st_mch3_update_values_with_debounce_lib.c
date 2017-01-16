/***************************************************
Name:- Tommy Gartlan
Date last modified:- Jan 2016
Updated: Update to use xc8 compiler within Mplabx

Filename:- st_mch3_2_debounce2b.c
Program Description:- Using a simple state machine
to loop through a menu system.
 
Rule of thumb: Always read inputs from PORTx and write outputs to LATx. 
  If you need to read what you set an output to, read LATx.
 * 
 * This version combines st_mch3 and debounce2b so that buttons are debounced
 * properly for the menu system
 * 
 * this follow on version uses two buttons'UP and 'DOWN' and and an 'Enter, button to update values 
 * Good example of a menu system where values can be updated.
*****************************************************************************************************/

/****************************************************
		Libraries included
*****************************************************/
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../Buttons_Debounce_State_Mch/Buttons_Debounce.X/Buttons_Debounce.h"
#include "../../LCD_library/lcdlib_2016.h"

#include <plib/timers.h>
//#include <plib/usart.h>
//#include <plib/can2510.h>

/***********************************************
		Configuration Selection bits
************************************************/

/*************************************************
					Clock
*************************************************/
#define _XTAL_FREQ 19660800

/************************************************
			Global Variables
*************************************************/
const unsigned char msg_ary[10][16] = {"A Value> ", "B Value> ", 
                                        "A Value> ","New A?> ", 
                                        "B Value> ", "New B?> ",
                                        "C Value> ", "New C?> "};

const unsigned char * problem = "Problem";

const unsigned char * startup = "Ready to go";
										  

/************************************************
			Function Prototypes
*************************************************/
void Initial(void);
void Window(unsigned char num);
void delay_s(unsigned char secs);


/************************************************
 Interrupt Function 
*************************************************/
unsigned char count_test =0;
void __interrupt myIsr(void)
{
    //Timer overflows every 10mS
    // only process timer-triggered interrupts
    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF) {
        
        Find_Button_Press();       //check the buttons every 10mS
        WriteTimer0(40960); 
        INTCONbits.TMR0IF = 0;  // clear this interrupt condition
        
        //Heartbeat signal
        count_test++;
        if(count_test == 100){
            PORTCbits.RC7 = ~PORTCbits.RC7;   //check the timer overflow rate
            count_test = 0;                   //Toggle every 1 second (heartbeat))
        }
    }
}


//declare Button
Bit_Mask Button_Press;	


/************************************************
			Macros
*************************************************/
#define MENU_E Button_Press.B0
#define ENTER_E Button_Press.B1
#define UP_E Button_Press.B2
#define DOWN_E Button_Press.B3
/*****************************************
 			Main Function
******************************************/

void main ( void ) 
{
    unsigned char A = 20;
    unsigned char B = 30;
    unsigned char C = 40;
    unsigned char Temp_Value = 0;
    
    typedef  enum {MENU_0 = 0,MENU_1,MENU_2, MENU_3} states;
    states  my_mch_state = MENU_3;
    
    Initial();
    lcd_start ();
    lcd_cursor ( 0, 0 ) ;
    lcd_print ( startup ) ;
    
    delay_s(2);
    //Initial LCD Display
    Window(0);
    lcd_cursor ( 10, 0 ) ;
    lcd_display_value(A);
    lcd_cursor ( 10, 1 ) ;
    lcd_display_value(B);
    
    while(1)
    {
		
		//wait for a button Event
		//while(!MENU_E && !ENTER_E && !UP_E && !DOWN_E);  
		while(!Got_Button_E);
        
		switch(my_mch_state)	
		{
			case MENU_0: 
				if (MENU_E){
                    my_mch_state = MENU_1; //state transition
                    Window(1);             //OnEntry action
                }
                
				break;
			case MENU_1: 
				if (MENU_E){
                    my_mch_state = MENU_2;  //state transition
                    Window(2);              //OnEntry action
                }
				break;
			case MENU_2: 
				if (MENU_E){
                    my_mch_state = MENU_3;
                    Window(3);
                }
				break;
			case MENU_3: 
                 if (MENU_E){
                    my_mch_state = MENU_0;
                    Window(0);
                }   
				break;
			default: 
				if (MENU_E){
                    my_mch_state = MENU_0;
                    Window(0);
                }
				break;
		}
		
		
		switch(my_mch_state)	
		{
			case MENU_0: 
				lcd_cursor ( 10, 0 ) ;    //state actions
                lcd_display_value(A);
                lcd_cursor ( 10, 1 ) ;
                lcd_display_value(B);
                LATC = 0x01;
				
				break;
			case MENU_1: 
                if (ENTER_E)          //state actions with guard
                    A = Temp_Value;
                if (UP_E)
                    Temp_Value++;
                if (DOWN_E)
                    Temp_Value = (Temp_Value != 0 ? Temp_Value - 1 : 0);
                
				lcd_cursor ( 10, 0 ) ;
                lcd_display_value(A);
				lcd_cursor ( 10, 1 ) ;
                lcd_display_value(Temp_Value);
                LATC= 0x02;
				break;
			case MENU_2: 
				if (ENTER_E)
                    B = Temp_Value;
                if (UP_E)
                    Temp_Value++;
                if (DOWN_E)
                    Temp_Value = (Temp_Value != 0 ? Temp_Value - 1 : 0);
                
				lcd_cursor ( 10, 0 ) ;
                lcd_display_value(B);
				lcd_cursor ( 10, 1 ) ;
                lcd_display_value(Temp_Value);
                LATC = 0x03;
				break;
			case MENU_3: 
				if (ENTER_E)
                    C = Temp_Value;
                if (UP_E)
                    Temp_Value++;
                if (DOWN_E)
                    Temp_Value = (Temp_Value != 0 ? Temp_Value - 1 : 0);
                
				lcd_cursor ( 10, 0 ) ;
                lcd_display_value(C);
				lcd_cursor ( 10, 1 ) ;
                lcd_display_value(Temp_Value);
                LATC = 0x04;
				break;
			default: 
				lcd_cursor ( 0, 0 ) ;
                lcd_clear();
				lcd_print ( problem );
                LATC = 0x05;
				break;
		}
		
        Button_Press.Full = 0;  //Clear all events since only allowing one button event at a time
                                //which will be dealt with immediately
    
    }
}   


void Initial(void){
    
    ADCON1 = 0x0F;
	TRISB = 0xFF; //Buttons
	TRISC = 0x00;   //LEDS

	LATC = 0xff;
	delay_s(3);
	LATC = 0x00;
    
    //0n, 16bit, internal clock(Fosc/4), prescale by 2)
    OpenTimer0( TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_2);
    WriteTimer0(40960);  //65,536 - 24,576  //overflows every 10mS
    ei();
    
}

void Window(unsigned char num)
{
    lcd_clear();
    lcd_cursor ( 0, 0 ) ;	
	lcd_print ( msg_ary[num*2]);
    lcd_cursor ( 0, 1 ) ;
    lcd_print ( msg_ary[(num*2)+1]);
}


void delay_s(unsigned char secs)
{
    unsigned char i,j;
    for(j=0;j<secs;j++)
    {
        for (i=0;i<25;i++)
                __delay_ms(40);  //max value is 40 since this depends on the _delay() function which has a max number of cycles
        
    }
}

