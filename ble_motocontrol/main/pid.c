
#include <stdlib.h>
#include <string.h>
#include "pid.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void pid_param_init(PID_TypeDef *pid,
                    uint16_t maxout,
                    uint16_t intergral_limit,
                    float deadband,
                    int16_t max_err,
                    int16_t target,
                    float kp,
                    float ki,
                    float kd)
{

    memset(pid, 0, sizeof(PID_TypeDef));

    pid->DeadBand = deadband;
    pid->IntegralLimit = intergral_limit;
    pid->MaxOutput = maxout;
    pid->Max_Err = max_err;
    pid->target = target;

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->output = 0;
}

inline void pid_reset(PID_TypeDef *pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

inline void pid_target(PID_TypeDef *pid, uint16_t new_target)
{
    pid->target = new_target;
}

float pid_cal(PID_TypeDef *pid, int16_t measure)
{

    pid->lasttime = pid->thistime;
    pid->thistime = xTaskGetTickCount() * portTICK_RATE_MS; //换算为绝对时间
    pid->dtime = pid->thistime - pid->lasttime;             //更新两次计算的时间

    pid->last_err = pid->err;
    pid->err = pid->target - measure;

    if (abs(pid->err) > pid->DeadBand)
    {
        pid->pout = pid->kp * pid->err;
        pid->iout += (pid->ki * pid->err);
        pid->dout = pid->kd * (pid->err - pid->last_err); //除以dtime？？

        //积分限幅
        if (pid->iout > pid->IntegralLimit)
            pid->iout = pid->IntegralLimit;
        if (pid->iout < -pid->IntegralLimit)
            pid->iout = -pid->IntegralLimit;

        pid->output = pid->pout + pid->iout + pid->dout;

        if(pid->output>pid->MaxOutput)         
		{
			pid->output = pid->MaxOutput;
		}
		if(pid->output < -(pid->MaxOutput))
		{
			pid->output = -(pid->MaxOutput);
		}

    }

    return pid->output; //进入死区维持原值
}
