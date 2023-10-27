#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>
#include <usart.h>

#pragma config OSC      =   INTIO67
#pragma config BOREN    =   OFF
#pragma config WDT      =   OFF
#pragma config LVP      =   OFF
#pragma config CCP2MX   =   PORTBE

#define _XTAL_FREQ      	8000000           // Set operation for 8 Mhz

#define DS3231_ID       	0x68

#define ACCESS_CFG      	0xAC
#define START_CONV      	0xEE
#define READ_TEMP       	0xAA
#define CONT_CONV       	0x02

#define DS1621_ID       	0x48
#define ACCESS_CFG      	0xAC
#define START_CONV      	0xEE
#define READ_TEMP       	0xAA
#define CONT_CONV       	0x02

#define ACK             	1
#define NAK             	0

#define ON              	1
#define OFF             	0

//  MAIN SCREEN & TIME SETUP SCREEN
#define start_x             1
#define start_y             2
#define temp_x              28
#define temp_y              13
#define circ_x              40
#define circ_y              26
#define data_tempc_x        15
#define data_tempc_y        26
#define tempc_x             45
#define tempc_y             26
#define cirf_x              95
#define cirf_y              26
#define data_tempf_x        70
#define data_tempf_y        26
#define tempf_x             100
#define tempf_y             26
#define time_x              50
#define time_y              46
#define data_time_x         15
#define data_time_y         56
#define date_x              50
#define date_y              76
#define data_date_x         15
#define data_date_y         86

#define alarm_x             10
#define alarm_y             107
#define data_alarm_x        14
#define data_alarm_y        118
   
#define Fan_x               50
#define Fan_y               107
#define data_Fan_x          50
#define data_Fan_y          118

#define set_dc_x            81
#define set_dc_y            107
#define data_set_dc_x       85
#define data_set_dc_y       118


#define dc_x                17
#define dc_y                130
#define data_dc_x           12
#define data_dc_y           142

#define RPM_x               85
#define RPM_y               130
#define data_RPM_x          80
#define data_RPM_y          142

//  Setup DC Screen   
#define setup_dc_x          15
#define setup_dc_y          80    
#define setup_data_dc_x     40
#define setup_data_dc_y     94     
    
#define TS_1            	1               // Size of Normal Text
#define TS_2            	2               // Size of Number Text 

void interrupt high_priority chkisr() ;

void Initialize_Main_Screen(void);
void Initialize_Setup_Time_Screen(void);
void Initialize_Setup_Alarm_Time_Screen(void); 
void Initialize_Setup_dc_Screen(void);
void Main_Screen(void);

void Setup_Time();
void Setup_Alarm_Time(void);
void Setup_Fan_DC(void);

void Update_Main_Screen(void);
void Update_Setup_Time_Screen(void);
void Update_Setup_Alarm_Time_Screen(void);
void Update_Setup_Screen_Cursor(char);
void Update_Setup_dc_Screen(void);
void Do_Setup();

void DS1621_Init();
char DS1621_Read_Temp(void);

void DS3231_Read_Time(void);
void DS3231_Write_Time(void);

void DS3231_Read_Alarm_Time(void);
void DS3231_Write_Alarm_Time(void);
void DS3231_Turn_Off_Alarm(void);
void DS3231_Turn_Off_Alarm(void);
void DS3231_Turn_On_Alarm(void);
void DS3231_Init(void);
void DS3231_Turn_On_Alarm(void);
void Set_RGB_Color(char);

void T0_ISR();
void INT0_ISR();
void INT1_ISR();
void INT2_ISR();
void Init_Timer_1();

void do_update_pwm(char duty_cycle);
int  bcd_2_dec(char);
int  dec_2_bcd(char);

#define _XTAL_FREQ      8000000

#define TFT_DC          PORTDbits.RD0
#define TFT_CS          PORTDbits.RD1
#define TFT_RST         PORTDbits.RD3
#define RED             PORTDbits.RD5         
#define GREEN           PORTDbits.RD6         
#define BLUE            PORTDbits.RD7  

#define enter_setup     PORTAbits.RA0
#define setup_sel0      PORTAbits.RA1
#define setup_sel1      PORTAbits.RA2

#define alarm_on        PORTEbits.RE0       // put your port assigment here from the schematics
#define fan_on          PORTEbits.RE1       // put your port assigment here from the schematics
#define SEC_LED         PORTEbits.RE2

#define ALARM_MATCH_NOT PORTBbits.RB3

#include "ST7735_TFT.c"                     // Important file that contains important definitions for the Initialize screen subroutines
                                            // and define statements
#define SCL_PIN PORTBbits.RB4
#define SCL_DIR TRISBbits.RB4
#define SDA_PIN PORTBbits.RB5
#define SDA_DIR TRISBbits.RB5
#include "softi2c.c"


char buffer[31] = " ECE3301L Spring 2020\0";
char *nbr;
char *txt;
char tempC[]            = "25";
char tempF[]            = "77";
char time[]             = "00:00:00";
char date[]             = "01/01/00";
char alarm_text[]       = "OFF";
char Fan_text[]         = "OFF";  
char set_dc_text[]      = "100%";
char dc_text[]          = "100%";
char RPM_text[]         = "3600";

