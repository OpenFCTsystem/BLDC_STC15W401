
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
	sbit PWM2_L = P5^5;
	sbit PWM1_L = P3^3;
	sbit PWM0_L = P3^6;
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
			CCAP0H = PWM_Value;	CCAP1H=0;	CCAP2H=0;	// ��A��ĸ߶�
			PWM1_L = 1; 		// ��B��ĵͶ�
			ADC_CONTR = 0XED;	// ѡ��P1.5��ΪADC���� ��c���ѹ
			CMPCR1 = 0x9C;		//bit7=1 ����Ƚ���, bit4=1 �ȽϽ����1��0, �����½����ж� (������Ӧ�½����ж�?)
			break;
   case 1:  // AC
			PWM0_L=0;	PWM1_L=0;
			CCAP0H = PWM_Value;	CCAP1H=0;	CCAP2H=0;	// ��A��ĸ߶�
			PWM2_L = 1;			// ��C��ĵͶ�
			ADC_CONTR = 0XEC;	// ѡ��P1.4��ΪADC���� ��B���ѹ
			CMPCR1 = 0xAC;		//�������ж�
	 
      break;
   case 2:  // BC
			PWM0_L=0;	PWM1_L=0;
			CCAP0H=0;	CCAP2H=0;	CCAP1H = PWM_Value; // ��B��ĸ߶�
			PWM2_L = 1;			// ��C��ĵͶ�
			ADC_CONTR = 0XEB;	// ѡ��P1.3��ΪADC���� ��a���ѹ
			CMPCR1 = 0x9C;		//�½����ж�
      break;
   case 3:  // BA
			PWM1_L=0;	PWM2_L=0;
			CCAP0H=0;	CCAP2H=0;	CCAP1H = PWM_Value; // ��B��ĸ߶�
			PWM0_L = 1;			// ��A��ĵͶ�
			ADC_CONTR = 0XED;	// ѡ��P1.5��ΪADC���� ��c���ѹ 
			CMPCR1 = 0xAC;		//�������ж�
			
      break;
   case 4: // CA
			PWM1_L=0;	PWM2_L=0;
			CCAP0H=0;	CCAP1H=0;	CCAP2H = PWM_Value; // ��C��ĸ߶�
			PWM0_L = 1;			// ��A��ĵͶ�
			ADC_CONTR = 0XEC;	// ѡ��P1.4��ΪADC���� ��B���ѹ
			CMPCR1 = 0x9C;		//�½����ж�
      break;
   case 5: // CB
      		PWM0_L=0;	PWM2_L=0;
			CCAP0H=0;	CCAP1H=0;	CCAP2H = PWM_Value;// ��C��ĸ߶�
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

//	CMOD = 1 << 1; //ѡ��ϵͳʱ��/2Ϊʱ��Դ����PWMƵ��=24M/2/256=46.9K
	CMOD = 5 << 1; //ѡ��ϵͳʱ��/4Ϊʱ��Դ����PWMƵ��=24M/4/256=23.4K
//	CMOD = 6 << 1; //ѡ��ϵͳʱ��/6Ϊʱ��Դ����PWMƵ��=24M/6/256=15.6K
	CL=0;			// PCA����������
	CH=0;
	
	PCA_PWM0 = 0X00;
	CCAP0H=0;    // ��ʼ��ռ�ձ�Ϊ0% H��ֵװ�ص�L��
	CCAP0L=0;
	CCAPM0=0x42;	// ����ΪPWMģʽ
	
	PCA_PWM1 = 0X00;
	CCAP1H=0;    // ��ʼ��ռ�ձ�Ϊ0%
	CCAP1L=0;
	CCAPM1=0x42;	// ����ΪPWMģʽ
	
	PCA_PWM2 = 0X00;
	CCAP2H=0;    // ��ʼ��ռ�ձ�Ϊ0%
	CCAP2L=0;
	CCAPM2=0x42;	// ����ΪPWMģʽ
	
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
		for(i=0; i<timer; i++)	delay_us(50);  //
		timer -= timer /15 + 1;
		if(timer < 25)	return(1);
		if( Step < 5)	Step++;
		else			Step = 0;
		StepXL();
	}
}

