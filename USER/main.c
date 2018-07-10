
#include "global.h"

//�������룬����ʱ��ģ��Ŀ���
#define NEED_MPU6050   0
#define NEED_BLUETOOTH 1


//unsigned char string_1[] = {"1.3' OLED TEST  "}; //���Բ����������С
//unsigned char string_2[] = {"THIS IS A TEST  "};

unsigned char CmdRx_Buffer[10] = {0};//���ڽ��ջ���

u8 light_screen_2;
extern volatile u8 light_screen;//�����жϺ����������õı�����������־��
extern volatile u8 timing_flag; //������ʱ��־

extern volatile unsigned char adjust_real_time_flag ;	//�Ƿ�У׼ʱ���־

extern MPU6050_DATA mpu_data ;  //ԭʼ����
extern EULER euler_angle;		  //ת��Ϊ��̬��

//�������еĴ�ѭ��
void main_loop(void);
extern void key_init(void);//�������û��ʹ������ͷ�ļ�������ֱ���������ⲿ�����ɡ�



/*********************������***********************/
int main(void) 
{  	
	OLED_Init();       		//OLED��ʼ��
	
	i2c_GPIO_Config();		//iicЭ���������ã�����Ҫ��6050��ʼ��ǰ�ٺ����ʼ��
		
	DS1302_Init();     		//ʱ��ģ���ʼ��
	
	key_init();        		//������ʼ��
	
	ADC_All_Init();    		//ADC��ʼ��������DMA�����ж�
	
	Usart1_GPIO_Init();      //���ڣ��������ߴ���תUSB����λ����������
	Usart1_Configuration(115200);   		//��������115200������
	
	OLED_Clear();       		//����һ��

		
	light_screen = 0;
	light_screen_2 = 0;
	
	//�ṹ���ʼ������
	memset(&mpu_data,0,sizeof(MPU6050_DATA));	
	memset(&euler_angle,0,sizeof(EULER));	
	
	MPU6050_Init();					//6050��ʼ��
	while(!MPU6050ReadID());  		//�ȴ�6050��ȡ�ɹ�
	IMU_init();
	
	time_2_init(1);			//������ʱ��2����ʱ1ms������ѭ��ִ��һЩ����
	
	while(1)//��ѭ��
	{		
		
		if(light_screen == 1)		//�������߶�����������
		{
			Timing_1ms(3000);//��ʱ3s
			OLED_Display_On(); //����ʾ
			
			while(timing_flag == 0) //ѭ����ⶨʱ����û�е�ʱ��
			{
				if(light_screen == 1)
				{
					Display_Real_Time();//����ʾʱ���ڣ�ʱ�仹����ˢ�µ�
					OLED_Show_RealTime_Battery(110,0);//110��������԰ѵ����ʾ�����Ҳ�
				}
				else
					OLED_Show_Big_Time(36,0,48,2);

			}
			OLED_Clear();			//��ʾ��֮����һ��������ֹӰ����һ������
			OLED_Display_Off(); //����ʾ
			timing_flag = !timing_flag;//��ʱʱ�䵽֮�󣬱�־�ٴ�ȡ�����Ա���һ�ζ�ʱ��ʹ��
			light_screen = !light_screen;				
			
		}
		//����Ƿ���ҪУ׼ʱ��
		if(adjust_real_time_flag != 0)
		{
			adjust_real_time_flag = 0;
			adjust_real_time();			//���ú�����DS1302д��У׼���ʱ��
		}

	}//��ѭ������
		
}


/*
** �ڶ�ʱ���ж�ʱѭ��ִ�����к���
** �ú�������ʱ��2���жϺ������ã���ʱ1ms����һ��
** ����������ж�1ms�����ۼƣ���ͬ������ִ�����ڲ�ͬ��
*/
void main_loop(void)
{
	static unsigned char time_1ms = 0, time_10ms = 0 ;
	static int16_t time_1s = 0;	//�ڶ��ζ�������־
		
	if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET )		//1ms�жϵ���
	{	
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);    		//����жϱ�־
		
  		time_1ms++;
		time_10ms++;
		time_1s++;                                                                                                                                                                                                                                                                                                                                                                                                               

		if(time_1ms == 3)//12ms
		{
			time_1ms = 0;					//�����־	
			mpu6050_data_process();    //��ȡ6050���ݲ�������Ԫ����̬����
						
			#if NEED_MPU6050
			Send_Senser();             //����λ��ͨ�Žӿڣ���������				
			#endif
			
			if(euler_angle.roll >= 150) //�ɶ�������
			{
				light_screen = 1;
			}
			
		}
		
		#if NEED_BLUETOOTH
		if(time_10ms == 10)
		{
			time_10ms = 0;

			if(strstr((const char *)CmdRx_Buffer,"LEDON"))   //�ж��������ڽ��յ��ַ���,strstr���غ����ַ�����ǰ���ַ����еĵ�һ��λ��
			{
				light_screen = 1;
				memset(CmdRx_Buffer,0,sizeof(CmdRx_Buffer));		//��������
			}
			else if(strstr((const char *)CmdRx_Buffer,"LEDOFF"))
			{
				light_screen = 0;
				memset(CmdRx_Buffer,0,sizeof(CmdRx_Buffer));
			}
		
		}
		#endif

		if(time_1s >= 100) 
		{
			time_1s = 0;		//ע������
			if(light_screen == 1 && euler_angle.roll >= 150)
			{
				light_screen = 0;
				light_screen_2 = 1;	//����ʾ��һ��Ҳ����ʱ��Ļ����ϣ�������ʾ�ڶ���ͼ	
			}
		}			
	}	
}



