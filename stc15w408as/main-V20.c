
/*---------------------------------------------------------------------*/
/* --- STC MCU International Limited ----------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: 13922805190 --------------------------------------------*/
/* --- Fax: 0513-55012956,55012947,55012969 ---------------------------*/
/* --- Tel: 0513-55012928,55012929,55012966 ---------------------------*/
/* --- Web: www.GXWMCU.com   www.stcmcu.com ---------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* ���Ҫ�ڳ�����ʹ�ô˴���,���ڳ�����ע��ʹ���˺꾧�Ƽ������ϼ�����   */
/*---------------------------------------------------------------------*/


/*************	����˵��	**************

����������ʹ��STC15W401AS-35I-SOP16<RMB1.6>��������ģ�õ��޴�������ˢ����ֱ�����.

������ο������ϵĴ���(����: ����), ��������.

��·ͼ���ļ� "BLDC-V10-ʵ���·.pdf".

�����ź���P3.2�����������ź�, ���5~20ms, ������1.000~1.610ms.

1.160ms��ʼ����, 1.610msΪ����ٶ�, �ֱ���Ϊ2us.

����������Ǽ򵥿���, ���û�д��� ��0��ʱ30���л� �������.

���ڹ�0��ⲿ����RC�˲�, ���Ըı����ֵ���Դ�Լ�Ķ�Ӧ�������ʱ��ʱ30�ȵ�ʱ��.

�����߿��������Ƶ�·�ͳ���.

******************************************/




#define MAIN_Fosc		24000000L	//������ʱ��

#include "STC15Fxxxx.H"

#define		MCU_PIN		16	/* ѡ��MCU������, ֻ֧��16��20��(28�Ż�32�Ÿ�20��һ��) */

// #include "reg51.h"
#include "intrins.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;
void SendData(BYTE dat);
void SendString(char *s);
// #define FOSC 11059200L          //ϵͳƵ��
// #define BAUD 115200             //���ڲ�����

#define NONE_PARITY     0       //��У��
#define ODD_PARITY      1       //��У��
#define EVEN_PARITY     2       //żУ��
#define MARK_PARITY     3       //���У��
#define SPACE_PARITY    4       //�հ�У��

#define PARITYBIT NONE_PARITY   //����У��λ



//CMPCR1
#define	CMPEN	0x80	//1: ����Ƚ���, 0: ��ֹ,�رձȽ�����Դ
#define	CMPIF	0x40	//�Ƚ����жϱ�־, ���������ػ��½����ж�, �����0
#define	PIE		0x20	//1: �ȽϽ����0��1, �����������ж�
#define	NIE		0x10	//1: �ȽϽ����1��0, �����½����ж�
#define	PIS		0x08	//����������ѡ��, 0: ѡ���ⲿP5.5��������,           1: ��ADCIS[2:0]��ѡ���ADC�������������.
#define	NIS		0x04	//���븺����ѡ��, 0: ѡ���ڲ�BandGap��ѹBGv��������, 1: ѡ���ⲿP5.4������.
#define	CMPOE	0x02	//1: ����ȽϽ�������P1.2, 0: ��ֹ.
#define	CMPRES	0x01	//�ȽϽ��, 1: CMP+��ƽ����CMP-,  0: CMP+��ƽ����CMP-,  ֻ��

//CMPCR2
#define	INVCMPO	0x80	//1: �Ƚ������ȡ��,  0: ��ȡ��
#define	DISFLT	0x40	//1: �ر�0.1uF�˲�,   0: ����
#define	LCDTY	0x00	//0~63, �ȽϽ���仯��ʱ������

#if	(MCU_PIN == 20)
	sbit PWM2_L = P3^4;
	sbit PWM1_L = P3^5;
	sbit PWM0_L = P3^6;
#endif

#if	(MCU_PIN == 16)
	sbit PWM2_L = P5^5;		// C-
	sbit PWM1_L = P3^3;		// B-
	sbit PWM0_L = P3^6;		// A-
