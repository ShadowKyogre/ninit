#ifdef INIT_SYSTEM
#define set_sa(a) system_set_sigaction(a)
#else
#define set_sa(a) set_sigaction(a)
#endif

#if defined(INIT_SYSTEM) && defined(__i386__)
typedef unsigned short gid__t;
extern int SYS_mknod(const char *path, gid__t mode, gid__t dev);
extern int SYS_chown(const char *path, gid__t owner, gid__t group);
extern int SYS_setgroups(size_t size, const gid__t *list);
#else
typedef gid_t gid__t;
#define SYS_chown	chown
#define SYS_mknod	mknod
#define SYS_setgroups	setgroups
#endif

#if defined(__linux__) && defined(INIT_SYSTEM)
/* LINUX_REBOOT_MAGIC1	0xfee1dead
   LINUX_REBOOT_MAGIC2	672274793 */
extern int SYS_reboot(int magic, int magic2, int cmd);
#define set_reboot(cmd) SYS_reboot(0xfee1dead, 672274793, cmd)
#else
#define set_reboot	reboot
#endif