int DS1621_tempC, DS1621_tempF;

int INT0_flag, INT1_flag, INT2_flag, T0_flag;
unsigned char second, minute, hour, dow, day, month, year;
int Tach_cnt;
int RPM;
char duty_cycle;
char set_duty_cycle;

//char current_duty_cycle;
char fan_has_been_on;
//BGR No color, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE
char led_array[8] = {0b000,0b001,0b010,0b011,0b100,0b101,0b110,0b111};
char led_array_index=0;
char alarm_has_been_enabled;

int Half_sec_cnt = 0;               // Initialize half_sec count 


char setup_time[]       = "00:00:00";
char setup_date[]       = "01/01/00";
char setup_alarm_time[] = "00:00:00"; 
char setup_dc_text[]    = "050";
    

unsigned char alarm_second, alarm_minute, alarm_hour, alarm_date;
unsigned char setup_alarm_second, setup_alarm_minute, setup_alarm_hour;
unsigned char setup_second, setup_minute, setup_hour, setup_day, setup_month, setup_year;
unsigned char setup_duty_cycle;
unsigned char alarm_mode;
unsigned char fan_mode;
unsigned char MATCHED;

void putch (char c)
{   
    while (!TRMT);       
    TXREG = c;
}

void init_UART()
{
    	OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 25);
    	OSCCON = 0x70;
}

void interrupt  high_priority chkisr() 
{ 
    if (INTCONbits.TMR0IF == 1) T0_ISR(); 
    if (INTCONbits.INT0IF == 1) INT0_ISR(); 
    if (INTCON3bits.INT1IF == 1) INT1_ISR(); 
    if (INTCON3bits.INT2IF == 1) INT2_ISR();
}

void T0_ISR()
{ 
    INTCONbits.TMR0IF=0;        // Clear the interrupt flag 
    T0CONbits.TMR0ON=0;         // Turn off Timer0 
    TMR0H=0x0b;                 // Reload Timer High and 
    TMR0L=0xdb;                 // Timer Low 
    SEC_LED = ~SEC_LED;         // 
    Half_sec_cnt++;             // increment half_sec count    
    T0_flag = 1;                //
    T0CONbits.TMR0ON=1;         // Turn on Timer0  
    Tach_cnt = TMR1L;
    TMR1L = 0;
} 

void INT0_ISR() 
{   
    for (int j=0;j<20000;j++);
    INTCONbits.INT0IF=0;            // Clear the interrupt flag 
    INT0_flag = 1;                  // set software INT0_flag 
} 

void INT1_ISR() 
{ 
    for (int j=0;j<20000;j++);
    INTCON3bits.INT1IF=0;           // Clear the interrupt flag 
    INT1_flag = 1;                  // set software INT1_flag 
} 

void INT2_ISR() 
{    
    for (int j=0;j<20000;j++);
    INTCON3bits.INT2IF=0;           // Clear the interrupt flag
    INT2_flag = 1;                  // set software INT2_flag 
} 

void Do_Init()                      // Initialize the ports 
{ 
    init_UART();                    // Initialize the uart
    OSCCON=0x70;                    // Set oscillator to 8 MHz 
    ADCON1=0x0F;                    // Configure all pins to digital 
    TRISA = 0xff;                   // 
    TRISB = 0x0F;                   // Configure the PORTB for the External  
                                    // pins to make sure that all the INTx are // inputs 
    TRISC = 0x01;                   //
    TRISD = 0x00;                   //
    TRISE = 0x03;                   //
    PORTD = 0;    
    RBPU = 0;
    INTCONbits.INT0IF = 0 ;         // Clear INT0IF  
    INTCON3bits.INT1IF = 0;         // Clear INT1IF 
    INTCON3bits.INT2IF =0;          // Clear INT2IF  
    INTCONbits.INT0IE =1;           // Set INT0 IE 
    INTCON3bits.INT1IE=1;           // Set INT1 IE  
    INTCON3bits.INT2IE=1;           // Set INT2 IE  
    INTCON2bits.INTEDG0=0 ;         // INT0 EDGE falling 
    INTCON2bits.INTEDG1=0;          // INT1 EDGE falling 
    INTCON2bits.INTEDG2=1;          // INT2 EDGE rising 
    
    T0CON=0x03;                     // Timer0 off, increment on positive  
                                    // edge, 1:16 pre-scaler 
    TMR0H=0x0b;                     // Set Timer High 
    TMR0H=0xdb;                     // Set Timer high    
    INTCONbits.TMR0IE=1;            // Set interrupt enable 
    INTCONbits.TMR0IF=0;            // Clear interrupt flag 

    T0CONbits.TMR0ON=1;             // Turn on Timer0 
    INTCONbits.GIE=1;               // Set the Global Interrupt Enable 
    I2C_Init(100000);               // Initialize I2C Master with 100KHz clock
    DS1621_Init();
    DS3231_Init();
    Init_Timer_1();
    LCD_Reset();                    // Screen reset
    TFT_GreenTab_Initialize(); 
    alarm_mode = 0;
    MATCHED = 0;
} 

void Init_Timer_1()
{
    TMR1L = 0x00;                   //
    T1CON = 0x03;                   //
}

