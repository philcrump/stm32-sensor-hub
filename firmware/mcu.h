#ifndef __MCU_H__
#define __MCU_H__

typedef struct {
    float vbat_volts;
    float temperature_degrees;
    char *firmware_name;
    char *firmware_version;
} mcu_info_t;

THD_FUNCTION(mcu_service_thread, arg);

#endif /* __MCU_H__ */