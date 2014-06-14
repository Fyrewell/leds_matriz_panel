/*
Programa Letreiro Matriz de Leds
write by Diego Cardoso
28/02/2012
SCS-RS
*/

#include <18F4525.h>
#device adc=16

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES WDT128                   //Watch Dog Timer uses 1:128 Postscale
#FUSES XT                       //Crystal osc <= 4mhz for PCM/PCH , 3mhz to 10 mhz for PCD
#FUSES NOFCMEN                  //Fail-safe clock monitor disabled
#FUSES NOIESO                   //Internal External Switch Over mode disabled
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOPBADEN                 //PORTB pins are configured as digital I/O on RESET
#FUSES NOLPT1OSC                //Timer1 configured for higher power operation
#FUSES NOMCLR                   //Master Clear pin used for I/O
#FUSES NOSTVREN                 //Stack full/underflow will not cause reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOXINST                  //Extended set extension and Indexed Addressing mode disabled (Legacy mode)

#use delay(clock=4000000)

/////////////////////////////////////4094/////////////////////////////////////////
#define stb pin_e2
#define data pin_e0
#define clk pin_e1
////////////////////////////////////TECLADO MATRICIAL/////////////////////////////////////////
#define col1 pin_b0
#define c2 pin_b1
#define c3 pin_b2
#define c4 pin_b3
#define lin1 pin_b4
#define lin2 pin_b5
#define lin3 pin_b6
#define lin4 pin_b7

#define config pin_c0

#define SENSOR_SELECT PIN_A5 //habilita chip escravo(slave select)
#define SENSOR_DO PIN_C5 //recebe dados SPI
#define SENSOR_DI PIN_C4 //transmite dados SPI
#define SENSOR_CLK PIN_C3 //clock de sincronismo
/////////////////////////////////////comandos interno do sensor TC72/////////////////////////////////////////
#define SENSOR_ENDE_ESC 0X80 //endereço de escrita
#define SENSOR_ENDE_LER 0X02 //endereço de leitura
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <escritas3.c>

void desloca_4094(unsigned int8 dado, unsigned int8 x,unsigned int8 y);
void passa_bits();
void escreve(unsigned int8 msg);
void decide_onde_escrever(void);
void le_teclado(void);
void identifica_letras(unsigned int8 Ind,unsigned int8 msg,unsigned int8 C1[8][8]);
void escreve_le_temp(void);
void identifica_temperatura(unsigned int8 Ind,unsigned int8 C1[8][8]);
void revela_temp(void);

unsigned int8 caractere[100][35],qual=0,modo=0,val=0,contamsgs=0;
unsigned int8 contmsg=0,contatempo=0;
unsigned int1 flag_config=1;
unsigned int8 conte=0,id=6,cont_vazio=0,conta_l=0,delay_comeco=0;
unsigned int8 control_byte,temperatura[8];
signed int8 MSB,LSB;
unsigned int1 flag_tc=0;

