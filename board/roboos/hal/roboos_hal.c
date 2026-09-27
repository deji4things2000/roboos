/*
 * roboos_hal.c - RoboOS Hardware Abstraction Layer for PiCar-X
 *
 * Communicates with the Robot HAT v4 (AT32F415 MCU) at I2C address 0x14
 * on /dev/i2c-1. Provides motor control primitives.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_BUS         "/dev/i2c-1"
#define HAT_ADDR        0x14

#define REG_PWM_PRESCALER   0x43
#define REG_PWM_PERIOD      0x47
#define REG_MOTOR_LEFT      0x2D
#define REG_MOTOR_RIGHT     0x2C

#define PWM_PRESCALER       848
#define PWM_PERIOD          849
#define PWM_MAX             4095

static int hat_fd = -1;

static int hat_write_reg16(uint8_t reg, uint16_t value)
{
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = value & 0xFF;

    if (write(hat_fd, buf, 3) != 3) {
        perror("HAL: i2c write failed");
        return -1;
    }
    return 0;
}

static int hat_read_reg8(uint8_t reg)
{
    if (write(hat_fd, &reg, 1) != 1) {
        perror("HAL: i2c write (addr) failed");
        return -1;
    }
    uint8_t value = 0;
    if (read(hat_fd, &value, 1) != 1) {
        perror("HAL: i2c read failed");
        return -1;
    }
    return (int)value;
}

int hal_init(void)
{
    hat_fd = open(I2C_BUS, O_RDWR);
    if (hat_fd < 0) {
        perror("HAL: cannot open I2C bus");
        return -1;
    }

    if (ioctl(hat_fd, I2C_SLAVE, HAT_ADDR) < 0) {
        perror("HAL: cannot set I2C slave address");
        close(hat_fd);
        hat_fd = -1;
        return -1;
    }

    if (hat_write_reg16(REG_PWM_PRESCALER, PWM_PRESCALER) < 0) return -1;
    if (hat_write_reg16(REG_PWM_PERIOD,    PWM_PERIOD)    < 0) return -1;

    printf("HAL: initialized on %s (HAT address 0x%02X)\n",
           I2C_BUS, HAT_ADDR);
    return 0;
}

int hal_motor_set(uint8_t motor_id, float speed)
{
    if (hat_fd < 0) return -1;

    if (speed >  1.0f) speed =  1.0f;
    if (speed < -1.0f) speed = -1.0f;

    float magnitude = fabsf(speed);
    uint16_t pwm = (uint16_t)(magnitude * PWM_MAX);

    uint8_t reg = (motor_id == 0) ? REG_MOTOR_LEFT : REG_MOTOR_RIGHT;

    if (hat_write_reg16(reg, pwm) < 0) return -1;

    printf("HAL: motor %u speed %.2f (PWM %u)\n",
           motor_id, speed, pwm);
    return 0;
}

int hal_motor_stop_all(void)
{
    int r = 0;
    r |= hal_motor_set(0, 0.0f);
    r |= hal_motor_set(1, 0.0f);
    return r;
}

int hal_hat_version(void)
{
    if (hat_fd < 0) return -1;
    return hat_read_reg8(0x01);
}

void hal_cleanup(void)
{
    if (hat_fd >= 0) {
        hal_motor_stop_all();
        close(hat_fd);
        hat_fd = -1;
        printf("HAL: shutdown complete\n");
    }
}

#ifdef HAL_TEST_MAIN
int main(void)
{
    printf("=== RoboOS HAL Test ===\n");

    if (hal_init() < 0) return 1;

    int ver = hal_hat_version();
    printf("HAT firmware version: 0x%02X\n", ver);

    printf("Moving left motor forward at 50%%...\n");
    hal_motor_set(0, 0.5f);
    sleep(2);

    printf("Stopping left motor...\n");
    hal_motor_set(0, 0.0f);

    printf("Moving right motor forward at 50%%...\n");
    hal_motor_set(1, 0.5f);
    sleep(2);

    printf("Stopping all motors...\n");
    hal_motor_stop_all();

    hal_cleanup();
    printf("=== Test complete ===\n");
    return 0;
}
#endif
