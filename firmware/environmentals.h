#ifndef __ENVIRONMENTALS_H__
#define __ENVIRONMENTALS_H__

typedef struct {
    char *name;
    float temperature;
    float humidity;
    /* TODO: Driver config */
} environmental_t;

typedef struct {
    environmental_t *environmental;
    uint32_t count;
} environmentals_t;

THD_FUNCTION(environmentals_service_thread, arg);

#endif /* __ENVIRONMENTALS_H__ */