void main()
{
   setup_timer_3(T3_DISABLED|T3_DIV_BY_1);
   setup_spi(SPI_MASTER | SPI_L_TO_H| SPI_CLK_DIV_4| SPI_SS_DISABLED|SPI_SAMPLE_AT_END);//desabilita o chip escravo
   output_low(SENSOR_DO);
   output_low(SENSOR_CLK);
   output_low(SENSOR_SELECT);//desabilita sensor
   LSB=input(SENSOR_DI); //configura pino como entrada
   output_high(SENSOR_SELECT);//habilita sensor
   spi_write(SENSOR_ENDE_ESC);//endereço de escrita
   spi_write(0x10);//configura control register(conversao continua)
   output_low(SENSOR_SELECT);//desabilita sensor
   delay_ms(20);
   //printf("\n\r configurado (conversao continua)");
   
   temperatura[6]=255; //C
   temperatura[5]=254; //grau
   temperatura[3]=253; //virgula
   
   caractere[0][0]=5;
   caractere[1][0]=11;
   caractere[2][0]=6;
   caractere[3][0]=9;
   caractere[4][0]=19;
   caractere[5][0]=0;
   caractere[6][0]=3;
   caractere[7][0]=1;
   caractere[8][0]=23;
   caractere[9][0]=5;
   caractere[10][0]=19;
   caractere[11][0]=25;
   caractere[12][0]=19;
 /*  caractere[13][0]=14;
   caractere[14][0]=15;
   caractere[15][0]=16;
   caractere[16][0]=17;
   caractere[17][0]=18;
   caractere[18][0]=19;
   caractere[19][0]=20;
   caractere[20][0]=21;
   caractere[21][0]=22;*/

   caractere[0][1]=25;
   caractere[1][1]=1;
   caractere[2][1]=18;
   caractere[3][1]=26;
   caractere[4][1]=1;
   caractere[5][1]=0;
   caractere[6][1]=3;
   caractere[7][1]=23;
   caractere[8][1]=27;
   caractere[9][1]=34;
   caractere[10][1]=0;
   caractere[11][1]=5;
   caractere[12][1]=19;
   caractere[13][1]=0;
   caractere[14][1]=25;
   caractere[15][1]=27;
   caractere[16][1]=15;
   caractere[17][1]=40;
   caractere[18][1]=23;
   caractere[19][1]=25;
   /*caractere[20][1]=18;
   caractere[21][1]=17;
   caractere[22][1]=16;
   caractere[23][1]=15;
   caractere[24][1]=14;
   caractere[25][1]=13;
   caractere[26][1]=12;
   caractere[27][1]=11;
   caractere[28][1]=10;
   caractere[29][1]=9;
   caractere[30][1]=8;
   caractere[31][1]=7;
   caractere[32][1]=6;
   caractere[33][1]=5;
   caractere[34][1]=4;
   caractere[35][1]=3;
   caractere[36][1]=2;
   caractere[37][1]=1;*/
   flag_tc=1;
   while(TRUE){
   revela_temp();
   
   if (input(config)==0){
      delay_ms(2);
      contmsg--;
      if (flag_config){
         flag_config=0;
      }else{
         flag_config=1;
      }
      val=0;
      while(input(config)==0);
   }
   
   if (flag_config){
      escreve(val);
      le_teclado();
   }
   
   if (flag_config==0){
      escreve(contmsg);
      if (delay_comeco<=30){
         delay_comeco++;
      }else conte++;
      if ((conte>2)&&(delay_comeco>30)){
         conte=0;
         if (flag_tc==1){
            delay_comeco++;
            if (delay_comeco>45){
               delay_comeco=0;
               flag_tc=0;
            }
         }else{
         id--;
         if (id==0){
            id=6;
            conta_l++;
         }
         }
      }
      if (cont_vazio>25){
         cont_vazio=0;
         contmsg++;
         if (contmsg==2){   //2msg
            contmsg=0;
            flag_tc=1;
         }
         id=6;
         delay_comeco=0;
         conta_l=0;
      }
   }

   }
}


void passa_bits(){
   output_high(stb);
   delay_us(10);
   output_low(stb);
   delay_ms(1);
}

void escreve(unsigned int8 msg){
   int x,valor=255, Ind;
   unsigned int8 C1[8][8];
   if (flag_tc==0){
      for (Ind=0;Ind<8;Ind++)
         identifica_letras(Ind,msg,C1);
   }else{
      for (Ind=0;Ind<8;Ind++)
         identifica_temperatura(Ind,C1);
   }
   x=0;
   
   while(x<8){
   unsigned int8 y;
   if (flag_tc==0){  //desloca pois n eh temp
   for (y=7;y!=0;y--){
      desloca_4094(C1[y][x],6,0);
      }
      desloca_4094(C1[0][x],id,0);
   }else{
      for (y=7;y!=255;y--){
         if ((y==0)||(y==3)) // centralizar ou virgula
            desloca_4094(C1[y][x],3,0);
         else
            desloca_4094(C1[y][x],6,0);
      }
   }
   //inicia varredura
   valor = 0b10000000;
   valor = ~(valor>>x);
   x++;
   output_d(valor);
   passa_bits();
   }
   if (flag_tc==0){
   if ((C1[0][0]==0)&&(C1[0][1]==0)&&(C1[0][2]==0)&&(C1[0][3]==0)&&(C1[0][4]==0)&&(C1[0][5]==0)&&(C1[0][6]==0)&&(C1[0][7]==0)){
      cont_vazio++;
   }else cont_vazio=0;
   }
}

