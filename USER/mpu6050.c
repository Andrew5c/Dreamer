
#include "global.h"



MPU6050_DATA mpu_data ;  		//ԭʼ����
EULER euler_angle;				//ת��Ϊ��̬��

#define ACC_FILTER_NUM 3		//���ٶȼƻ����˲�����������
#define GYRO_FILTER_NUM 3		//�����ǻ����˲�����������


//���ڴ��ļ��ڵ��ã�����Ϊ��̬����
//static void MPU6050ReadGyro(void);
//static void MPU6050ReadAcc(void);
static void MPU6050_ReadTemp(short*Temperature);
static void MPU6050_ReadData(u8 reg_add,unsigned char*Read,u8 num);
static void MPU6050_WriteReg(u8 reg_add,u8 reg_dat);


/*----------------------------------
**�������ƣ�MPU6050_WriteReg
**����������6050д�Ĵ�������
**����˵����reg_add���Ĵ�����ַ
						reg_dat��д�������
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
static void MPU6050_WriteReg(u8 reg_add,u8 reg_dat)
{
	i2c_Start();
	i2c_SendByte(MPU6050_SLAVE_ADDRESS);    //д�ӻ���ַ�������ó�дģʽ
	i2c_WaitAck();
	i2c_SendByte(reg_add);     //д�Ĵ�����ַ
	i2c_WaitAck();
	i2c_SendByte(reg_dat);     //д�Ĵ�������
	i2c_WaitAck();
	i2c_Stop();
}


/*----------------------------------
**�������ƣ�MPU6050_ReadData
**������������ȡ6050����
**����˵����reg_add���Ĵ�����ַ
				* Read����Ŷ�ȡ����ֵ
				num����Ҫ��ȡ�Ĵ���
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
static void MPU6050_ReadData(u8 reg_add,unsigned char *Read,u8 num)
{
	unsigned char i;
	
	i2c_Start();
	i2c_SendByte(MPU6050_SLAVE_ADDRESS);
	i2c_WaitAck();
	i2c_SendByte(reg_add);
	i2c_WaitAck();
	
	i2c_Start();
	i2c_SendByte(MPU6050_SLAVE_ADDRESS+1);   //д�ӻ���ַ�������óɶ�ģʽ
	i2c_WaitAck();
	
	for(i=0;i<num;i++)
	{
		*Read=i2c_ReadByte(1);
		Read++;
	}
	*Read=i2c_ReadByte(0);
	i2c_Stop();
}

//**************����mpu6050�Ķ�д�����������㶨�ˣ�����ֻ��Ҫ��ʼ���Ϳ��Զ�ȡ����������


/*----------------------------------
**�������ƣ�MPU6050_Init
**����������6050��ʼ�������ּĴ�����ʼ���ݣ��μ��Ĵ����������ܱ�
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.24
**2018.3.19  ���ڼ��ٶȼƲ������á�0x00->2G,0x10->8G
-----------------------------------*/
void MPU6050_Init(void)
{
  int i=0,j=0;
	
  //�ڳ�ʼ��֮ǰҪ��ʱһ��ʱ�䣬��û����ʱ����ϵ�����ϵ����ݿ��ܻ����
  //����ʱ��Ϊ 0.28646919s - 0.21462243s = 0.072s,72ms
  for(i=0;i<1000;i++)
  {
    for(j=0;j<1000;j++)
    {
		;
    }
  } 
	MPU6050_WriteReg(MPU6050_RA_PWR_MGMT_1, 0x00);	    //�������״̬
	MPU6050_WriteReg(MPU6050_RA_SMPLRT_DIV , 0x07);	    //�����ǲ����ʣ�1KHz
	MPU6050_WriteReg(MPU6050_RA_CONFIG , 0x06);	        //��ͨ�˲��������ã���ֹƵ����1K��������5K
	MPU6050_WriteReg(MPU6050_RA_ACCEL_CONFIG , 0x00);	  //���ü��ٶȴ�����������+-8Gģʽ�����Լ�,32767/8 = 4096LSB/G
	MPU6050_WriteReg(MPU6050_RA_GYRO_CONFIG, 0x18);     //�������Լ켰������Χ������ֵ��0x18(���Լ죬2000deg/s,16.4LSB/S)
}

