#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "can.h"


static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();

static const twai_filter_config_t f_config = {.acceptance_code = (0),
                                             .acceptance_mask = ~(0),
                                             .single_filter = true};

static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NO_ACK);

moto_measure_t moto_chassis[MOTO_NUM];

enum MOTO_ID {
    CAN_3510Moto_ALL_ID = 0x200,
	CAN_3510Moto1_ID = 0x201,
	CAN_3510Moto2_ID = 0x202,
	CAN_3510Moto3_ID = 0x203,
	CAN_3510Moto4_ID = 0x204,
};




esp_err_t can_init(void){

    static TaskHandle_t fb_handle;

    //init can interface hardware and driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));

    //start operation
    ESP_ERROR_CHECK(twai_start());

    if(pdPASS == xTaskCreate(feedback_update_task,"moto_fb",4096,NULL,FB_UPDATE_PRIO,&fb_handle)){
        return ESP_OK;
    }

    return ESP_FAIL;
}


void feedback_update_task(void* n){
    twai_message_t rx_message;

    while(1){
        ESP_ERROR_CHECK(twai_receive(&rx_message, portMAX_DELAY));
        //printf("fb:%hd\n",(rx_message.data[2]<<8 | rx_message.data[3]));
        switch (rx_message.identifier){
            case CAN_3510Moto1_ID:
            case CAN_3510Moto2_ID:
            case CAN_3510Moto3_ID:
            case CAN_3510Moto4_ID:{
                uint8_t i = rx_message.identifier - CAN_3510Moto1_ID;
                get_moto_measure(&moto_chassis[i],&rx_message);
            }
                break;


        }
    }

}

void get_moto_measure(moto_measure_t * ptr,twai_message_t* message_ptr){
    //处理数据
    ptr->last_angle = ptr->angle;
	ptr->angle = (uint16_t)(message_ptr->data[0]<<8 | message_ptr->data[1]);//角度
	ptr->speed_rpm  = (int16_t)(message_ptr->data[2]<<8 | message_ptr->data[3]);//速度
	ptr->real_current = (int16_t)(message_ptr->data[4]<<8 | message_ptr->data[5]);//实际转矩电流
	ptr->temperature = message_ptr->data[6];//电机温度，2006没有，为0


	if(ptr->angle - ptr->last_angle > 4096)
		ptr->round_cnt --;
	else if (ptr->angle - ptr->last_angle < -4096)
		ptr->round_cnt ++;
	ptr->total_angle = ptr->round_cnt * 8192 + ptr->angle - ptr->offset_angle;

}


void set_moto_current(int16_t iq1, int16_t iq2, int16_t iq3, int16_t iq4){
	twai_message_t tx_msg = {.data_length_code=8,.identifier = 0x200,.self=0,.extd=0};


	tx_msg.data[0] = iq1 >> 8;
	tx_msg.data[1] = iq1;
	tx_msg.data[2] = iq2 >> 8;
	tx_msg.data[3] = iq2;
	tx_msg.data[4] = iq3 >> 8;
	tx_msg.data[5] = iq3;
	tx_msg.data[6] = iq4 >> 8;
	tx_msg.data[7] = iq4;

	ESP_ERROR_CHECK(twai_transmit(&tx_msg, portMAX_DELAY));
}	