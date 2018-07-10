
#include "global.h"


//ͨ������Kp��Ki�������������Կ��Ƽ��ٶȼ����������ǻ�����̬���ٶ�
#define Kp 3.0f       	
#define Ki 0.003f       		
#define halfT 0.004f    // �������ڵ�һ��

extern MPU6050_DATA mpu_data ;
extern EULER euler_angle;

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;          // ��Ԫ����Ԫ�أ�������Ʒ���
float exInt = 0, eyInt = 0, ezInt = 0;         // ��������С�������

/*
** ��Ԫ������ֵ
** ������ðѼ��ٶȼ�У׼�����ʱ��ľ���ȡ���ڼ��ٶȼƵľ��ȡ�����У׼.
** û�дŴ���������̬�ں��£�ֻ����ʱ��ƫ���� = 0��������ֻ��ȷ��ˮƽ��
** ֻ������������ʼ����ʱ�����һ�μ���.
*/
void IMU_init(void)
{
	float norm;
	float init_ax, init_ay, init_az;
	float init_roll, init_pitch, init_yaw;
	
	MPU6050ReadAcc();
	
	init_ax =  (float)mpu_data.acc_data.x / ACC_8G;	   //���㵥λΪG�ĸ����������ٶȷ���,�������й�
	init_ay =  (float)mpu_data.acc_data.y / ACC_8G;
	init_az =  (float)mpu_data.acc_data.z / ACC_8G;

	//��һ��
	norm = sqrt(init_ax*init_ax + init_ay*init_ay + init_az*init_az);      
	init_ax = init_ax / norm;            
	init_ay = init_ay / norm;
	init_az = init_az / norm; 
	
	//������x��Ϊǰ������
	init_roll  = atan2(init_ay, init_az);
	init_pitch = -asin(init_ax);              //init_Pitch = asin(ax / 1);   
//	init_yaw   = -atan2(init_my*cos(init_Roll) - init_mz*sin(init_Roll),init_mx*cos(init_Pitch) + init_my*sin(init_Roll)*sin(init_Pitch) + init_mz*sin(init_Pitch)*cos(init_Roll));                       //atan2(mx, my);
   init_yaw   = 0;
 
	q0 = cos(0.5*init_roll)*cos(0.5*init_pitch)*cos(0.5*init_yaw) + sin(0.5*init_roll)*sin(0.5*init_pitch)*sin(0.5*init_yaw);  //w
	q1 = sin(0.5*init_roll)*cos(0.5*init_pitch)*cos(0.5*init_yaw) - cos(0.5*init_roll)*sin(0.5*init_pitch)*sin(0.5*init_yaw);  //x   ��x����ת��roll
	q2 = cos(0.5*init_roll)*sin(0.5*init_pitch)*cos(0.5*init_yaw) + sin(0.5*init_roll)*cos(0.5*init_pitch)*sin(0.5*init_yaw);  //y   ��y����ת��pitch
	q3 = cos(0.5*init_roll)*cos(0.5*init_pitch)*sin(0.5*init_yaw) - sin(0.5*init_roll)*sin(0.5*init_pitch)*cos(0.5*init_yaw);  //z   ��z����ת��yaw

 
}

/* ��Ԫ�������㷨
** ����gx��gy��gz�ֱ��Ӧ������Ľ��ٶȣ���λ�ǻ���/��;
** ����ax��ay��az�ֱ��Ӧ������ļ��ٶ�ԭʼ����,���ڼ��ٶȵ������ϴ󣬴˴�Ӧ�����˲��������
*/
void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
{
		float norm;
		float vx, vy, vz;
		float ex, ey, ez; 
	
//		float q0_last,	q1_last,q2_last,q3_last;	//���д��д�����ԣ�д����ֻ�Ǳ�����⣬��ʵ����q0��һ��ֵ
//		 //��Ԫ�����֣���õ�ǰ��̬
//		 q0_last = q0;
//		 q1_last = q1;
//		 q2_last = q2;
//		 q3_last = q3;
	
		//���ٶȼƵ���ά����ת��Ϊ��λ����
		norm = sqrt(ax*ax + ay*ay + az*az);      
		ax = ax / norm;            
		ay = ay / norm;
		az = az / norm;      

		//�����������ٶȵķ����ڷ����������еı�ʾ
		vx = 2*(q1*q3 - q0*q2);
		vy = 2*(q0*q1 + q2*q3);
		vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

		//���ٶȼƶ�ȡ�ķ������������ٶȶ�ȡ����Ĳ�ֵ�������Ĳ��
		ex = (ay*vz - az*vy);
		ey = (az*vx - ax*vz);
		ez = (ax*vy - ay*vx);

		//����ۼ�
		exInt = exInt + ex*Ki;
		eyInt = eyInt + ey*Ki;
		ezInt = ezInt + ez*Ki;

		//�ò������PI����������ƫ�������������е�ƫ����
		gx = gx + Kp*ex + exInt;
		gy = gy + Kp*ey + eyInt;
		gz = gz + Kp*ez + ezInt;


		//һ�׽����㷨����Ԫ���˶����̵���ɢ����ʽ�ͻ���
		q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
		q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
		q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
		q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;  

		//��Ԫ���淶��
		norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
		q0 = q0 / norm;
		q1 = q1 / norm;
		q2 = q2 / norm;
		q3 = q3 / norm;

}

/*
** ��Ԫ��ת��Ϊŷ���ǣ�������ת��Ĵ���ͬ����ʽҲ��ͬ��
** �ڴ���x��Ϊ��ת�ᡣ
*/
void get_euler_angle(void)
{	
	euler_angle.roll  = atan2(2.0f * (q0*q1+q2*q3),q0*q0 - q1*q1 - q2*q2 + q3*q3) * 180 / PI;
	euler_angle.pitch = asin(2.0f * (q0*q2 - q1*q3)) * 180 / PI;
	euler_angle.yaw   = atan2(2.0f * (q0*q1 + q2*q3),q0*q0 + q1*q1 - q2*q2 - q3*q3) * 180 / PI;
	
}


