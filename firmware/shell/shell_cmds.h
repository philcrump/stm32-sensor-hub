#ifndef __SHELL_CMDS_H__
#define __SHELL_CMDS_H__

void shell_cmd_netconfig(BaseSequentialStream *chp, int argc, char *argv[]);
void shell_cmd_resetconfig(BaseSequentialStream *chp, int argc, char *argv[]);
void shell_cmd_reboot(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* __SHELL_CMDS_H__ */