void main()
{
    Do_Init();                      // Initialization    

    txt = buffer;     
   
    Initialize_Main_Screen();
    alarm_mode = 0;
    fan_mode = 0;    
    set_duty_cycle = 50;
    duty_cycle = 0;
    fan_has_been_on=fan_on;
    alarm_has_been_enabled = alarm_on;
    do_update_pwm(duty_cycle);    
    RPM = 1800; 

    while(TRUE)
    { 
        if (enter_setup == 0)
        {
            Main_Screen();
        }
        else
        {
            Do_Setup();
        }
    }
}

void Main_Screen(void)
{
        if (T0_flag == 1)
        {
            T0_flag = 0;
            if ((Half_sec_cnt %2) == 0)
            {

                DS3231_Read_Time();
                DS1621_tempC = DS1621_Read_Temp();
                DS1621_tempF = DS1621_tempC*9/5 + 32;   
                RPM = Tach_cnt * 60;
                
// PLACE CODE HERE TO MONITOR FAN_ON AND ALARM_ON //TODO finish this
                alarm_mode = alarm_on;//alarm_on
                if(alarm_on)
                {
                    if(!alarm_has_been_enabled)
                    {
                        DS3231_Turn_On_Alarm();
                        alarm_has_been_enabled=1;
                    }
                    if(!ALARM_MATCH_NOT)
                    {
                        Set_RGB_Color(led_array[led_array_index]);
                        led_array_index++;
                        led_array_index &=7;
                    }
                }
                else
                {
                    if(alarm_has_been_enabled)
                    {
                        DS3231_Turn_Off_Alarm();
                        alarm_has_been_enabled=0;
                        led_array_index=0;
                        Set_RGB_Color(0);
                    }
                }
                
                
                
                fan_mode = fan_on;//fan_on
                if(fan_on)
                {
                    if(!fan_has_been_on)
                        duty_cycle = set_duty_cycle;
                    fan_has_been_on=1;
                }
                else
                {
                    fan_has_been_on=0;
                    duty_cycle=0;
                }
                do_update_pwm(duty_cycle);
                
                Update_Main_Screen();
                printf ("Time: %02x:%02x:%02x  Date: %02x:%02x:%02x  ",hour,minute,second,month,day,year);
                printf ("Temp: %2d C %2d F  %d\r", DS1621_tempC,DS1621_tempF, RPM);
            }
        }   
        
        if (INT0_flag == 1)//key up             // if software INT0 flag is set 
        {
            duty_cycle+=5;
            if(duty_cycle==105)
                duty_cycle=0;
			INT0_flag=0;
        }
        
        if (INT1_flag == 1)//key down             // if software INT0 flag is set 
        {
            if(duty_cycle==0)
                duty_cycle=100;
            else
                duty_cycle-=5;
			INT1_flag=0;
        }
        
        if (INT2_flag == 1)             // if software INT0 flag is set 
        { 
            DS3231_Turn_Off_Alarm();
            alarm_has_been_enabled=0;
            led_array_index=0;
            Set_RGB_Color(0);
			INT2_flag=0;
        } 
}

void Initialize_Main_Screen(void) 
{ 
    LCD_Reset();                                                            // Screen reset
    TFT_GreenTab_Initialize();   
    fillScreen(ST7735_BLACK);                                               // Fills background of screen with color passed to it

 
    strcpy(txt, " ECE3301L Spring 2020\0");                                 // Text displayed 
    drawtext(start_x , start_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);   // X and Y coordinates of where the text is to be displayed
    strcpy(txt, "Temperature:");
    drawtext(temp_x  , temp_y , txt, ST7735_MAGENTA, ST7735_BLACK, TS_1);                                                                                               // including text color and the background of it
    drawCircle(circ_x, circ_y , 2  , ST7735_YELLOW);
    strcpy(txt, "C/");
    drawtext(tempc_x , tempc_y, txt, ST7735_YELLOW , ST7735_BLACK, TS_2); 
    drawCircle(cirf_x, cirf_y , 2  , ST7735_YELLOW);
    strcpy(txt, "F");         
    drawtext(tempf_x , tempf_y, txt, ST7735_YELLOW , ST7735_BLACK, TS_2);
    strcpy(txt, "Time");
    drawtext(time_x  , time_y , txt, ST7735_BLUE   , ST7735_BLACK, TS_1);
    strcpy(txt, "Date");
    drawtext(date_x  , date_y , txt, ST7735_RED    , ST7735_BLACK, TS_1);
    strcpy(txt, "ALARM");
    drawtext(alarm_x , alarm_y, txt, ST7735_WHITE   , ST7735_BLACK, TS_1);
    strcpy(txt, "Fan");
    drawtext(Fan_x   , Fan_y  , txt, ST7735_WHITE   , ST7735_BLACK, TS_1);
    strcpy(txt, "Set DC");
    drawtext(set_dc_x, set_dc_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);
    strcpy(txt, "DC");
    drawtext(dc_x    , dc_y   , txt, ST7735_WHITE  , ST7735_BLACK, TS_1);
    strcpy(txt, "RPM");
    drawtext(RPM_x   , RPM_y  , txt, ST7735_WHITE   , ST7735_BLACK, TS_1); 
}


