#ifndef __MAIN_H__
#define __MAIN_H__

#include "ch.h"
#include "hal.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

/* Round up N to the next multiple of S */
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

/* Mark binary constants as known GCC extension to suppress warning */
#define GCC_BINARY(x)   (__extension__ (x))

#define _token_concat(x,y) x##y
#define token_concat(x,y) _token_concat(x,y)

#define nibble_to_hex(i) ((i) <= 9 ? '0' + (i) : 'A' - 10 + (i))

#define ITERATOR_DEF()    uint8_t iterator = 0
#define ITERATOR_VAR()    iterator < 1 ? "/" : iterator < 2 ? "-" : "\\"
#define ITERATOR_INC()    iterator = iterator < 2 ? iterator + 1 : 0

#define debugWriteChar(c)  sdPut(&SD3, c)
#define debugWriteStr(s)  sdWrite(&SD3, (uint8_t *)s, sizeof(s))
#define debugPrintf(...) chprintf((BaseSequentialStream*)&SD3, __VA_ARGS__)

#define IP4_ADDR_VALUE(a,b,c,d)        \
        (((u32_t)((d) & 0xff) << 24) | \
         ((u32_t)((c) & 0xff) << 16) | \
         ((u32_t)((b) & 0xff) << 8)  | \
          (u32_t)((a) & 0xff))

#define chprintf_ip4_addr(_chp, u32_addr)   chprintf(_chp, "%d.%d.%d.%d", (u32_addr & 0xff), ((u32_addr >> 8) & 0xff), ((u32_addr >> 16) & 0xff), ((u32_addr >> 24) & 0xff))

#include "watchdog.h"
#include "random.h"
#include "ip_link.h"
#include "contacts.h"
#include "environmentals.h"
#include "config.h"
#include "web/web.h"
#include "shell/shell.h"

#endif /* __MAIN_H__ */