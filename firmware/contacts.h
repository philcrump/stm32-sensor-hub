#ifndef __CONTACTS_H__
#define __CONTACTS_H__

typedef struct {
    char *name;
    bool value;
    uint32_t line;
} contact_t;

typedef struct {
    contact_t *contact;
    uint32_t count;
} contacts_t;

THD_FUNCTION(contacts_service_thread, arg);

#endif /* __CONTACTS_H__ */