void identifica_letras(unsigned int8 Ind,unsigned int8 msg,unsigned int8 C1[8][8]){
      int x;
      if (caractere[Ind+conta_l][msg]==0){
         for (x=0;x<8;x++){
            C1[Ind][x]=branco[x];
         }
      }else if (caractere[Ind+conta_l][msg]==1){
         for (x=0;x<8;x++){
            C1[Ind][x]=A[x];
         }
      }else if (caractere[Ind+conta_l][msg]==2){
         for (x=0;x<8;x++){
            C1[Ind][x]=B[x];
         }
      }else if (caractere[Ind+conta_l][msg]==3){
         for (x=0;x<8;x++){
            C1[Ind][x]=C[x];
         }
      }else if (caractere[Ind+conta_l][msg]==4){
         for (x=0;x<8;x++){
            C1[Ind][x]=UM[x];
         }
      }else if (caractere[Ind+conta_l][msg]==5){
         for (x=0;x<8;x++){
            C1[Ind][x]=D[x];
         }
      }else if (caractere[Ind+conta_l][msg]==6){
         for (x=0;x<8;x++){
            C1[Ind][x]=E[x];
         }
      }else if (caractere[Ind+conta_l][msg]==7){
         for (x=0;x<8;x++){
            C1[Ind][x]=F[x];
         }
      }else if (caractere[Ind+conta_l][msg]==8){
         for (x=0;x<8;x++){
            C1[Ind][x]=DOIS[x];
         }
      }else if (caractere[Ind+conta_l][msg]==9){
         for (x=0;x<8;x++){
            C1[Ind][x]=G[x];
         }
      }else if (caractere[Ind+conta_l][msg]==10){
         for (x=0;x<8;x++){
            C1[Ind][x]=H[x];
         }
      }else if (caractere[Ind+conta_l][msg]==11){
         for (x=0;x<8;x++){
            C1[Ind][x]=I[x];
         }
      }else if (caractere[Ind+conta_l][msg]==12){
         for (x=0;x<8;x++){
            C1[Ind][x]=TRES[x];
         }
      }else if (caractere[Ind+conta_l][msg]==13){
         for (x=0;x<8;x++){
            C1[Ind][x]=J[x];
         }
      }else if (caractere[Ind+conta_l][msg]==14){
         for (x=0;x<8;x++){
            C1[Ind][x]=K[x];
         }
      }else if (caractere[Ind+conta_l][msg]==15){
         for (x=0;x<8;x++){
            C1[Ind][x]=L[x];
         }
      }else if (caractere[Ind+conta_l][msg]==16){
         for (x=0;x<8;x++){
            C1[Ind][x]=QUATRO[x];
         }
      }else if (caractere[Ind+conta_l][msg]==17){
         for (x=0;x<8;x++){
            C1[Ind][x]=M[x];
         }
      }else if (caractere[Ind+conta_l][msg]==18){
         for (x=0;x<8;x++){
            C1[Ind][x]=N[x];
         }
      }else if (caractere[Ind+conta_l][msg]==19){
         for (x=0;x<8;x++){
            C1[Ind][x]=O[x];
         }
      }else if (caractere[Ind+conta_l][msg]==20){
         for (x=0;x<8;x++){
            C1[Ind][x]=CINCO[x];
         }
      }else if (caractere[Ind+conta_l][msg]==21){
         for (x=0;x<8;x++){
            C1[Ind][x]=P[x];
         }
      }else if (caractere[Ind+conta_l][msg]==22){
         for (x=0;x<8;x++){
            C1[Ind][x]=Q[x];
         }
      }else if (caractere[Ind+conta_l][msg]==23){
         for (x=0;x<8;x++){
            C1[Ind][x]=R[x];
         }
      }else if (caractere[Ind+conta_l][msg]==24){
         for (x=0;x<8;x++){
            C1[Ind][x]=SEIS[x];
         }
      }else if (caractere[Ind+conta_l][msg]==25){
         for (x=0;x<8;x++){
            C1[Ind][x]=S[x];
         }
      }else if (caractere[Ind+conta_l][msg]==26){
         for (x=0;x<8;x++){
            C1[Ind][x]=T[x];
         }
      }else if (caractere[Ind+conta_l][msg]==27){
         for (x=0;x<8;x++){
            C1[Ind][x]=U[x];
         }
      }else if (caractere[Ind+conta_l][msg]==28){
         for (x=0;x<8;x++){
            C1[Ind][x]=SETE[x];
         }
      }else if (caractere[Ind+conta_l][msg]==29){
         for (x=0;x<8;x++){
            C1[Ind][x]=V[x];
         }
      }else if (caractere[Ind+conta_l][msg]==30){
         for (x=0;x<8;x++){
            C1[Ind][x]=W[x];
         }
      }else if (caractere[Ind+conta_l][msg]==31){
         for (x=0;x<8;x++){
            C1[Ind][x]=X1[x];
         }
      }else if (caractere[Ind+conta_l][msg]==32){
         for (x=0;x<8;x++){
            C1[Ind][x]=OITO[x];
         }
      }else if (caractere[Ind+conta_l][msg]==33){
         for (x=0;x<8;x++){
            C1[Ind][x]=Y[x];
         }
      }else if (caractere[Ind+conta_l][msg]==34){
         for (x=0;x<8;x++){
            C1[Ind][x]=Z[x];
         }
      }else if (caractere[Ind+conta_l][msg]==35){
         for (x=0;x<8;x++){
            C1[Ind][x]=NOVE[x];
         }
      }else if (caractere[Ind+conta_l][msg]==36){
         for (x=0;x<8;x++){
            C1[Ind][x]=zero[x];
         }
      }else if (caractere[Ind+conta_l][msg]==37){
         for (x=0;x<8;x++){
            C1[Ind][x]=branco[x];
         }
      }else if (caractere[Ind+conta_l][msg]==38){
         for (x=0;x<8;x++){
            C1[Ind][x]=ponto[x];
         }
      }else if (caractere[Ind+conta_l][msg]==39){
         for (x=0;x<8;x++){
            C1[Ind][x]=doispontos[x];
         }
      }else if (caractere[Ind+conta_l][msg]==40){
         for (x=0;x<8;x++){
            C1[Ind][x]=travessao[x];
         }
      }
}