#endif

u8	Step;
u8	PWM_Value; // ����PWMռ�ձȵ�ֵ
u16	RxPulseWide;
bit	B_RxOk;
bit	B_RUN;
u8	PWW_Set;
u8	cnt10ms;
u8	Rx_cnt;
u8	TimeOut;	//��ת��ʱ

#define DISABLE_CMP_INT CMPCR1 &= ~0X40		// �رձȽ����ж�
#define ENABLE_CMP_INT  CMPCR1 |= 0X40		// �򿪱Ƚ����ж�

/*************************/

void	Delay_n_ms(u8 dly)
{
	u16	j;
	do
	{
		j = MAIN_Fosc / 13000;	//��ʱ1ms, �������ڴ˽���������
		while(--j)	;
	}while(--dly);
}


void delay_us(u8 us)
{
	do
	{
		NOP(20);	//@24MHz
	}
	while(--us);
}

void StepXL(void) // �������к���
{
 switch(Step)
  {
   case 0:  // AB
			PWM0_L=0;	PWM2_L=0;
			PWM0_OUT_1();
			PWM1_OUT_1();
			PWM2_OUT_1();
			PWM0_NORMAL();
			CCAP0H = PWM_Value;		// ��A��ĸ߶�
			PWM1_L = 1; 		// ��B��ĵͶ�
			ADC_CONTR = 0XED;	// ѡ��P1.5��ΪADC���� ��c���ѹ
			CMPCR1 = 0x9C;		//bit7=1 ����Ƚ���, bit4=1 �ȽϽ����1��0, �����½����ж� (������Ӧ�½����ж�?)
		break;
   case 1:  // AC
			PWM0_L=0;	PWM1_L=0;
			PWM0_OUT_1();
			PWM1_OUT_1();
			PWM2_OUT_1();
			PWM0_NORMAL();
			CCAP0H = PWM_Value;		// ��A��ĸ߶�
			PWM2_L = 1;			// ��C��ĵͶ�
			ADC_CONTR = 0XEC;	// ѡ��P1.4��ΪADC���� ��B���ѹ
			CMPCR1 = 0xAC;		//�������ж�
	 
      	break;
   case 2:  // BC
			PWM0_L=0;	PWM1_L=0;
			PWM0_OUT_1();
			PWM1_OUT_1();
			PWM2_OUT_1();
			PWM1_NORMAL();
			CCAP1H = PWM_Value; // ��B��ĸ߶�
			PWM2_L = 1;			// ��C��ĵͶ�
			ADC_CONTR = 0XEB;	// ѡ��P1.3��ΪADC���� ��a���ѹ
			CMPCR1 = 0x9C;		//�½����ж�
      	break;
   case 3:  // BA
			PWM1_L=0;	PWM2_L=0;
			PWM0_OUT_1();
			PWM1_OUT_1();
			PWM2_OUT_1();
			PWM1_NORMAL();
			CCAP1H = PWM_Value; // ��B��ĸ߶�
			PWM0_L = 1;			// ��A��ĵͶ�
			ADC_CONTR = 0XED;	// ѡ��P1.5��ΪADC���� ��c���ѹ 
			CMPCR1 = 0xAC;		//�������ж�
			
      	break;
   case 4: // CA
			PWM1_L=0;	PWM2_L=0;
			PWM0_OUT_1();
			PWM1_OUT_1();
			PWM2_OUT_1();
			PWM2_NORMAL();
			CCAP2H = PWM_Value; // ��C��ĸ߶�
			PWM0_L = 1;			// ��A��ĵͶ�
			ADC_CONTR = 0XEC;	// ѡ��P1.4��ΪADC���� ��B���ѹ
			CMPCR1 = 0x9C;		//�½����ж�
     	break;
   case 5: // CB
      		PWM0_L=0;	PWM2_L=0;
			PWM0_OUT_1();
			PWM1_OUT_1();
			PWM2_OUT_1();
			PWM2_NORMAL();
			CCAP2H = PWM_Value; // ��C��ĸ߶�
      		PWM1_L = 1;			// ��B��ĵͶ�
			ADC_CONTR = 0XEB;	// ѡ��P1.3��ΪADC���� ��a���ѹ
			CMPCR1 = 0xAC;		//�������ж�
	 
		break;
	 
	default:
		break;
  }	
}



