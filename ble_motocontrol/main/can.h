#ifndef _CAN_H_
#define _CAN_H_

#include "stdint.h"
#include "esp_err.h"
#include "driver/twai.h"

#define TX_GPIO_NUM 13
#define RX_GPIO_NUM 15
#define MOTO_NUM 1
#define FB_UPDATE_PRIO 10

typedef struct{
	int16_t	 	speed_rpm;
    int16_t  	real_current;
    //int16_t  	given_current;
    uint8_t  	temperature;
	uint16_t 	angle;				//abs angle range:[0,8191]
	uint16_t 	last_angle;	//abs angle range:[0,8191]
	uint16_t	offset_angle;
	int32_t		round_cnt;
	int32_t		total_angle;
	//uint8_t		buf_idx;
	//uint16_t	angle_buf[FILTER_BUF_LEN];
	//uint16_t	fited_angle;
	uint32_t	msg_cnt;
}moto_measure_t;

extern moto_measure_t moto_chassis[MOTO_NUM];



esp_err_t  can_init(void);//初始化要求接入CAN收发器，形成rxtx短接


//construct a task updating moto_chassis from rx queue
void feedback_update_task(void*);


void get_moto_measure(moto_measure_t * moto_p,twai_message_t* message_p);

void set_moto_current(int16_t iq1, int16_t iq2, int16_t iq3, int16_t iq4);


#endif