void Update_Main_Screen(void)
{
    tempC[0]  = DS1621_tempC/10  + '0'; // Degree C
    tempC[1]  = DS1621_tempC%10  + '0'; //
    tempF[0]  = DS1621_tempF/10  + '0'; // DEgrees F                    
    tempF[1]  = DS1621_tempF%10  + '0';                     
    time[0]  = (hour>>4)  + '0';        // Hour
    time[1]  = (hour & 0x0f)+ '0';  
    time[3]  = (minute>>4) + '0';       // minute
    time[4]  = (minute&0x0f) + '0';     //  
    time[6]  = (second>>4) + '0';       // second   
    time[7]  = (second&0x0f) + '0';     // 
    date[0]  = (month>>4) + '0';        // month  
    date[1]  = (month&0x0f) + '0';      //
    date[3]  = (day>>4) + '0';          // day
    date[4]  = (day&0x0f) + '0';        // 
    date[6]  = (year>>4) + '0';         // year
    date[7]  = (year&0x0f) + '0';       // 

    char dc_char1 = duty_cycle/100;
    char dc_char2 = (duty_cycle/10)%10;
    char dc_char3 = duty_cycle%10;    
    dc_text[0] = dc_char1 + '0';
    dc_text[1] = dc_char2 + '0';
    dc_text[2] = dc_char3 + '0';

    char RPM1 = (RPM/1000)%10;          // Convert RPM numerical value to ASCII text
    char RPM2 = (RPM/100)%10;           //
    char RPM3 = (RPM/10)%10;            //
    char RPM4 = RPM%10;                 //
    RPM_text[0] = RPM1 + '0';           // 
    RPM_text[1] = RPM2 + '0';           // 
    RPM_text[2] = RPM3 + '0';           // 
    RPM_text[3] = RPM4 + '0';           // 

    char set_dc_char1 = set_duty_cycle/100;            //convert set_duty_cycle value to ASCII text
    char set_dc_char2 = (set_duty_cycle/10)%10;        //  
    char set_dc_char3 = set_duty_cycle%10;             //    
    set_dc_text[0] = set_dc_char1 + '0';               //  
    set_dc_text[1] = set_dc_char2 + '0';               // 
    set_dc_text[2] = set_dc_char3 + '0';               //
    
    if (alarm_mode == 1) 
    {   
        strcpy(alarm_text, "ON ");
    }
    else 
    {
        strcpy(alarm_text, "OFF");
    }
    
    if (fan_mode == 1)
    {
        strcpy(Fan_text, "ON ");
    }
    else 
    {
        strcpy(Fan_text, "OFF");
    }
    
    drawtext(data_tempc_x , data_tempc_y , tempC      , ST7735_YELLOW , ST7735_BLACK , TS_2);       
    drawtext(data_tempf_x , data_tempf_y , tempF      , ST7735_YELLOW , ST7735_BLACK , TS_2);
    drawtext(data_time_x  , data_time_y  , time       , ST7735_CYAN   , ST7735_BLACK , TS_2);
    drawtext(data_date_x  , data_date_y  , date       , ST7735_GREEN  , ST7735_BLACK , TS_2);
    drawtext(data_alarm_x , data_alarm_y , alarm_text , ST7735_MAGENTA, ST7735_BLACK , TS_1);
    drawtext(data_Fan_x   , data_Fan_y   , Fan_text   , ST7735_MAGENTA, ST7735_BLACK , TS_1);
    drawtext(data_set_dc_x, data_set_dc_y, set_dc_text, ST7735_MAGENTA, ST7735_BLACK , TS_1);
    drawtext(data_dc_x    , data_dc_y    , dc_text    , ST7735_MAGENTA, ST7735_BLACK , TS_1);
    drawtext(data_RPM_x   , data_RPM_y   , RPM_text   , ST7735_MAGENTA, ST7735_BLACK , TS_1);     
}

void Do_Setup()
{
    if ((setup_sel1 == 0) && (setup_sel0 == 0)) Setup_Time();
    else if ((setup_sel1 == 0) && (setup_sel0 == 1)) Setup_Alarm_Time();
    else Setup_Fan_DC();
    Initialize_Main_Screen();
}