void identifica_temperatura(unsigned int8 Ind,unsigned int8 C1[8][8]){
     int x;
     if (temperatura[Ind]==0){
        for (x=0;x<8;x++){
            C1[Ind][x]=branco[x];
        }
     }else if (temperatura[Ind]==1){
        for (x=0;x<8;x++){
            C1[Ind][x]=zero[x];
        }
     }else if (temperatura[Ind]==2){
        for (x=0;x<8;x++){
            C1[Ind][x]=um[x];
        }
     }else if (temperatura[Ind]==3){
        for (x=0;x<8;x++){
            C1[Ind][x]=dois[x];
        }
     }else if (temperatura[Ind]==4){
        for (x=0;x<8;x++){
            C1[Ind][x]=tres[x];
        }
     }else if (temperatura[Ind]==5){
        for (x=0;x<8;x++){
            C1[Ind][x]=quatro[x];
        }
     }else if (temperatura[Ind]==6){
        for (x=0;x<8;x++){
            C1[Ind][x]=cinco[x];
        }
     }else if (temperatura[Ind]==7){
        for (x=0;x<8;x++){
            C1[Ind][x]=seis[x];
        }
     }else if (temperatura[Ind]==8){
        for (x=0;x<8;x++){
            C1[Ind][x]=sete[x];
        }
     }else if (temperatura[Ind]==9){
        for (x=0;x<8;x++){
            C1[Ind][x]=oito[x];
        }
     }else if (temperatura[Ind]==10){
        for (x=0;x<8;x++){
            C1[Ind][x]=nove[x];
        }
     }else if (temperatura[Ind]==255){ //C
        for (x=0;x<8;x++){
         C1[Ind][x]=C[x];
        }
     }else if (temperatura[Ind]==254){ //°
        for (x=0;x<8;x++){
         C1[Ind][x]=grau[x];
        }
     }else if (temperatura[Ind]==253){ //virgula
        for (x=0;x<8;x++){
         C1[Ind][x]=virgula[x];
        }
     }else if (temperatura[Ind]==252){ //virgula
        for (x=0;x<8;x++){
         C1[Ind][x]=travessao[x];
        }
     }
}

void desloca_4094(unsigned int8 dado, unsigned int8 x,unsigned int8 y){
     while(x>y){
         if(bit_test(dado,y)==0){
            output_low(data);
         }else{
            output_high(data);
         }
         output_high(clk);
         delay_us(10);
         output_low(clk);
         y++;
      }
}

