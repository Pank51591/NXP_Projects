#include "driver_dc_fan.h"
#include "r_cg_timer.h"
#include "r_cg_intc.h"
#include "rts_log.h"
#include "r_cg_timer.h"
#include "GlobalVariable.h"

static uint32_t time_us=0;
static int32_t target_rpm=0;
static int32_t current_rpm=0;
static int32_t pwm=0;


#define FC 2.0f //截止频率
#define TS 0.02f //采样周期
#define PI 3.14159f //π

float alpha = 0; //滤波系数

void low_pass_filter_init(void)
{
    float b = 2.0 * PI * FC * TS;
    alpha = b / (b + 1);
}

float low_pass_filter(float value)
{
    static float out_last = 0; //上一次滤波值
    float out;

    static char fisrt_flag = 1;
    if (fisrt_flag == 1) {
        fisrt_flag = 0;
        out_last = value;
    }

    out = out_last + alpha * (value - out_last);
    out_last = out;
    return out;
}

void driver_dc_fan_init(void)
{
    //PWM
    R_TAU0_Channel0_Start();

    //FB pin interrput
    R_INTC2_Start();

    //time
    R_TAU0_Channel1_Start();//65535us

    low_pass_filter_init();
}

void driver_dc_fan_set_duty(uint16_t d)
{
    //1000‰ ->16000
    TDR02 =16*d;

    pwm=d;
}

/*
 * 12个脉冲一圈
 *
*/
#pragma vector = INTP2_vect
__interrupt static void r_intc2_interrupt(void)
{
    //get diff time us
    time_us=_FFFF_TAU_TDR01_VALUE-TCR01;
    TS0 |= _0002_TAU_CH1_START_TRG_ON;//restart timer
}

#pragma vector = INTTM01_vect
__interrupt static void r_tau0_channel1_interrupt(void)
{
    //65ms time out
    time_us=0;
}

void driver_dc_fan_set_level(dc_fan_level_t level)
{
    static dc_fan_level_t last_level=FAN_LEVEL_0;

    if(level==last_level) return;
    else last_level=level;

    switch(level)
    {
    case FAN_LEVEL_0:
        target_rpm=0;
        driver_dc_fan_set_duty(0); //preset duty
        break;
    case FAN_LEVEL_1:
        target_rpm=850;
        //driver_dc_fan_set_duty(475); //preset duty
        break;
    case FAN_LEVEL_2:
        target_rpm=980;
        //driver_dc_fan_set_duty(520); //preset duty
        break;
    case FAN_LEVEL_3:
        target_rpm=1150;
        //driver_dc_fan_set_duty(576); //preset duty
        break;
    case FAN_LEVEL_4:
        target_rpm=1350;
        //driver_dc_fan_set_duty(650); //preset duty
        break;
    case FAN_LEVEL_5:
        target_rpm=1500;
        //driver_dc_fan_set_duty(714); //preset duty
        break;
    }
}


#define KP 0.4 // 比例系数
#define KI 0.05 // 积分系数
#define KD 0.0 // 微分系数

float pid_control(float target_speed,float current_speed)
{
    static float last_error = 0;
    static float integral = 0;
    static float error;
    static float derivative;
    static float pwm=0;

    error= target_speed - current_speed;
    integral += error;

    derivative = error - last_error;
    last_error = error;

    pwm=KP * error + KI * integral + KD * derivative;

    if(pwm>1000)pwm=1000;
    else if(pwm<0)pwm=0;
    return pwm;
}

void driver_dc_fan_pid_100ms(void)
{
    static int32_t err=0;
    static uint16_t cnt_10s=(10*10);

    //rpm=1000*1000*60/time_us*12
    if(time_us)current_rpm=5000000U/time_us;
    else current_rpm=0;

    current_rpm=low_pass_filter(current_rpm);

    //检测风机异常
    if((target_rpm!=0) && (current_rpm<400))
    {
        if(cnt_10s)
        {
            cnt_10s--;
            if(cnt_10s==0)
            {
                work.err_code=ERROR_E1;
                //关闭所有执行部件
                work.fan=FAN_LEVEL_0;
                work.relay_wait_off_s=2;
                work.ud_swing_pos=H_SWING_POS_0;
            }
        }
    }
    else cnt_10s=(10*10);

//    //速度微调
//    err=current_rpm-target_rpm;
//    if(err>5)
//    {
//        if(pwm>=1)pwm-=1;
//        else pwm=0;
//    }
//    else if(err<5)
//    {
//        pwm+=1;
//        if(pwm>=1000)pwm=1000;
//    }

//    if(target_rpm==0)pwm=0;

    pwm=pid_control(target_rpm,current_rpm);

    driver_dc_fan_set_duty(pwm);

    //rts_log("$%d ",(int16_t)target_rpm);
    //rts_log("%d ",(int16_t)pwm);
    //rts_log("%d;\r\n",(int16_t)current_rpm);
}