void Setup_Time(void)
{
    char Key_Next_Flag, Key_Up_Flag, Key_Dn_Flag;
    Key_Next_Flag = Key_Up_Flag = Key_Dn_Flag =0;
    char Select_Field;    
    Select_Field = 0;
    DS3231_Read_Time();
    setup_second = bcd_2_dec(second);
    setup_minute = bcd_2_dec(minute);
    setup_hour = bcd_2_dec(hour);
    setup_day = bcd_2_dec(day);
    setup_month = bcd_2_dec(month);
    setup_year = bcd_2_dec(year);
    Initialize_Setup_Time_Screen();
    
    Update_Setup_Time_Screen();
    
    while (enter_setup == 1)
    {   
        if (T0_flag == 1)
        {
            T0_flag = 0;
        }
          
        if (INT0_flag == 1)         // if software INT0 flag is set 
        { 
            INT0_flag = 0;        
            Key_Up_Flag = 1;        // 
        }
        if (INT1_flag == 1)         // if software INT1 flag is set 
        { 
            INT1_flag = 0;        
            Key_Dn_Flag = 1;;       // 
        }        
        if (INT2_flag == 1)         // if software INT1 flag is set 
        { 
            INT2_flag = 0;        
            Key_Next_Flag = 1;;     // 
        }  

        if (Key_Up_Flag == 1 )
        {
            switch (Select_Field)
            {
                    case 0:
                        setup_hour++;
                        if (setup_hour == 24) setup_hour = 0; 
                        break;

                    case 1:
                        setup_minute++;
                        if (setup_minute == 60) setup_minute = 0;
                        break; 

                    case 2:
                       setup_second++;
                        if (setup_second == 60) setup_second = 0;                        

                        break;   

                    case 3:
                        setup_month++;
                        if (setup_month == 13) setup_month = 1;
                        break;   

                    case 4:
                        setup_day++;
                        if((setup_month == 02) && (setup_day == 30)) 
                        {
                            setup_day = 1;
                        } else if(setup_day == 32) setup_day = 1;
                        break;    

                    case 5:
                        setup_year++;
                        if (setup_year == 100) setup_year = 0;
                        break;    

                    default:
                        break;
            }    
            Update_Setup_Time_Screen();
            Key_Up_Flag = 0;
        }

        if (Key_Dn_Flag == 1 )
        {
            switch (Select_Field)
            {
                    case 0:
                        if (setup_hour == 0) setup_hour = 23;
                        else --setup_hour;
                        break;

                    case 1:
                        if (setup_minute == 0) setup_minute = 59;
                        else --setup_minute;                              
                        break; 

                    case 2:
                        if (setup_second == 0) setup_second = 59;
                        else --setup_second;                        

                        break;   

                    case 3:
                        if (setup_month == 01) setup_month = 12;
                        else --setup_month;
                        break;   

                    case 4:
                        if ((setup_month == 02) && (setup_day == 1)) 
                        {
                            setup_day = 29;
                        } else if (setup_day == 1) 
                        {
                            setup_day = 1;
                        } else --setup_day;
                        break;    

                    case 5:
                        if (setup_year == 0) setup_year = 99;
                        else --setup_year;
                        break;    

                    default:
                    break;
            }                
            Update_Setup_Time_Screen();
            Key_Dn_Flag = 0;                      
        } 

        if (Key_Next_Flag == 1 )
        {        
            Select_Field++;
            if (Select_Field == 6) Select_Field = 0;
            Update_Setup_Screen_Cursor(Select_Field);
            Key_Next_Flag = 0;
        }    
    }         

    DS3231_Write_Time();
    DS3231_Read_Time();
    //Initialize_Main_Screen(); this is now moved to the Do_Setup function. I think thats a better place
          
}
    
void Initialize_Setup_Time_Screen(void) 
{ 
    fillScreen(ST7735_BLACK);                                    // Fills background of screen with color passed to it
 
    strcpy(txt, " ECE3301L Spring 2020\0");                      // Text displayed 
    drawtext(start_x , start_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);
								// X and Y coordinates of where the text is to be displayed

    strcpy(txt, "Time Setup\0");                                // Text displayed 
    drawtext(start_x+3 , start_y+15, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2); 
       
    strcpy(txt, "Time");
    drawtext(time_x  , time_y , txt, ST7735_BLUE   , ST7735_BLACK, TS_1);
    
    fillRect(data_time_x-1, data_time_y+16, 25,2,ST7735_CYAN);
    strcpy(txt, "Date");
    drawtext(date_x  , date_y , txt, ST7735_RED    , ST7735_BLACK, TS_1);
}

void Update_Setup_Time_Screen(void)
{
    setup_time[0]  = (setup_hour/10)  + '0';
    setup_time[1]  = (setup_hour%10)  + '0';  
    setup_time[3]  = (setup_minute/10)  + '0';
    setup_time[4]  = (setup_minute %10)+ '0';
    setup_time[6]  = (setup_second/10)  + '0';
    setup_time[7]  = (setup_second %10)+ '0'; 
    setup_date[0]  = (setup_month/10)  + '0';
    setup_date[1]  = (setup_month %10)+ '0';  
    setup_date[3]  = (setup_day/10)  + '0';
    setup_date[4]  = (setup_day %10)+ '0';
    setup_date[6]  = (setup_year/10)  + '0';
    setup_date[7]  = (setup_year %10)+ '0';

    drawtext(data_time_x, data_time_y, setup_time, ST7735_CYAN, ST7735_BLACK, TS_2);
    drawtext(data_date_x, data_date_y, setup_date, ST7735_GREEN, ST7735_BLACK, TS_2);
}