/*----------------------------------
**�������ƣ�MPU6050ReadID
**�������������ݶ�ȡMPU6050_RA_WHO_AM_I�Ĵ�����ֵȷ��6050�Ƿ���ȷ��ʼ��
**����˵��������ֵ1�ɹ���0���ɹ�
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
uint8_t MPU6050ReadID(void)
{
	unsigned char Re = 0;
	
    MPU6050_ReadData(MPU6050_RA_WHO_AM_I,&Re,1);    //��������ַ
	
	if(Re != 0x68)  //����Ĵ����ĵ���ֵΪ0x68
	{
//		printf("MPU6050 dectected error!\r\n��ⲻ��MPU6050ģ�飬����ģ���뿪����Ľ���");
		return 0;
	}
	else
	{
//		printf("MPU6050 ID = %d\r\n",Re);
		return 1;
	}

}

/*----------------------------------
**�������ƣ�MPU6050ReadAcc
**������������ȡ���ٶȼĴ�����ֵ�����ڱ�Ҫ��ʱ���ȥ��ƫֵ
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
void MPU6050ReadAcc(void)
{
    uint8_t buf[6];
	 int16_t accData[3];	//���ٶȼ����16λ������������ʵint16_t���� signed short int ռ2���ֽ�
	
    MPU6050_ReadData(MPU6050_ACC_OUT, buf, 6);   //��ȡ���ٶȼƼĴ��������ݣ���������ٶ�
    accData[0] = (int16_t)((buf[0] << 8) | buf[1]);	 //��ת��Ϊ16λ���ݣ�������8λ��
    accData[1] = (int16_t)((buf[2] << 8) | buf[3]);
    accData[2] = (int16_t)((buf[4] << 8) | buf[5]);
	
		mpu_data.acc_data.x = accData[0];
		mpu_data.acc_data.y = accData[1];
		mpu_data.acc_data.z = accData[2];
		
}


/*----------------------------------
**�������ƣ�MPU6050ReadGyro
**������������ȡ��������ֵ
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
void MPU6050ReadGyro(void)
{
    uint8_t buf[6];
	 int16_t gyroData[3];
	
    MPU6050_ReadData(MPU6050_GYRO_OUT,buf,6);  //��ȡ�����ǼĴ��������ݣ��������ǽǼ��ٶ�
    gyroData[0] = (int16_t)((buf[0] << 8) | buf[1]);
    gyroData[1] = (int16_t)((buf[2] << 8) | buf[3]);
    gyroData[2] = (int16_t)((buf[4] << 8) | buf[5]);
	
		mpu_data.gyro_data.x = gyroData[0];
		mpu_data.gyro_data.y = gyroData[1];
		mpu_data.gyro_data.z = gyroData[2];
	
}


/*----------------------------------
**�������ƣ�MPU6050_ReadTemp
**������������ȡ6050�¶ȣ�ת�������϶�
**����˵����* Temperature�����ת������¶�ֵ
**���ߣ�Andrew
**���ڣ�2018.1.24
-----------------------------------*/
static void MPU6050_ReadTemp(short* Temperature)
{
	short temp3;
	uint8_t buf[2];
	
	MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H,buf,2);     //��ȡ�¶�ֵ
	
	temp3= (buf[0] << 8) | buf[1];
	
	*Temperature=(((double) (temp3 + 13200)) / 280)-13;

}


