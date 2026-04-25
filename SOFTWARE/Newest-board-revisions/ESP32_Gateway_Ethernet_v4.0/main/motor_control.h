#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t motor_control_init(void);
void motor_control_set_enabled(bool enabled);
void motor_control_set_pid_enabled(bool enabled);
void motor_control_set_target_rps(float target_rps);
void motor_control_set_manual_pwm(float duty_percent);

bool motor_control_is_enabled(void);
bool motor_control_is_pid_enabled(void);
float motor_control_get_target_rps(void);
float motor_control_get_output_duty(void);
