#INCLUDE <16f887.h>
#DEVICE ADC=10 
#FUSES XT,NOWDT,NOPROTECT,NOLVP,PUT,BROWNOUT, 
#USE delay(internal=4000000)  
#USE i2c(Master,Fast=100000, sda=PIN_C4, scl=PIN_C3,force_sw)  
#DEFINE USE_PORTB_KBD 
#INCLUDE "i2c_Flex_LCD.c"
#INCLUDE <KBD_4x4.c>
#INCLUDE <stdlib.h> 
#USE     STANDARD_IO(B) //activa las resistencias de pull-up
#BYTE porta = 5 // Identificador para el puerto A. 
#BYTE portb = 6 // Identificador para el puerto B. 
#BYTE portc = 7 // Identificador para el puerto C. 
#BYTE portd = 8 // Identificador para el puerto D. 
#BYTE porte = 9 // Identificador para el puerto E.
#DEFINE POWER PORTD,1
#DEFINE STOP PORTD,0
///////////////////VARIABLES Y FUNCIONES///////////////////////////////////////
float duty=0;
int Timer2,Poscaler;
float SP=0;
int16 salida=0;
float error=0;
float PV=0;
char TECLA; //variable para el valor de la tecla presionada
void config();
void pausa();
//////////////FUNCION PARA LEER EL TECLADO/////////////////////////////////////
char tecla_time(void) {
   char tecla='\0'; //ponemos la variable en blanco (la borramos)
   unsigned int16 timeout;
   timeout=0;
   tecla=kbd_getc(); //Captura valor del teclado
      while(tecla=='\0' && (++timeout< (20000))) //preguntamos por el teclado por 100 ms
      {
         delay_us(10);
         tecla=kbd_getc(); //Captura valor del teclado
      }
   return(tecla);
}
///////////////////////////////////////////////////////////////////////////////
void main() {
//////PUERTOS////////////
PORT_B_PULLUPS (0xFF); //usamos las resistencias pull-up
SET_TRIS_A(0B00000001);
SET_TRIS_B(0);
SET_TRIS_C(0);
SET_TRIS_D(0);
PORTC=0;
PORTD=0;
/////////LCD//////////
lcd_init(0x4E,16,2);
lcd_backlight_led(ON); //Enciende la luz de Fondo
lcd_clear();  //Limpia el LCD
///////////PWM/////////////
   Timer2=249;
   Poscaler=1;
   setup_timer_2(t2_div_by_4,Timer2,Poscaler);   //Configuracion de Timer 2 para establecer frec. PWM a 1kHz
   setup_ccp1(ccp_pwm);                //Configurar modulo CCP1 en modo PWM
   setup_adc_ports(sAN0);        //Configurar ADC
   setup_adc(adc_clock_internal);
   set_pwm1_duty(0);
///////////TECLADO//////////////
KBD_INIT();       //Inicializar el driver del teclado
//////////Secuencia de inicio//////////
lcd_gotoxy(1,1);
printf(lcd_putc, "Bienvenido");
lcd_gotoxy(1,2);
printf(lcd_putc, "Fija el SP...");
delay_ms(1000);
config();
lcd_clear();
///////////////////////////////////////
   while(true)
   {
        BIT_CLEAR(STOP);
        BIT_SET(POWER);
        set_adc_channel(0);
        delay_us(100);       
        duty=read_adc(); 
        PV=duty*0.4882;
        error=PV-SP;
        if(error<0){
         error=0;
        }
        salida=(error/0.4882)*50;
        if(salida>1023){
         salida=1023;
        }
        if(salida<10){
           salida=0;
        }
        set_pwm1_duty(salida);
        TECLA=tecla_time(); //Leemos el valor que devuelve la funcion para leer el teclado
        lcd_gotoxy(1, 1);
        printf(lcd_putc, "PV:%.1f ", PV);   //%lu para imprimir el valor de una int16
        switch(TECLA){
         case '#':
            config();//vamos a fijar un SP
            break;
         case '*':
            pausa(); //pausamos el programa
            break;
        }
        
        lcd_gotoxy(1,2);
        printf(lcd_putc, "SP: %.1f", SP);
        delay_ms(100);
   }
}

void config(){
// en esta funcion fijamos un nuevo SP
int i=0;
char teclas[2];
BIT_CLEAR(POWER);
BIT_SET(STOP);
set_pwm1_duty(0);
lcd_clear();
lcd_gotoxy(1,1);
printf(lcd_putc, "SP entre 0 y 99");
lcd_gotoxy(1,2);
printf(lcd_putc, "SP:");
TECLA='\0';
   while(TECLA!='#'){ 
      TECLA='\0';   
      TECLA=tecla_time();
      if(TECLA=='A'||TECLA=='B'||TECLA=='C'||TECLA=='D'){
         switch(TECLA){
            case 'A':
               SP=16;
               TECLA='#';
               break;
            case 'B':
               SP=18;
               TECLA='#';
               break;
            case 'C':
               SP=20;
               TECLA='#';
               break;
            case 'D':
               SP=24;
               TECLA='#';
               break;
         }
         
      }else if(TECLA!='#' && TECLA!='*' && TECLA!='\0' && i<=1){
            teclas[i]=TECLA;
            SP=atol(teclas);
            lcd_gotoxy(4,2);
            printf(lcd_putc,"%.1f", SP);
            i++;
      }
   }
   lcd_clear();
   delay_ms(10);
   printf(lcd_putc, "SP fijado: %.1f",SP);
   delay_ms(1000);
   lcd_clear();
}

void pausa(){
BIT_CLEAR(POWER);
BIT_SET(STOP);
set_pwm1_duty(0);
lcd_clear();
lcd_gotoxy(1,1);
printf(lcd_putc, "PAUSA");
TECLA='\0';
while(TECLA!='*'){
   TECLA=tecla_time();
   }
lcd_clear();
}
