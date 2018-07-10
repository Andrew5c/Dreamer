/*
** ����ģ�������������
*/
#include "global.h"

//������λ��ͨ��Э��
//���ݷֶη���
#define BYTE0(dwTemp)       (*(char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
#define BYTE4(dwTemp)       (*((char *)(&dwTemp) + 4))
#define BYTE5(dwTemp)       (*((char *)(&dwTemp) + 5))
#define BYTE6(dwTemp)       (*((char *)(&dwTemp) + 6))
#define BYTE7(dwTemp)       (*((char *)(&dwTemp) + 7))

extern MPU6050_DATA mpu_data;  //��Ż�ȡ����
extern EULER euler_angle;

//DS1302�еı������ڴ����жϺ������޸�ʱ���ʱ������
//ע���ʽ���� �� ʱ �� �� �� ��
u8 init_time[8];			 
unsigned char temp[8];
volatile unsigned char adjust_real_time_flag = 0;

unsigned char data_to_send[50] = {0};  //�������ݴ��

unsigned char Tx_Buffer[256] ;
unsigned char Tx_Counter = 0;
unsigned char Tx_Counter_Temp = 0;
unsigned char Rx_Buffer[50] ;		//���ܻ���
unsigned char Rx_Counter = 0;          //���ܼ�������

extern unsigned char CmdRx_Buffer[10];	   //���ϸ�ʽ�Ľ����ַ�����


static void Send_Data(uint8_t *dataToSend , uint8_t length);
static void Uart1_Put_Char(unsigned char DataToSend);
static void Uart1_Put_Buf(unsigned char *DataToSend ,unsigned char data_num);


/*----------------------------------
**�������ƣ�Usart_GPIO_Init
**��������������ʹ�����ų�ʼ��
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
void Usart1_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 				//TX
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   	//�����������
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   //���ö˿��ٶ�Ϊ50M
  	GPIO_Init(GPIOA, &GPIO_InitStructure);				   	//���ݲ�����ʼ��GPIOA�Ĵ���	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 				//RX
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������(��λ״̬);
  	GPIO_Init(GPIOA, &GPIO_InitStructure);				   	//���ݲ�����ʼ��GPIOA�Ĵ���	
}


/*----------------------------------
**�������ƣ�Usart_Configuration
**�������������ڲ������ã������ж�����������
**����˵����BaudRate����Ҫ���õĲ�����
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
void Usart1_Configuration(uint32_t BaudRate)
{
	USART_InitTypeDef USART_InitStructure;							    	//����һ�����ڽṹ��
	NVIC_InitTypeDef  NVIC_InitStructure;									//�ж������ṹ��
	
	USART_DeInit(USART1); 														//��λ����
	USART_InitStructure.USART_BaudRate            =BaudRate ;	  			//�������Լ����ã�����������9600
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b; 	//���������ʹ��8λ����
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;	 	//��֡��β����1λֹͣλ
	USART_InitStructure.USART_Parity              = USART_Parity_No ;	 	//��żʧ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ����ʧ��
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx; //���պͷ���ģʽ
	USART_Init(USART1, &USART_InitStructure);								//���ݲ�����ʼ�����ڼĴ���
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);							//ʹ�ܴ����жϽ���
	USART_Cmd(USART1, ENABLE);     											//ʹ�ܴ�������
	
	//�����������1���ֽ��޷���ȷ���ͳ�ȥ������ 
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* �巢����Ǳ�־��Transmission Complete flag */
	
	
	//���ô����ж�����
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�жϷ���1
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			//�趨�ж�ԴΪ
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//�ж�ռ���ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;			//�����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ʹ���ж�
	NVIC_Init(&NVIC_InitStructure);							   	//���ݲ�����ʼ���жϼĴ���
	
}


