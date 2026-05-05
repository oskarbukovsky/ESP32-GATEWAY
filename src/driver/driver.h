#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t driver_init(void);
void driver_set_enabled(bool enabled);
void driver_set_pid_enabled(bool enabled);
void driver_set_position_mode(bool enabled);
void driver_set_target_rps(float target_rps);
void driver_set_manual_pwm(float duty_percent);
void driver_set_direction(bool forward);
void driver_set_home(void);
void driver_set_target_angle_deg(float target_deg);
void driver_set_limit_deg(float limit_deg);

bool driver_is_enabled(void);
bool driver_is_pid_enabled(void);
bool driver_is_position_mode(void);
bool driver_get_direction(void);
bool driver_is_limit_hit(void);
float driver_get_target_rps(void);
float driver_get_output_duty(void);
float driver_get_current_angle_deg(void);
float driver_get_target_angle_deg(void);
float driver_get_limit_deg(void);