void Update_Setup_Screen_Cursor(char cursor_position)
{
    char xinc = 36;
    char yinc = 30;   
    switch (cursor_position)
    {
        case 0:
            fillRect(data_time_x-1+2*xinc, data_time_y+16+yinc, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+2*xinc, data_time_y+16, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1, data_time_y+16, 25,2,ST7735_CYAN);
            break;
            
        case 1:
            fillRect(data_time_x-1, data_time_y+16, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+xinc, data_time_y+16, 25,2,ST7735_CYAN);
            break; 
            
        case 2:
            fillRect(data_time_x-1+xinc, data_time_y+16, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+2*xinc, data_time_y+16, 25,2,ST7735_CYAN);
            break;  
            
        case 3:
            fillRect(data_time_x-1+2*xinc, data_time_y+16, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1, data_time_y+16+yinc, 25,2,ST7735_CYAN);
            break;   
            
       case 4:
            fillRect(data_time_x-1, data_time_y+16+yinc, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+xinc, data_time_y+16+yinc, 25,2,ST7735_CYAN);
            break;  

       case 5:
            fillRect(data_time_x-1+xinc, data_time_y+16+yinc, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+2*xinc, data_time_y+16+yinc, 25,2,ST7735_CYAN);
            break;              
    }
}

void Setup_Alarm_Time(void)
{
    char Key_Next_Flag, Key_Up_Flag, Key_Dn_Flag;
    Key_Next_Flag = Key_Up_Flag = Key_Dn_Flag =0;
    char Select_Field;    
    Select_Field = 0;
    DS3231_Read_Alarm_Time();
    setup_alarm_second = bcd_2_dec(setup_alarm_second);
    setup_alarm_minute = bcd_2_dec(setup_alarm_minute);
    setup_alarm_hour = bcd_2_dec(setup_alarm_hour);
    Initialize_Setup_Alarm_Time_Screen();
    
    Update_Setup_Alarm_Time_Screen();
    
    while (enter_setup == 1)
    {   
        if (T0_flag == 1)
        {
            T0_flag = 0;
        }
          
        if (INT0_flag == 1)         // if software INT0 flag is set 
        { 
            INT0_flag = 0;        
            Key_Up_Flag = 1;        // 
        }
        if (INT1_flag == 1)         // if software INT1 flag is set 
        { 
            INT1_flag = 0;        
            Key_Dn_Flag = 1;;       // 
        }        
        if (INT2_flag == 1)         // if software INT1 flag is set 
        { 
            INT2_flag = 0;        
            Key_Next_Flag = 1;;     // 
        }  

        if (Key_Up_Flag == 1 )
        {
            switch (Select_Field)
            {
                    case 0:
                        setup_alarm_hour++;
                        if (setup_alarm_hour == 24) setup_alarm_hour = 0; 
                        break;

                    case 1:
                        setup_alarm_minute++;
                        if (setup_alarm_minute == 60) setup_alarm_minute = 0;
                        break; 

                    case 2:
                       setup_alarm_second++;
                        if (setup_alarm_second == 60) setup_alarm_second = 0;                        

                        break;      
                        
                    default:
                        break;
            }    
            Update_Setup_Alarm_Time_Screen();
            Key_Up_Flag = 0;
        }

        if (Key_Dn_Flag == 1 )
        {
            switch (Select_Field)
            {
                    case 0:
                        if (setup_alarm_hour == 0) setup_alarm_hour = 23;
                        else --setup_alarm_hour;
                        break;

                    case 1:
                        if (setup_alarm_minute == 0) setup_alarm_minute = 59;
                        else --setup_alarm_minute;                              
                        break; 

                    case 2:
                        if (setup_alarm_second == 0) setup_alarm_second = 59;
                        else --setup_alarm_second;                        
                        break;     

                    default:
                    break;
            }                
            Update_Setup_Alarm_Time_Screen();
            Key_Dn_Flag = 0;                      
        } 

        if (Key_Next_Flag == 1 )
        {        
            Select_Field++;
            if (Select_Field == 3) Select_Field = 0;
            Update_Setup_Screen_Cursor(Select_Field);
            Key_Next_Flag = 0;
        }    
    }
    DS3231_Write_Alarm_Time();
}
     
void Initialize_Setup_Alarm_Time_Screen(void) 
{ 
    fillScreen(ST7735_BLACK);                                   // Fills background of screen with color passed to it
 
    strcpy(txt, " ECE3301L Spring 2020\0");                     // Text displayed 
    drawtext(start_x , start_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);
                                                                // X and Y coordinates of where the text is to be displayed

    strcpy(txt, "Alrm Setup");                                  // Text displayed 
    drawtext(start_x+5 , start_y+15, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2); 
    
    strcpy(txt, "Time");
    drawtext(time_x  , time_y , txt, ST7735_BLUE   , ST7735_BLACK, TS_1);
    fillRect(data_time_x-1, data_time_y+16, 25,2,ST7735_CYAN);
}

void Update_Setup_Alarm_Time_Screen(void)
{
    setup_alarm_time[0]  = (setup_alarm_hour/10)  + '0';
    setup_alarm_time[1]  = (setup_alarm_hour%10)  + '0';  
    setup_alarm_time[3]  = (setup_alarm_minute/10)  + '0';
    setup_alarm_time[4]  = (setup_alarm_minute %10)+ '0';
    setup_alarm_time[6]  = (setup_alarm_second/10)  + '0';
    setup_alarm_time[7]  = (setup_alarm_second %10)+ '0';    // Put your code here
    drawtext(data_time_x, data_time_y, setup_alarm_time, ST7735_CYAN, ST7735_BLACK, TS_2);
}
 