/*----------------------------------
**�������ƣ�USART1_IRQHandler
**���������������жϷ�����
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
void USART1_IRQHandler()
{
	unsigned char i = 0;
	unsigned char j = 0;
	
	/*�쳣�жϴ���*/
	if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)	//
    {
       USART_ReceiveData(USART1);										//��������
		 USART_ClearFlag(USART1, USART_FLAG_ORE);
    }
	 
	 /*����*/
	if(USART_GetITStatus(USART1,USART_IT_TXE) != RESET)
	{
		USART1->DR = Tx_Buffer[Tx_Counter_Temp++];    	//��DR�Ĵ���д���ݵ�ʱ�򣬴��ھͻ��Զ�����
			
		if(Tx_Counter == Tx_Counter_Temp)					//���ݰ��������
		{
		   USART_ITConfig(USART1,USART_IT_TXE,DISABLE);	//�ر�TXE�ж�
			Tx_Counter = 0;		//������ɼ�������
			Tx_Counter_Temp = 0;
		}
	}
	  
	 /*���ܣ�����������������͹�����У׼ʱ��*/
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)	//��ȡ�����жϱ�־λUSART_IT_RXNE 
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);	//����жϱ�־λ
		
		Rx_Buffer[Rx_Counter++] = USART_ReceiveData(USART1);//�������ݵ�������������������Զ�����жϱ�־λ
		
		if((Rx_Buffer[Rx_Counter-2] == 0x0d) && (Rx_Buffer[Rx_Counter-1] == 0x0a))//������Ļس����в�������
		{
			memcpy(CmdRx_Buffer,Rx_Buffer,Rx_Counter);	//ֱ��ʹ���ڴ濽����Ч�ʱȽϸ�
			
//			for(i = 0;i<Rx_Counter;i++)
//			{
//				CmdRx_Buffer[i] = Rx_Buffer[i];  //�������������
//			}
			
			CmdRx_Buffer[Rx_Counter] = '\0';		//������'\0',������������
			Rx_Counter = 0;							//��������
			printf("CMD OK!");						//���ճɹ������ͱ�־
		}
		
		//��⵽����̶����ȹ̶���ʽ�����ݰ�������Ϊ��У׼��ʱ�䣬Ҫ���ֻ����͹̶���ʽ��ʱ�����ݰ�:�� �� ʱ �� �� �� ��
		//���磺{12.12.12.05.01.07.18.T}
		else if(Rx_Counter == 22 && Rx_Buffer[Rx_Counter-1] == 'T')
		{
			for(i=0; i<Rx_Counter-1; i+=3)
			{
				//���ֵ�ascii�룬ʮ����0Ϊ48����16����Ϊ������16���ƴ洢��
				temp[j] = (Rx_Buffer[i] - 48)*16 + (Rx_Buffer[i+1] - 48) ;		
				j++;
			}
			
			memcpy(init_time,temp,j);
			init_time[j] = '\0';			
			Rx_Counter = 0;
			adjust_real_time_flag = 1;		//�ñ�־������ѭ�������⣬�����Ƿ�У׼ʱ��
			
			printf("Time have adjusted.");
		}
	}
}


/*----------------------------------
**�������ƣ�fputc
**����������printf�ض�����
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
//�ض���fputc����������ʹ��printf
int fputc(int Data, FILE *f)
{   
	while(!USART_GetFlagStatus(USART1,USART_FLAG_TXE));	 	//USART_GetFlagStatus���õ�����״̬λ
																				//USART_FLAG_TXE:���ͼĴ���Ϊ�� 1��Ϊ�գ�0��æ״̬
	USART_SendData(USART1,Data);						  				//����һ���ַ�	
	return Data;										  					//���ط��͵�ֵ
}


/*�жϷ���һ���ַ�*/
static void Uart1_Put_Char(unsigned char DataToSend)
{
	Tx_Buffer[Tx_Counter++] = DataToSend;  
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE); 
}

/*�жϷ��ͷ�ʽ�������ַ�*/
static void Uart1_Put_Buf(unsigned char *DataToSend ,unsigned char data_num)
{
	uint8_t i;
	
	for(i = 0;i < data_num;i++)	//Ҫ���͵����ݷŵ����ͻ�����
		Tx_Buffer[Tx_Counter++] = *(DataToSend+i);

	if(!(USART1->CR1 & USART_CR1_TXEIE))					//�жϷ�ʽ����
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE); 	//ֻҪ���ͼĴ����գ��ͻ�һֱ���ж�
}