/*----------------------------------
**�������ƣ�mpu6050_data_process
**����������6050ԭʼ���ݵ��˲�
**����˵������
**���ߣ�Andrew
**���ڣ�2018.1.24
**���������ʱ��� 7ms
-----------------------------------*/
void mpu6050_data_process(void)
{
	unsigned char i = 0,  j = 0;
	static unsigned char acc_filter_cnt = 0;//�˲���������
	static unsigned char gyro_filter_cnt = 0;
	
	int32_t  acc_temp[3] = {0};//������ٶ�,ע�⣬������ܶ���Ϊ��̬
	int32_t  gyro_temp[3] = {0};
	
	static int16_t  acc_x_buf[ACC_FILTER_NUM] = {0}, //�������ڻ��棬ÿ����һ�Σ��������ݶ�һ����
						 acc_y_buf[ACC_FILTER_NUM] = {0},
						 acc_z_buf[ACC_FILTER_NUM] = {0};

	static int16_t  gyro_x_buf[GYRO_FILTER_NUM] = {0}, //�������ڻ��棬ÿ����һ�Σ��������ݶ�һ����
						 gyro_y_buf[GYRO_FILTER_NUM] = {0},
   	             gyro_z_buf[GYRO_FILTER_NUM] = {0};

//	float  init_ax = 0,init_ay = 0,init_az = 0;

//�ṹ���ʼ����������������ʼ�����棬����ǰ����һ�μ��ɡ�
//	memset(&mpu_data,0,sizeof(MPU6050_DATA));	
//	memset(&euler_angle,0,sizeof(EULER));	

//��ȡԭʼ����						 
	MPU6050ReadAcc();   
	MPU6050ReadGyro();

//�����˲�,���ٶȼ����ݴ���
//-----------------------------------------------------
	acc_x_buf[acc_filter_cnt] = mpu_data.acc_data.x;
	acc_y_buf[acc_filter_cnt] = mpu_data.acc_data.y;
	acc_z_buf[acc_filter_cnt] = mpu_data.acc_data.z;

	for(i=0;i<ACC_FILTER_NUM;i++)	
	{
		acc_temp[0]+= acc_x_buf[i];
		acc_temp[1]+= acc_y_buf[i];
		acc_temp[2]+= acc_z_buf[i];
	}

	//---------------------------
	mpu_data.acc_filter.x = (float)acc_temp[0] / ACC_FILTER_NUM; 
	mpu_data.acc_filter.y = (float)acc_temp[1] / ACC_FILTER_NUM;
	mpu_data.acc_filter.z = (float)acc_temp[2] / ACC_FILTER_NUM;
	//--------------------------

	acc_filter_cnt++;
	if(acc_filter_cnt == ACC_FILTER_NUM)
	{
		acc_filter_cnt = 0;
	}
	

//���������ݴ���
//-----------------------------------------------------------
	gyro_x_buf[gyro_filter_cnt] = mpu_data.gyro_data.x;
	gyro_y_buf[gyro_filter_cnt] = mpu_data.gyro_data.y;
	gyro_z_buf[gyro_filter_cnt] = mpu_data.gyro_data.z;

	for(j=0;j<GYRO_FILTER_NUM;j++)
	{
		gyro_temp[0]+= gyro_x_buf[j];
		gyro_temp[1]+= gyro_y_buf[j];
		gyro_temp[2]+= gyro_z_buf[j];
	}

	//-----------------------------
	mpu_data.gyro_filter.x = gyro_temp[0] / GYRO_FILTER_NUM;
	mpu_data.gyro_filter.y = gyro_temp[1] / GYRO_FILTER_NUM;
	mpu_data.gyro_filter.z = gyro_temp[2] / GYRO_FILTER_NUM;
	//--------------------------

	gyro_filter_cnt++;
	if(gyro_filter_cnt == GYRO_FILTER_NUM)
	{
		gyro_filter_cnt = 0;
	}

	//���������ݻ�Ϊ������
	mpu_data.gyro_rad.x = (float)mpu_data.gyro_data.x * GYRO_R;
	mpu_data.gyro_rad.y = (float)mpu_data.gyro_data.y * GYRO_R;
	mpu_data.gyro_rad.z = (float)mpu_data.gyro_data.z * GYRO_R;

	
	//��Ԫ�����£�ע�⴫��Ĳ�����˳��
	IMUupdate((float)mpu_data.gyro_rad.x,(float)mpu_data.gyro_rad.y,(float)mpu_data.gyro_rad.z,\
				 (float)mpu_data.acc_filter.x,(float)mpu_data.acc_filter.y,(float)mpu_data.acc_filter.z);
	
	//������Ԫ������ŷ����
	get_euler_angle();
	
}