void Setup_Fan_DC(void)
{
    //TODO DO THIS
    Initialize_Setup_dc_Screen();
    while(enter_setup)
    {
		if (T0_flag == 1)
        {
            T0_flag = 0;
        }
		if(INT0_flag) //key up
		{
			INT0_flag=0;
			set_duty_cycle+=5;
			if(set_duty_cycle==105)
				set_duty_cycle=0;
			Update_Setup_dc_Screen();
		}
		if(INT1_flag) //key down
		{
			if(set_duty_cycle==0)
				set_duty_cycle=100;
			else
				set_duty_cycle-=5;
			INT1_flag=0;
			Update_Setup_dc_Screen();
		}
	}
}  
    
void Initialize_Setup_dc_Screen(void) 
{ 
    fillScreen(ST7735_BLACK);                                    // Fills background of screen with color passed to it
 
    strcpy(txt, " ECE3301L Spring 2020\0");                     // Text displayed 
    drawtext(start_x , start_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);   // X and Y coordinates of where the text is to be displayed

    strcpy(txt, "Fan\0");                                       // Text displayed 
    drawtext(start_x+25 , start_y+15, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2);     
    strcpy(txt, "Setup\0");
    drawtext(start_x+25 , start_y+35, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2);                            

    strcpy(txt, "    Set DC");
    drawtext(setup_dc_x , setup_dc_y, txt, ST7735_YELLOW  , ST7735_BLACK, TS_1);
 
}
    
void Update_Setup_dc_Screen(void)
{
    char dc_char1 = set_duty_cycle/100;
    char dc_char2 = (set_duty_cycle/10)%10;
    char dc_char3 = set_duty_cycle%10;    
    setup_dc_text[0] = dc_char1 + '0';
    setup_dc_text[1] = dc_char2 + '0';
    setup_dc_text[2] = dc_char3 + '0';
    drawtext(setup_data_dc_x, setup_data_dc_y ,setup_dc_text, ST7735_BLUE, ST7735_BLACK, TS_2);
}

void do_update_pwm(char duty_cycle) 
{ 
    float dc_f;  
    int dc_I;    
    PR2 = 0b00000100 ;                                    // set the frequency for 25 Khz   
    T2CON = 0b00000111 ;                                  //     
    dc_f = ( 4.0 * duty_cycle / 20.0) ;                   // calculate factor of duty cycle versus a 25 Khz           
                                                          // signal     
    dc_I = (int) dc_f;                                    // get the integer part   
    if (dc_I > duty_cycle) dc_I++;                        // round up function    
    CCP1CON = ((dc_I & 0x03) << 4) | 0b00001100;    
    CCPR1L = (dc_I) >> 2; 
}

char DS1621_Read_Temp()
{
char Device=0x48;
char Cmd = 0xAA;
char Data_Ret;    
  I2C_Start();                      // Start I2C protocol
  I2C_Write((Device << 1) | 0);     // Device address
  I2C_Write(Cmd);                   // Send register address
  I2C_ReStart();                    // Restart I2C
  I2C_Write((Device << 1) | 1);     // Initialize data read
  Data_Ret = I2C_Read(NAK);         //
  I2C_Stop(); 
  return Data_Ret;
}

void DS1621_Init()
{
    char Device=0x48;
    I2C_Write_Cmd_Write_Data (Device, ACCESS_CFG, CONT_CONV);
    I2C_Write_Cmd_Only(Device, START_CONV);
}

void DS3231_Read_Time()
{
    char Device = 0x68;
    char Address = 0x00;
    char Data_Ret;    
    I2C_Start();                      // Start I2C protocol
    I2C_Write((Device << 1) | 0);     // Device address
    I2C_Write(Address);               // Send register address
    I2C_ReStart();                    // Restart I2C
    I2C_Write((Device << 1) | 1);     // Initialize data read
    second = I2C_Read(ACK);           //
    minute = I2C_Read(ACK);
    hour = I2C_Read(ACK);
    dow = I2C_Read(ACK);
    day = I2C_Read(ACK);
    month = I2C_Read(ACK);
    year = I2C_Read(NAK);
    I2C_Stop();
}

void DS3231_Write_Time()
{
char Device = 0x68;
char Address = 0x00;

    second = dec_2_bcd(setup_second);
    minute = dec_2_bcd(setup_minute);
    hour = dec_2_bcd(setup_hour);
    dow = 0;
    day = dec_2_bcd(setup_day);
    month = dec_2_bcd(setup_month);
    year = dec_2_bcd(setup_year);
               
    I2C_Start();                      // Start I2C protocol
    I2C_Write((Device << 1) | 0);     // Device address Write mode
    I2C_Write(Address);               // Send register address

    I2C_Write(second);                // Reset seconds as 0 and start oscillator
    I2C_Write(minute);                // Write minute value to DS3231
    I2C_Write(hour);                  // Write hour value to DS3231
    I2C_Write(dow);                   // Write day of week to DS3231
    I2C_Write(day);                   // Write date value to DS3231
    I2C_Write(month);                 // Write month value to DS3231
    I2C_Write(year);                  // Write year value to DS3231
    I2C_Stop(); 
  }

