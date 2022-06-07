/* ����ͷ�ļ� */
#include <ioCC2530.h>
#include "hal_defs.h"

/*�궨��*/
#define LED1 P1_0 
#define LED2 P1_1
#define SW1 P1_2

/*�������*/
uint8 counter = 0; //ͳ�ƶ�ʱ���������
uint8 buff[2];
uint8 flag = 0;
uint16 value = 0;

/*��������*/
void Delay1Ms(uint8 time);//��ʱ������32MHzϵͳʱ���£�Լ1ms��ʱ����
void InitCLK(void);//ϵͳʱ�ӳ�ʼ��������Ϊ32MHz
void InitLED(void);//LED��IO�˿ڳ�ʼ������
void InitTime1(void);//��ʱ��1��ʼ�������������������Ϊ50ms
void InitADC(void);//ADCģ���ʼ��
uint16 get_adc(void);//��ȡADCת����������طֱ���Ϊ0.01V�ĵ�ѹֵ
void InitUart0(void);//����0��ʼ������
void UART0SendByte(unsigned char c);//UART0����һ���ֽں���
void UART0SendString(unsigned char *str);//UART0���������ַ���
void UART0SendData(unsigned char *str,int len);//UART0����ָ�������ֽ�����
/*���庯��*/
void Delay1Ms(uint8 time)
{
  unsigned int i,j;
  for(i=0;i<=time;i++)
  {
    for(j=0;j<1100;j++);
  }
}

void InitCLK(void)
{
  CLKCONCMD &= 0x80;
  while(CLKCONSTA & 0x40);
}

void InitLED(void)
{
  P1SEL &= ~0x07;//����P1_0��P1_1��P1_2ΪGPIO��
  P1DIR |= 0x03;//����P1_0��P1_1Ϊ���
}

void InitTime1(void)
{
  T1CC0L = 50000 & 0xff;
  T1CC0H = (50000 &0xff00)>>8;
  T1CCTL0 |= 0x04;//�趨��ʱ��1ͨ��0�Ƚ�ģʽ
  T1CTL = 0x0a;//���ö�ʱ��1Ϊ32��Ƶ��ģģʽ������ʼ����
  TIMIF &= ~0x40;//��������ʱ��1������ж�
  T1IE = 1;//ʹ�ܶ�ʱ��1�ж� 
}

void InitADC(void)
{
  APCFG  |=0x01;
  P0SEL  |=0x01;	
  P0DIR  &=~0x01;	
}

uint16 get_adc(void)
{
  uint32 value;
  ADCIF = 0;   //��ADC �жϱ�־
  //���û�׼��ѹavdd5:3.3V��ͨ��0������ADת��
  ADCCON3 = (0x80 | 0x10 | 0x00);
  while (!ADCIF); //�ȴ�ADת������
  value = ADCH;
  value = value<< 8;
  value |= ADCL;
  // ADֵת���ɵ�ѹֵ
  // 0 ��ʾ 0V ��32768 ��ʾ 3.3V
  // ��ѹֵ = (value*3.3)/32768 ��V)
  value = (value * 330);
  value = value >> 15;   // ����32768
  // ���طֱ���Ϊ0.01V�ĵ�ѹֵ
  return (uint16)value;
}

void InitUart0(void)
{
  PERCFG = 0x00;	
  P0SEL = 0x3c;	
  U0CSR |= 0x80;
  U0BAUD = 216;
  U0GCR = 11;
  U0UCR |= 0x80;
  UTX0IF = 0;  // ����UART0 TX�жϱ�־ 
}

void UART0SendByte(unsigned char c)
{
  U0DBUF = c;// ��Ҫ���͵�1�ֽ�����д��U0DBUF
  while (!UTX0IF) ;// �ȴ�TX�жϱ�־����U0DBUF����
  UTX0IF = 0;// ����TX�жϱ�־
}

void UART0SendString(unsigned char *str)
{
  while(*str != '\0')
  {
    UART0SendByte(*str++);
  }
}

void UART0SendData(unsigned char *str,int len)
{
  for(int i=0;i<len;i++)
  {
    U0DBUF = str[i];		// ��Ҫ���͵�1�ֽ�����д��U0DBUF
    while (!UTX0IF) ;  // �ȴ�TX�жϱ�־����U0DBUF����
    UTX0IF = 0;       // ����TX�жϱ�־UART0SendByte(*str++);   // ����һ�ֽ�
  }
}

/*������*/
void main(void)
{
  InitCLK();
  InitLED();
  InitTime1();
  InitADC();
  InitUart0();
  
  /*.......������1��ʼ��LED�Ƴ�ʼ״̬����...........*/
  LED1 = LED2 = 0;//����LED1��LED2�ĳ�ʼ״̬
  /*.......������1����...........*/
  
  /*.......������2��ʼ��SW1�����ж����빦�ܳ�ʼ��...........*/
  P1DIR &= ~0x04;//����P1_2Ϊ����
  P1INP &= ~0x04;//����P1_2�˿�Ϊ������/������ģʽ
  P2INP &= ~0x40;//��������P1�˿�Ϊ��������
  PICTL |= 0x02;//����P1_2�˿��жϴ�����ʽΪ���½��ش���
  IEN2 |= 0x10;//ʹ��P1�˿��ж�
  P1IEN |= 0x04;//ʹ��P1_2�˿��ж�
  /*.......������2����...........*/
  
  EA = 1;//ʹ�����ж�
  
  while(1)
  {
    /*..������3��ʼ��ÿ��2s�ɼ����ն����ݲ�ͨ�����ڷ��ͣ�ÿ�βɼ�LED2��˸..*/
    if(counter >= 40)
    {
      counter = 0;
      if(flag == 1)
      {
        value = get_adc();
        buff[0] = value>>8;
        buff[1] = value;
        UART0SendData(buff,2);
        LED2 = 1;
        Delay1Ms(200);
        LED2 = 0;
      }
    }
    /*.......������3����...........*/
  }
}

/*�жϷ�����*/
/*.......������4��ʼ�������жϷ�����...........*/
#pragma vector = P1INT_VECTOR
__interrupt void P1_ISR(void)
{
  if(P1IF == 1)
  {
    if(P1IFG & 0x04)
    {
      if(flag == 0)
      {
        flag = 1;
        LED1 = 1;
      }
      else
      {
        flag = 0;
        LED1 = 0;
      }
      P1IFG &= ~0x04;
    }
    P1IF = 0;
  }
}
/*.......������4����...........*/

#pragma vector = T1_VECTOR
__interrupt void T1_ISR(void)
{
  counter++;
  T1STAT &= ~0x01;  //���ͨ��0�жϱ�־
}