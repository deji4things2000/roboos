// roboos_hal.h
#ifndef ROBOOS_HAL_H
#define ROBOOS_HAL_H

// Initializes GPIO and I2C for the PiCar-X
int hal_init(void);

// Sets the speed and direction for a single motor
// motor_id: 0 for left, 1 for right
// speed: -1.0 (full reverse) to 1.0 (full forward)
void hal_motor_set(uint8_t motor_id, float speed);

// Sets the steering angle in degrees
// angle: -90 (full left) to 90 (full right)
void hal_steer_set(float angle);

// Cleanup
void hal_cleanup(void);

#endif