void PWM_Init(void)
{
	PWM0_L = 0;
	PWM1_L = 0;
	PWM2_L = 0;
	
	#if	(MCU_PIN == 20)
		P3n_push_pull(0x70);
	#endif
	#if	(MCU_PIN == 16)
		P3n_push_pull(0x48);
		P5n_push_pull(0x20);
	#endif

	// CMOD = 1 << 1; //ѡ��ϵͳʱ��/2Ϊʱ��Դ����PWMƵ��=24M/2/256=46.9K
	CMOD = 5 << 1; //ѡ��ϵͳʱ��/4Ϊʱ��Դ����PWMƵ��=24M/4/256=23.4K
	// CMOD = 6 << 1; //ѡ��ϵͳʱ��/6Ϊʱ��Դ����PWMƵ��=24M/6/256=15.6K
	CL=0;			// PCA����������
	CH=0;
	
	PCA_PWM0 = 0X00;
	CCAP0H=0;    // ��ʼ��ռ�ձ�Ϊ0% H��ֵװ�ص�L��
	CCAP0L=0;
	CCAPM0=0x42;	// ����ΪPWMģʽ, 8λ�����жϡ�

	PCA_PWM1 = 0X00;
	CCAP1H=0;    // ��ʼ��ռ�ձ�Ϊ0%
	CCAP1L=0;
	CCAPM1=0x42;	// ����ΪPWMģʽ
	
	PCA_PWM2 = 0X00;
	CCAP2H=0;    // ��ʼ��ռ�ձ�Ϊ0%
	CCAP2L=0;
	CCAPM2=0x42;	// ����ΪPWMģʽ
	
	PWM0_OUT_1();
	PWM1_OUT_1();
	PWM2_OUT_1();

	CR = 1;
}

void ADC_Init(void)
{
	P1n_pure_input(0x38);
	P1ASF = 0X38; // ��ͨP1.3 P1.4 P1.5��AD�����
}

void CMP_INT(void) interrupt 21
{
	CMPCR1 &= ~0X40; // ���������жϱ�־λ
	if(Step<5)	Step++;
	else		Step = 0;
	StepXL();
	TimeOut = 10;	//10ms��ʱ
}

void CMP_Init(void)
{
	CMPCR1 = 0X8C;	// 1000 1100 �򿪱Ƚ�����P5.4��Ϊ�Ƚ����ķ�������ˣ�ADC������Ϊ������� 
	CMPCR2 = 60;	// 60��ʱ���˲�
	P5n_pure_input(0x10);
}

u8 StartMotor(void)
{
	u16 timer,i;
	DISABLE_CMP_INT;	// ��ֹ�Ƚ����ж�
	PWM_Value = 30;		// ��ʼռ�ձ�=16/256=6%
	Step = 0;
	StepXL();			// ��ʼλ��
	Delay_n_ms(5);//delay_ms(5);
	timer = 300;

	while(1)
	{
		for(i=0; i<timer; i++)	delay_us(50);  // ��һ����15���룬���������5������ʱ���ܼƴ�Լ20����
		timer -= timer /15 + 1;
		if(timer < 25)	{
			CMPCR1 &= ~0X40; 	// ���������жϱ�־λ
			B_RUN = 1;			// ������б�־λ
			PWM_Value = 0;		// ������ɡ�ͣת���ȴ��ڵ�PWW_SETֵ
			TimeOut = 10;		// 10���׳�ʱ
			ENABLE_CMP_INT; 	// �򿪱Ƚ����ж�
			return(1);
		}
		if( Step < 5)	Step++;
		else			Step = 0;
		StepXL();
		// SendString("Start Motor!\r\n");
	}
}