void le_teclado(void){
   output_low(col1);
   output_high(c2);
   output_high(c3);
   output_high(c4);
   delay_ms(2);
   if(input(lin1)==0){
      if ((caractere[qual][val]==1)||(caractere[qual][val]==2)||(caractere[qual][val]==3)) caractere[qual][val]++;
      else caractere[qual][val]=1;
      while (input(lin1)==0);
      return;
   }else if(input(lin2)==0){
      if ((caractere[qual][val]==17)||(caractere[qual][val]==18)||(caractere[qual][val]==19)) caractere[qual][val]++;
      else caractere[qual][val]=17;
      while (input(lin2)==0);
      return;
   }else if(Input(lin3)==0){
      if ((caractere[qual][val]==33)||(caractere[qual][val]==34)||(caractere[qual][val]==35)) caractere[qual][val]++;
      else caractere[qual][val]=33;
      while (input(lin3)==0);
      return;
   }else if(Input(lin4)==0){
         val++;
         if (val==5){   //6 msgs
            val=0;
         }
         if (contamsgs<6){ //6msgs
            contamsgs++;
         }
         qual=0;
      while(input(lin4)==0);
      return;
   }
   
   output_high(col1);
   output_low(c2);
   output_high(c3);
   output_high(c4);
   delay_ms(2);
   if(input(lin1)==0){
      if ((caractere[qual][val]==5)||(caractere[qual][val]==6)||(caractere[qual][val]==7)) caractere[qual][val]++;
      else caractere[qual][val]=5;
      while (input(lin1)==0);
      return;
   }else if(input(lin2)==0){
      if ((caractere[qual][val]==21)||(caractere[qual][val]==22)||(caractere[qual][val]==23)) caractere[qual][val]++;
      else caractere[qual][val]=21;
      while (input(lin2)==0);
      return;
   }else if(input(lin3)==0){
      if ((caractere[qual][val]==37)||(caractere[qual][val]==38)||(caractere[qual][val]==39)) caractere[qual][val]++;
      else caractere[qual][val]=37;
      while (input(lin3)==0);
      return;
   }else if(input(lin4)==0){
         qual++;
         if (qual>=6){
         conta_l++;
         }
         if (qual==14){
            qual=0;
         }
      while (input(lin4)==0);
      return;
   }
   output_high(col1);
   output_high(c2);
   output_low(c3);
   output_high(c4);
   delay_ms(2);
   if(input(lin1)==0){
      if ((caractere[qual][val]==9)||(caractere[qual][val]==10)||(caractere[qual][val]==11)) caractere[qual][val]++;
      else caractere[qual][val]=9;
      while (input(lin1)==0);
      return;
   }else if(input(lin2)==0){
      if ((caractere[qual][val]==25)||(caractere[qual][val]==26)||(caractere[qual][val]==27)) caractere[qual][val]++;
      else caractere[qual][val]=25;
      while (input(lin2)==0);
      return;
   }else if(input(lin3)==0){
      return;
   }else if(input(lin4)==0){
      unsigned int8 ind;
      for (ind=0;ind<39;ind++){
            caractere[ind][val]=0; //limpa
      }
      qual=0;
      contamsgs=0;
      while(input(lin3)==0);
      return;
   }
   output_high(col1);
   output_high(c2);
   output_high(c3);
   output_low(c4);
   delay_ms(2);
   if(input(lin1)==0){
      if ((caractere[qual][val]==13)||(caractere[qual][val]==14)||(caractere[qual][val]==15)) caractere[qual][val]++;
      else caractere[qual][val]=13;
      while (input(lin1)==0);
      return;
   }else if(input(lin2)==0){
      if ((caractere[qual][val]==29)||(caractere[qual][val]==30)||(caractere[qual][val]==31)) caractere[qual][val]++;
      else caractere[qual][val]=29;
      while (input(lin2)==0);
      return;
   }else if(input(lin3)==0){
      caractere[qual][val]=0;
      if (qual>0){
         qual--;
         caractere[qual][val]=0;
      }
      while(input(lin3)==0);
      return;
   }
}

void escreve_le_temp()
{
   output_high(SENSOR_SELECT);//habilita sensor
   delay_us(50);
   spi_write(SENSOR_ENDE_LER);//endereço de leitura
   MSB = spi_read(0X02);//primeiro byte - MSB temperatura (veja que este valor enviado é só p /        pegar o dado do sensor(pode ser qualquer valor))
   LSB = spi_read(0X01);//segundo byte - LSB temperatura
   
   switch (LSB){  //escolhe 2ª casa
      case 64:
         if (MSB>=0)
         LSB=2;
         else LSB=7;
         break;
      case -128:
         LSB=4;
         break;
      case -64:
         if (MSB>=0)
         LSB=7;
         else LSB=2;
         break;
      case 0:
         if (MSB<0)
         MSB--;
      default:
         LSB=0;
         break;
   }
   control_byte = spi_read(0X03);//terceiro byte - valor do byte de controle
   output_low(SENSOR_SELECT);//desabilita sensor
}


void revela_temp(void){
   escreve_le_temp();
   if (MSB>=0){
   temperatura[2]=MSB+1;
   temperatura[1]=0;
   if (MSB>9){
      temperatura[1]=(MSB/10)+1;
      temperatura[2]=(MSB-((MSB/10)*10))+1;
   }
   temperatura[4]=LSB+1;
   }else{
      temperatura[1]=252; //negativo
      MSB=abs(MSB);
      temperatura[2]=MSB;
      temperatura[4]=LSB+1;
   }
}
