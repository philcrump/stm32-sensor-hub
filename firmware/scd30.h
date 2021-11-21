#ifndef __SCD30_H__
#define __SCD30_H__

typedef struct {
    bool valid;
    uint8_t fwver[2];
    float instant_co2_ppm;
    float instant_temperature_c;
    float instant_humidity_rh;
    float avg_co2_ppm;
    float avg_temperature_c;
    float avg_humidity_rh;
} scd30_t;

typedef struct {
    float co2_ppm;
    float temperature_c;
    float humidity_rh;
} scd30_history_row_t;

#define SCD30_HISTORY_ROW_COUNT		(24*60*2)  // 24h * 30s

typedef struct {
    bool started;
	scd30_history_row_t rows[SCD30_HISTORY_ROW_COUNT];
    int32_t head;
    bool rolled;
    bool copy_lock; // No mutex so that ISR can yield.
} scd30_history_t;

THD_FUNCTION(scd30_service_thread, arg);

#endif /* __SCD30_H__ */