// void T0_Iint(void)
// {
// 	Timer0_AsTimer();	/* ʱ��0������ʱ��	*/
// 	Timer0_12T();		/* Timer0 clodk = fo/12	12��Ƶ,	default	*/
// 	Timer0_16bit();
// 	Timer0_Gate_INT0_P32();	/* ʱ��0���ⲿINT0�ߵ�ƽ����ʱ���� */
// 	TH0 = 0;
// 	TL0 = 0;
// 	TR0 = 1; // �򿪶�ʱ��0
// 	ET0 = 1;// ����ET0�ж�
// }

// void T0_Interrupt(void) interrupt 1
// {
// 	Rx_cnt = 0;			//һ���������, ��ʼ��n��������Ч
// 	RxPulseWide = 1000;	//ֹͣ
// 	B_RxOk = 1;			//�����յ�һ������
// }

/********************* INT0�жϺ��� *************************/
// void INT0_int (void) interrupt INT0_VECTOR
// {
// 	u16	j;
	
// 	TR0 = 0;
// 	j = ((u16)TH0 << 8) + TL0;
// 	TH0 = 0;
// 	TL0 = 0;
// 	TR0 = 1;

// 	if(++Rx_cnt >= 5)	Rx_cnt = 5;
// 	j >>= 1;	//Ϊ�˺ô���, ת�ɵ�λΪus
// 	if((j >= 800) && (j <= 2000) && (Rx_cnt == 5))
// 	{
// 		RxPulseWide = j;
// 		B_RxOk = 1;		//��־�յ�һ������
// 	}

// }

/********************* ���ڳ�ʼ������ *************************/
bit busy;


void UartInit(void)		//9600bps@24.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x01;		//����1ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR |= 0x04;		//��ʱ��2ʱ��ΪFosc,��1T
	T2L = 0x8F;		//�趨��ʱ��ֵ
	T2H = 0xFD;		//�趨��ʱ��ֵ
	AUXR |= 0x10;		//������ʱ��2
    ES = 1;                     //ʹ�ܴ���1�ж�
}


/*----------------------------
UART �жϷ������
-----------------------------*/
void Uart() interrupt 4 using 1
{
    ES = 0;         //�رմ����ж�
	if (RI)
    {
		PWW_Set = SBUF; // ���յ������ݸ���PWW_Set
		RI=0; // ���ڽ��ձ�־��0
		B_RxOk = 1;
    }
    if (TI)
    {
        TI = 0;                 //���TIλ
        busy = 0;               //��æ��־
    }
	ES = 1;         //ʹ�ܴ����ж�
}


/*----------------------------
���ʹ�������
----------------------------*/
void SendData(BYTE dat)
{
    while (busy);               //�ȴ�ǰ������ݷ������
    ACC = dat;                  //��ȡУ��λP (PSW.0)
    if (P)                      //����P������У��λ
    {
		#if (PARITYBIT == ODD_PARITY)
				TB8 = 0;                //����У��λΪ0
		#elif (PARITYBIT == EVEN_PARITY)
				TB8 = 1;                //����У��λΪ1
		#endif
    }
    else
    {
		#if (PARITYBIT == ODD_PARITY)
				TB8 = 1;                //����У��λΪ1
		#elif (PARITYBIT == EVEN_PARITY)
				TB8 = 0;                //����У��λΪ0
		#endif
    }
    busy = 1;
    SBUF = ACC;                 //д���ݵ�UART���ݼĴ���
}

/*----------------------------
�����ַ���
----------------------------*/
void SendString(char *s)
{
    while (*s)                  //����ַ���������־
    {
        SendData(*s++);         //���͵�ǰ�ַ�
    }
}


/**********************************************/