void T0_Iint(void)
{
	Timer0_AsTimer();	/* ʱ��0������ʱ��	*/
	Timer0_12T();		/* Timer0 clodk = fo/12	12��Ƶ,	default	*/
	Timer0_16bit();
	Timer0_Gate_INT0_P32();	/* ʱ��0���ⲿINT0�ߵ�ƽ����ʱ���� */
	TH0 = 0;
	TL0 = 0;
	TR0 = 1; // �򿪶�ʱ��0
	ET0 = 1;// ����ET0�ж�
}

void T0_Interrupt(void) interrupt 1
{
	Rx_cnt = 0;			//һ���������, ��ʼ��n��������Ч
	RxPulseWide = 1000;	//ֹͣ
	B_RxOk = 1;			//�����յ�һ������
}

/********************* INT0�жϺ��� *************************/
void INT0_int (void) interrupt INT0_VECTOR
{
	u16	j;
	
	TR0 = 0;
	j = ((u16)TH0 << 8) + TL0;
	TH0 = 0;
	TL0 = 0;
	TR0 = 1;

	if(++Rx_cnt >= 5)	Rx_cnt = 5;
	j >>= 1;	//Ϊ�˺ô���, ת�ɵ�λΪus
	if((j >= 800) && (j <= 2000) && (Rx_cnt == 5))
	{
		RxPulseWide = j;
		B_RxOk = 1;		//��־�յ�һ������
	}

}


/**********************************************/
void main(void)
{
	u16 j;

	PWM_Init();
	ADC_Init();
	CMP_Init();
	T0_Iint();

	IE0 = 0;	// ������ж�0��־λ
	EX0 = 1;	// INT0 Enable
	IT0 = 1;	//INT0 �½����ж�
	
	RxPulseWide = 1000;
	PWW_Set = 0;
	cnt10ms = 0;
	Rx_cnt  = 0;
	TimeOut = 0;

	EA  = 1; // �����ж�
	
	while (1)
	{
		Delay_n_ms(1);	//��ʱ1ms, �������ڴ˽���������

		if(TimeOut > 0)
		{
			if(--TimeOut == 0)	//��ת��ʱ
			{
				CCAP0H=0;	CCAP1H=0;	CCAP2H=0;  // ռ�ձ�Ϊ0
				PWM0_L=0;	PWM1_L=0;	PWM2_L=0;
				DISABLE_CMP_INT; // �رȽ����ж�
				Delay_n_ms(250);	//��תʱ,��ʱ1��������
				Delay_n_ms(250);
				Delay_n_ms(250);
				Delay_n_ms(250);

				RxPulseWide = 1000;
				PWW_Set   = 0;
				PWM_Value = 0;
				B_RxOk = 0;
				B_RUN  = 0;
				Rx_cnt = 0;
				TimeOut = 0;
			}
		}
		
		if(B_RxOk)	//�յ�һ������
		{
			B_RxOk = 0;
			j = RxPulseWide;
			if(j >= 1100)				// 1100~1610��ӦPWMռ�ձ�ֵ0~255
			{
				j = (j - 1100) >> 1;	//2us��ӦPWMһ������
				if(j > 256)	j = 255;
			}
			else	j = 0;
			PWW_Set = (u8)j;
		}
		
		if(!B_RUN && (PWW_Set >= 30))		// PWM_Set >= 30, �������δ����, ���������
		{
			StartMotor();	// �������
			CMPCR1 &= ~0X40; // ���������жϱ�־λ
			ENABLE_CMP_INT; // �򿪱Ƚ����ж�
			B_RUN = 1;
			TimeOut = 0;
		}
		
		
		if(++cnt10ms >= 10)		// 10msʱ϶
		{
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
				}
			}
		}
	
	}
}