void DS3231_Read_Alarm_Time()
{
char Device = 0x68;
char Address = 0x07;

    I2C_Start();                                // Start I2C protocol
    
    I2C_Write((Device << 1) | 0);               // Device address
    I2C_Write(Address);                         // Send register address
    I2C_ReStart();                              // Restart I2C
    I2C_Write((Device << 1) | 1);               // Initialize data read
    alarm_second = I2C_Read(ACK);               // Read seconds from register 7
    alarm_minute = I2C_Read(ACK);               // Read minutes from register 8
    alarm_hour   = I2C_Read(ACK);               // Read hour from register 9
    alarm_date   = I2C_Read(NAK);               // Read hour from register A
    
    I2C_Stop(); 
}

void DS3231_Write_Alarm_Time()
{
    char Device = 0x68;
    char Address = 0x07;

    setup_alarm_second = dec_2_bcd(setup_alarm_second);
    setup_alarm_minute = dec_2_bcd(setup_alarm_minute);
    setup_alarm_hour = dec_2_bcd(setup_alarm_hour);
    //dow = 0;
    //day = dec_2_bcd(setup_day);
    //month = dec_2_bcd(setup_month);
    //year = dec_2_bcd(setup_year);
               
    I2C_Start();                      // Start I2C protocol
    I2C_Write((Device << 1) | 0);     // Device address Write mode
    I2C_Write(Address);               // Send register address

    I2C_Write(setup_alarm_second);    // Reset seconds as 0 and start oscillator
    I2C_Write(setup_alarm_minute);    // Write minute value to DS3231
    I2C_Write(setup_alarm_hour);      // Write hour value to DS3231 
    I2C_Stop(); 
}    


int bcd_2_dec (char bcd)
{
    int dec;
    dec = ((bcd>> 4) * 10) + (bcd & 0x0f);
    return dec;
}

int dec_2_bcd (char dec)
{
    int bcd;
    bcd = ((dec / 10) << 4) + (dec % 10);
    return bcd;
    
}

void DS3231_Init()
{
char Device = 0x68;
char Address_7 = 0x07;
char Address_E = 0x0E;  
char control_E;

    control_E = I2C_Write_Address_Read_One_Byte(Device, Address_E);
    control_E = control_E & 0x01;
    control_E = control_E | 0x25; 
    I2C_Write_Address_Write_One_Byte(Device, Address_E, control_E);
  
    I2C_Start();                                // Start I2C protocol   
    I2C_Write((Device << 1) | 0);               // Device address
    I2C_Write(Address_7);                       // Send register address
    I2C_ReStart();                              // Restart I2C
    I2C_Write((Device << 1) | 1);               // Initialize data read
    alarm_second = I2C_Read(ACK);               // Read seconds from register 7
    alarm_minute = I2C_Read(ACK);               // Read minutes from register 8
    alarm_hour   = I2C_Read(ACK);               // Read hour from register 9
    alarm_date   = I2C_Read(NAK);               // Read hour from register A
      
    alarm_second = alarm_second & 0x7f;         // Mask off bit 7
    alarm_minute = alarm_minute & 0x7f;         // Mask off bit 7 
    alarm_hour   = alarm_hour   & 0x7f;         // Mask off bit 7
    alarm_date   = alarm_date   | 0x80;         // Mask on bit 7
    
    I2C_Start();                                 // Start I2C protocol
    I2C_Write((Device << 1) | 0);                // Device address Write mode
    I2C_Write(Address_7);                        // Send register address 7

    I2C_Write(alarm_second);                     // Reset alarm seconds value to DS3231
    I2C_Write(alarm_minute);                     // Write alarm minute value to DS3231
    I2C_Write(alarm_hour);                       // Write alarm hour value to DS3231
    I2C_Write(alarm_date);                       // Write alarm date value to DS3231    
    I2C_Stop();   
}

void DS3231_Turn_On_Alarm()
{
char Device = 0x68;
char Address_E = 0x0E;   
char Address_F = 0x0F;
char control_E;
char control_F;

    control_E = I2C_Write_Address_Read_One_Byte(Device, Address_E);
    control_E = control_E | 0x01;
    I2C_Write_Address_Write_One_Byte(Device, Address_E, control_E);
    
    control_F = I2C_Write_Address_Read_One_Byte(Device, Address_F);
    control_F = control_F & 0xFE;
    I2C_Write_Address_Write_One_Byte(Device, Address_F, control_F);
    DS3231_Init();
    
}

void DS3231_Turn_Off_Alarm()
{
char Device = 0x68;
char Address_E = 0x0E;   
char Address_F = 0x0F;
char control_E;
char control_F;

    control_E = I2C_Write_Address_Read_One_Byte(Device, Address_E);
    control_E = control_E & 0xFE;
    I2C_Write_Address_Write_One_Byte(Device, Address_E, control_E);
    
    control_F = I2C_Write_Address_Read_One_Byte(Device, Address_F);
    control_F = control_F & 0xFE;
    I2C_Write_Address_Write_One_Byte(Device, Address_F, control_F);
    DS3231_Init();
}

void Set_RGB_Color(char color)
{
    PORTD = PORTD & 0x1f;
    PORTD = PORTD | (color << 5);
}
