
//���밴����ʵ��׶Σ���������ʱ����������10���Ϩ����������ϣ��ð�J-LINK���߰��˲����ð���KEY2��
//���������key2 ����PA15
//���жϷ���ʱ�����������Ȼ�����ж��ֳ��ı�����Ȼ����ת���ж��������в����жϴ���������ڵ�ַ��
//����ִ����Ӧ���жϴ�����
#include <stm32f10x.h>

/*----------------------------------
**�������ƣ�key_init
**�����������������ų�ʼ��
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.25
-----------------------------------*/
void key_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;			//����һ��EXTI�ṹ�����

	//1��ʹ��GPIO���ⲿ�жϱ�������APIOʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);

	//2��GPIO��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //�ж�����Ϊ��������
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //�����������ԣ�Ҳ����
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//3������EXTI��
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource15);//�����ж�Դ,��GPIO���ж�ӳ��һ��
	EXTI_InitStructure.EXTI_Line = EXTI_Line15 ;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//�ж�ģʽΪ�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;	//�½��س���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				//ʹ���ж���
	EXTI_Init(&EXTI_InitStructure);							//���ݲ�����ʼ���жϼĴ���
	
	//4�������ж�����
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�жϷ���1
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//�趨�ж�ԴΪ
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//�ж�ռ���ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//�����ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ʹ���ж�
	NVIC_Init(&NVIC_InitStructure);							   	//���ݲ�����ʼ���жϼĴ���
	
	//���ˣ��ж����е����þ��������ˣ����Ǽ򵥵İ������жϣ������Ŀ���Ҫ����һ��
	
	
}
//�жϺ�����IT.C��

