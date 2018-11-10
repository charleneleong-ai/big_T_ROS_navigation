#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "geometry_msgs/Quaternion.h"
#include "sensor_msgs/Imu.h"
#include "triangle_node/motor_control.h"
#include <math.h>

float omega1, omega2, omega3;
float r = 0.0525; //105mm/2
float h = 0.18; //180mm

float v_x = 0;
float v_y = 0;
float omega_body = 0;

float theta = 3.1415;	

	/*
		v_x		...........	global translation speed in x	......	( in m/s )
		v_y		...........	global translation speed in y	......	( in m/s )
		r		...........	radius of wheel	....................	( in m )
		h		...........	distance from center of body to wheel	( in m )
		omega_body	........... 	rotational speed of body ..........	( in rad/s )
		theta		...........	orientation of the body	............	( in rad )
	*/

//ros::Publisher motor_control_pub;		// set publisher to public
ros::Publisher pub;

ros::Publisher imu_pub;


int	omega2pwm(float omega) {
	/*	
		omega ... angular velocity ( in rad/s )
		rpm = omega*9.5493; // conversion from rad/s to rpm	 ( 1/(2*pi)*60 = 9.5493 )
		pwm = 2.4307*rpm + 36.2178; // conversion of rpm to pwm values
	*/
	if(omega*omega <= 0.05) return 0;

	return (int)(2.43*(sqrt(omega*omega)*9.55) + 36.22);
}

int sign(float number){
	if(number>=0) return 1; else return 0;
}

void imu_callback(const sensor_msgs::Imu & msg){

	imu_pub.publish(msg.orientation);	
}


// void cmd_vel_callback(const geometry_msgs::Twist::ConstPtr& msg){
	void cmd_vel_callback(const geometry_msgs::Twist & msg){
	/*
		omega1	...	rotation speed of motor 1	(in rad/s)
		omega2	...	rotation speed of motor 2	(in rad/s)
		omega3	...	rotation speed of motor 3	(in rad/s)
	*/
	ROS_INFO("Msg received");

	//triangle_node::motor_control out_msg;
	geometry_msgs::Twist out_msg;

	v_x = msg.linear.x;
	v_y = msg.linear.y;
	omega_body = msg.angular.z;

	//omega1 = h/r * omega_body - 2/(3*r) * v_y;
	//omega2 = h/r * omega_body + 1/(3*r) * v_y + sqrt(3)/(3*r) * v_x;
	//omega3 = h/r * omega_body + 1/(3*r) * v_y - sqrt(3)/(3*r) * v_x;

	omega1 = -2/(3*r) * cosf(theta) * v_y + 1/r * omega_body * h + 2/(3*r) * v_x * sinf(theta);
	omega2 = 1/r * cosf(theta) * v_y/3 + 1/r * omega_body * h - 1/r * v_x * sinf(theta)/3 + 1/r * sqrt(3) * sinf(theta) * v_y/3 + 1/r * sqrt(3) * v_x * cosf(theta)/3;
	omega3 = -1/r * sqrt(3) * sinf(theta) * v_y/3 + 1/r * cosf(theta) * v_y/3 + 1/r * omega_body * h - 1/r * sqrt(3) * v_x * cosf(theta)/3 - 1/r * v_x * sinf(theta)/3;

// convert from rad/s to pwm signal

/*	
// pwm signal
	out_msg.MOT1_PWM = omega2pwm(omega1);
	out_msg.MOT2_PWM = omega2pwm(omega2);
	out_msg.MOT3_PWM = omega2pwm(omega3);
	// motor enable
	out_msg.MOT1_EN = 1;
	out_msg.MOT2_EN = 1;
	out_msg.MOT3_EN = 1;
	// motor direction
	out_msg.MOT1_DIR = sign(omega1);		// set direction bit depending on the rotation speed
	out_msg.MOT2_DIR = sign(omega2);
	out_msg.MOT3_DIR = sign(omega3);
*/
	// pwm signal
	out_msg.linear.x = omega2pwm(omega1);
	out_msg.linear.y = omega2pwm(omega2);
	out_msg.linear.z = omega2pwm(omega3);
//	out_msg.linear.x = (omega1);
//	out_msg.linear.y = (omega2);
//	out_msg.linear.z = (omega3);
	// motor direction
	out_msg.angular.x = sign(omega1);		// set direction bit depending on the rotation speed
	out_msg.angular.y = sign(omega2);
	out_msg.angular.z = sign(omega3);

	pub.publish(out_msg);
}


int main(int argc, char **argv){

	ros::init(argc, argv, "triangle_control");

	ros::NodeHandle n;

	//motor_control_pub = n.advertise<triangle_node::motor_control>("motor_control_data", 1000);
	pub = n.advertise<geometry_msgs::Twist>("motor_control_data", 1000);

	ros::Subscriber imu_sub = n.subscribe("imu", 1000, imu_callback);
	
	imu_pub = n.advertise<geometry_msgs::Quaternion>("orientation", 1000);

	ros::Subscriber sub = n.subscribe("cmd_vel", 1000, cmd_vel_callback);

	ros::spin();

	return 0;

}