/*----------------------------------
**�������ƣ�USART1_Send_Byte
**����������ʹ�ô��ڷ���һ���ֽڵ�����
**����˵����Data����Ҫ���͵�����
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
void USART1_Send_Byte(uint16_t Data)
{ 
	while(!USART_GetFlagStatus(USART1,USART_FLAG_TXE));	  //USART_GetFlagStatus���õ�����״̬λ
														  //USART_FLAG_TXE:���ͼĴ���Ϊ�� 1��Ϊ�գ�0��æ״̬
	USART_SendData(USART1,Data);						  //����һ���ַ�
}

/*----------------------------------
**�������ƣ�USART1_Receive_Byte
**�������������ڽ���һ���ֽڵ����ݣ�����ֵΪ���յ��ֽ�����
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
uint8_t USART1_Receive_Byte(void)
{
  while(!(USART_GetFlagStatus(USART1,USART_FLAG_RXNE))); //USART_GetFlagStatus���õ�����״̬λ
														   //USART_FLAG_RXNE:�������ݼĴ����ǿձ�־λ 
														   //1��æ״̬  0������(û�յ����ݣ��ȴ�������)
	return USART_ReceiveData(USART1);					   //����һ���ַ�
}

/*------------------
**�������ƣ�Send_Data
**��������������һ������
**����˵����*dataToSend��Ҫ���������ָ��
            length�����鳤��
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------*/
static void Send_Data(uint8_t *dataToSend , uint8_t length)
{
	Uart1_Put_Buf(dataToSend,length);
}


/*----------------------------------
**�������ƣ�Send_Senser
**�������������ʹ����6050����������
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
void Send_Senser(void )
{
	u8 cnt = 0;
	uint16_t sum = 0;
	u8 i = 0;
	int16_t temp_angle;
		
	data_to_send[cnt++]=0xAA;	 //֡ͷ��AAAA
	data_to_send[cnt++]=0xAA;
	data_to_send[cnt++]=0x02;	 //�����֣�OXF2	��0x02һ����
	data_to_send[cnt++]=0;	     //��Ҫ�������ݵ��ֽ�������ʱ��0�������ڸ�ֵ��

	data_to_send[cnt++] = BYTE1(mpu_data.acc_data.x);//ȡdata[0]���ݵĸ��ֽڣ�
	data_to_send[cnt++] = BYTE0(mpu_data.acc_data.x);
	data_to_send[cnt++] = BYTE1(mpu_data.acc_data.y);
	data_to_send[cnt++] = BYTE0(mpu_data.acc_data.y);
	data_to_send[cnt++] = BYTE1(mpu_data.acc_data.z);
	data_to_send[cnt++] = BYTE0(mpu_data.acc_data.z);

	data_to_send[cnt++] = BYTE1(mpu_data.gyro_data.x);//���������ǵ����ݲ���Ҫ�˲��������ͺ��ȶ����˲�֮�󷴶�����
	data_to_send[cnt++] = BYTE0(mpu_data.gyro_data.x);
	data_to_send[cnt++] = BYTE1(mpu_data.gyro_data.y);
	data_to_send[cnt++] = BYTE0(mpu_data.gyro_data.y);
	data_to_send[cnt++] = BYTE1(mpu_data.gyro_data.z);
	data_to_send[cnt++] = BYTE0(mpu_data.gyro_data.z);
	
	temp_angle = (int16_t)(euler_angle.roll);
	data_to_send[cnt++] = BYTE1(temp_angle);
	data_to_send[cnt++] = BYTE0(temp_angle);
	
	temp_angle = (int16_t)(euler_angle.pitch);
	data_to_send[cnt++] = BYTE1(temp_angle);
	data_to_send[cnt++] = BYTE0(temp_angle);
	
	temp_angle = (int16_t)(euler_angle.yaw);
	data_to_send[cnt++] = BYTE1(temp_angle);
	data_to_send[cnt++] = BYTE0(temp_angle);

	data_to_send[3] = cnt-4;//���������ݵ��ֽ�����

	for(i=0;i<cnt;i++)
		sum += data_to_send[i];
	
	data_to_send[cnt++]=sum;  //���һλ��У���

	Send_Data(data_to_send, cnt);   //����һ�����ݰ�
		
}



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
End:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D:-D
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
