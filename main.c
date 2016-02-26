//#include "io430.h"
#include  "msp430x552x.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

char * dictionary[512];
char * buffer;

int index_count = 0;

char * s;
int s_length = 0;

int pw;
int flag = 0x01;
uint8_t go = 0;

void initDict() {
  char * temp;
  //char c = 0;
  for(int i = 0; i < 26; i++) {
    temp = calloc(1,sizeof(char));
    temp[0] = i+'a';
    dictionary[i] = temp;
    //printf("%s",dictionary[i]);
  }
  index_count = 26;
  s = calloc(10,sizeof(char));
  s_length = 0;
  pw = 512;
}

void free_dict(int start, int end) {
  for(int i=start; i <= end; i++) {
   free(dictionary[i]); 
  }
}

int equal(char * s1, char * s2,int length) {
  for(int i = 0; i < length ; i++) {
    if(s1[i]!=s2[i])
      return 0;
  }
  return 1;
}

void copystring(char * s1, char *s2, int length) {
  for(int i = 0; i < length; i++) {
    if(s2[i]!='\0') {
      s1[i] = s2[i];
    } else {
      break;
    }
  }
}

void string_cat(char *s1, char c, int length) {
  s1[length-1] = c; 
}
void lzw(char c) {

  UCA1IE &= ~UCRXIE;             // Enable USCI_A0 RX interrupt
  //__bic_SR_register(GIE);
  
  unsigned char sendres = '\0';
  // is the string s+c in the dictionary?
  int exists = 0;
  char code;
  for ( int i = 0; i < index_count;i++) {
    if(equal(s,dictionary[i],s_length)==1){
      code = i;
      break;
    }
  }
  if ( c == '.' ) {
    UCA1TXBUF = (code+1)+'0';
    while(UCA1STAT & UCBUSY);
    go = 1;
    return ;
  }
  *(s+s_length) = c;
  s_length++;
  //printf("%s\n",s);
  for ( int i = 0;i<index_count;i++) {
    if(equal(s,dictionary[i],s_length)==1) {
      exists = 1;
    }
  }
  // it is, s = s+c
  if ( exists == 0) {
    //printf("not exists\n");
    //printf("%d \n",code+1);
    dictionary[index_count] = calloc(s_length,sizeof(char));
    copystring(dictionary[index_count],s,s_length);
    //printf("\n%s\n",dictionary[index_count]);
    index_count++;
    memcpy(s,'\0',10);
    *s = c;
    s_length = 1;
    sendres = code+1;
  } else {
    //printf("exists\n");
  }
  // not
  //output the code denote P to the code stream
  
  if(sendres!='\0') {
    UCA1TXBUF = (int)sendres+'0';
    while(UCA1STAT & UCBUSY);
  }
  UCA1IE |= UCRXIE;
  //__bis_SR_register(GIE);
}

void decompress(char rec) {
  int code = rec - '0';
  code--;
  int exist = 0;
  // cw = fist code word
  // output the string.cw to charstream
  size_t size;
  if (pw!=512) {
    size = strlen(dictionary[pw]);
  } else {
    size = 0;
  }
  size_t sizec = strlen(dictionary[code]);
  
  //printf("current : %d , pre : %d",sizec,size);
  
  char *p = calloc(size+1,sizeof(char));
  memcpy(p,'\0',size+1);
  //first word
  if(pw!=512) copystring(p,dictionary[pw],size);
  //printf("\n%s\n",p);
  
  int trans = 0;
  //printf("\n%s\n",dictionary[code]);
  while(trans<sizec) {
    if((*(dictionary[code]+trans)-'a')<26) {
      UCA1TXBUF = *(dictionary[code]+trans);
      while(UCA1STAT & UCBUSY);
    }
      trans++;
  }
  string_cat(p,*dictionary[code],size+1);
  for(int i=0;i<index_count;i++) {
      if(equal(p,dictionary[i],size+1)==1) {
        exist = 1;
        break;
      }
  }
  if ( exist == 0) {
      dictionary[index_count] = calloc(size+1,sizeof(char));
      memcpy(dictionary[index_count],'\0',size+1);
      copystring(dictionary[index_count],p,size+1);
      //printf("\n%s\n",p);
      //printf("\n%s\n",dictionary[index_count]);
      index_count++;
  }
  // cw = next code word
  pw = code;
  // is the string.cw present in the dictionary
  memcpy(p,'\0',size);
  free(p);
  p = NULL;
}
// Echo back RXed character, confirm TX buffer is ready first

void main(void)
{
  //WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  WDTCTL = WDTPW + WDTCNTCL;
  P2DIR = 0x00;
  P2SEL = 0x00;
  P2REN = 0x02;
  P2OUT = 0x02;
  P2IES = 0x02;
  P2IE = 0x02;
  
  P1DIR &= ~0x02;
  P1REN |= 0x02;
  P1OUT |= 0x02;
  P1IES |= 0x02;
  P1IE |= 0x02;
  
  P4DIR = 0x80;
  P4OUT = 0x00;
  flag = 0x01;
  
  initDict();
  P4SEL = BIT4+BIT5;                        // P3.3,4 = USCI_A0 TXD/RXD
  UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
  UCA1CTL1 |=  UCSSEL0+UCSSEL1;                     // CLK = SMCLK
  
//===========set the baud-rate register====
  UCA1BR0 =109 ;//???????????????????????//
  UCA1BR1 = 0 ;//???????????????????????//
  UCA1MCTL = UCBRF0 + UCBRS2;//???????????????????????//
//=========================================  
  
  DMACTL0 = DMA0TSEL_0;               // USCIA0 RX trigger
  UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
  
  __bis_SR_register(GIE);       // Enter LPM3, interrupts enabled
  
  while(go == 0);
  
  __bic_SR_register(GIE);
  /*
  for(int i=0;i<count;i++) {
    UCA1TXBUF = buffer[i];
    while(UCA1STAT & UCBUSY);
    buffer[i] = 0;
  }*/
  //__no_operation();                         // For debugger
  for(int i=0;i<index_count;i++) {
    free(dictionary[i]);
    dictionary[i] = NULL;
  }
  WDTCTL = 0;
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
  switch(__even_in_range(UCA1IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG
    while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    char receive;
    receive = UCA1RXBUF;
    //printf("%c",receive);
    if (flag&0x01) {
      lzw(receive);
    } else {
      decompress(receive);
    }
    //UCA1TXBUF = receive;
    //printf("%d ",numb);
    
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;  
  }
}

#pragma vector=PORT2_VECTOR
__interrupt void P2_ISR(void) {
  
  P2IE = 0x00;
  UCA1IE &= ~UCRXIE;             // Enable USCI_A0 RX interrupt
  __bic_SR_register(GIE);
  go = 1;
  __delay_cycles(1000);
  P2IFG = 0x00;
  P2IE = 0x02;
}

#pragma vector=PORT1_VECTOR
__interrupt void P1_ISR(void) {
  P1IE = 0x00;
  flag = 0x00;
  P4OUT ^= 0x80;
  P1IFG = 0x00;
  P1IE = 0x02;
}