void main(void)
{
	PWM_Init();
	ADC_Init();
	CMP_Init();
	UartInit();
	// T0_Iint();

	IE0 = 0;	// ������ж�0��־λ
	// EX0 = 1;	// INT0 Enable
	// IT0 = 1;	//INT0 �½����ж�
	
	// RxPulseWide = 1000;
	PWW_Set = 0;
	cnt10ms = 0;
	Rx_cnt  = 0;
	TimeOut = 0;

	EA = 1; // �����ж�
	// Delay_n_ms(1);	//��ʱ
	SendString("BLDC Test !\r\n");
	while (1)
	{
		Delay_n_ms(1);	//��ʱ1ms, �������ڴ˽���������

		if (TimeOut > 0)
		{
			if(--TimeOut == 0)	//��ת��ʱ
			{
				DISABLE_CMP_INT; 	// �رȽ����ж�
				CCAP0H=0;	CCAP1H=0;	CCAP2H=0;  // ռ�ձ�Ϊ0
				PWM0_L=0;	PWM1_L=0;	PWM2_L=0;
				// RxPulseWide = 1000;
				PWW_Set   = 0;
				PWM_Value = 0;
				B_RxOk = 0;
				B_RUN  = 0;
				Rx_cnt = 0;
				TimeOut = 0;
				SendString("Time Out!\r\n");

				Delay_n_ms(250);	//��תʱ,��ʱ1��������
				Delay_n_ms(250);
				Delay_n_ms(250);
				Delay_n_ms(250);

			}
		}
		
		// if(B_RxOk)	//�յ�һ������
		// {
		// 	B_RxOk = 0;
		// 	j = RxPulseWide;
		// 	if(j >= 1100)				// 1100~1610��ӦPWMռ�ձ�ֵ0~255
		// 	{
		// 		j = (j - 1100) >> 1;	//2us��ӦPWMһ������
		// 		if(j > 256)	j = 255;
		// 	}
		// 	else	j = 0;
		// 	PWW_Set = (u8)j;
		// }
		// SendData(PWW_Set);
		if(B_RxOk) // ��������յ�����
		{
			// SendData(PWW_Set);
			B_RxOk = 0;
			// if(rec==0x22)// ��������
			// {
			// 	if(PWM_Value<250)
			// 	{
			// 		PWM_Value++; // ����ռ�ձ�
			// 	}
			// }
			// else if(rec==0x33)// ��������
			// {
			// 	if(PWM_Value>30)
			// 	{
			// 		PWM_Value--; // ��Сռ�ձ�
			// 	}
			// }
			// else if(rec==0x11) // ��������
			// {
			// 	StartMotor();	// �������
			// 	CMPCR1 &= ~0X40; // ���������жϱ�־λ
			// 	ENABLE_CMP_INT; // �򿪱Ƚ����ж�
			// 	B_RUN = 1;
			// 	TimeOut = 0;
			// }
			// else if(rec==0x44) // ֹͣ����
			// {
			// 	CCAP0H=0;CCAP1H=0;CCAP2H=0;  // ռ�ձȶ���0
			// 	EA = 0; // �ر�ȫ���ж�
			// 	DISABLE_CMP_INT; // �رձȽ����ж�
			// }
		}
		if(!B_RUN && (PWW_Set >= 30))		// PWM_Set >= 30, �������δ����, ���������
		{
			StartMotor();	// �������
			// TimeOut = 0;
		}
		
		
		if(++cnt10ms >= 10)		// 10msʱ϶
		{
			// SendData(cnt10ms);
			cnt10ms = 0;
			if(B_RUN)
			{
				if(PWM_Value < PWW_Set)	PWM_Value++;
				if(PWM_Value > PWW_Set)	PWM_Value--;
				if(PWM_Value < 20)	// ͣת
				{
					PWM_Value = 0;
					B_RUN = 0;
					CCAP0H=0;	CCAP1H=0;	CCAP2H=0;  // ռ�ձ�Ϊ0
					PWM0_L=0;	PWM1_L=0;	PWM2_L=0;
					DISABLE_CMP_INT; // �رȽ����ж�
					SendString("Stop Motor!\r\n");
				}
			}
		}
	}
}




