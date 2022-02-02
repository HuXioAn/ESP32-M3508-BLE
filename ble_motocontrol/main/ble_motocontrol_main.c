#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "can.h"
#include "pid.h"
#include "ble.h"

PID_TypeDef moto_pid;


void app_main(void)
{
  ESP_ERROR_CHECK(can_init());

  ble_init();

  pid_param_init(&moto_pid,16000,1000,10,8000,0,3.0f,0.2f,0.0f);

  moto_pid.target=0;

   uint32_t lt=0;
   lt=xTaskGetTickCount();

   while(1){
      if(xTaskGetTickCount()-lt > 1000/portTICK_RATE_MS){
         printf("%hd\n",moto_chassis->speed_rpm);
         lt=xTaskGetTickCount();
      }

       pid_cal(&moto_pid,moto_chassis[0].speed_rpm);

       set_moto_current(moto_pid.output,0,0,0);

       vTaskDelay(10/portTICK_RATE_MS);


   }

}
