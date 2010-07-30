#include <sys/stat.h>
#include <sys/utsname.h>

#include <ctype.h>
#include <dirent.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <aros/system.h>
#include <aros/kernel.h>
#include <aros/multiboot.h>
#include <utility/tagitem.h>

#include "debug.h"
#include "elfloader32.h"
#include "hostlib.h"
#include "shutdown.h"
#include "../kernel/hostinterface.h"

#define D(x)

static unsigned char __bss_track[32768];
char bootstrapdir[PATH_MAX];
char **Kernel_ArgV;
static char buf[256];

typedef int (*kernel_entry_fun_t)(struct TagItem *);

struct mb_mmap MemoryMap = {
    sizeof(struct mb_mmap),
    0,
    0,
    0,
    0,
    MMAP_TYPE_RAM
};

/*
 * Some helpful functions that link us to the underlying host OS.
 * Without them we would not be able to estabilish any interaction with it.
 */
struct HostInterface HostIFace = {
    Host_HostLib_Open,
    Host_HostLib_Close,
    Host_HostLib_GetPointer,
    Host_HostLib_GetErrorStr,
    Host_HostLib_GetInterface,
    Host_VKPrintF,
    Host_PutChar,
    Host_Shutdown,
    Host_Alert
};

/* Kernel message */
static struct TagItem km[] = {
    {KRN_KernelLowest , 0                  },
    {KRN_KernelHighest, 0                  },
    {KRN_CmdLine      , 0                  },
    {KRN_BootLoader   , 0                  },
    {KRN_MMAPAddress  , (IPTR)&MemoryMap   },
    {KRN_MMAPLength   , sizeof(MemoryMap)  },
    {KRN_KernelBss    , (IPTR)__bss_track  },
    {KRN_HostInterface, (IPTR)&HostIFace   },
    {TAG_DONE         , 0                  }
};

/* ***** This is the global SysBase ***** */
void *SysBase;

static char *GetConfigArg(char *str, char *option)
{
    size_t l = strlen(option);

    /* First check option name */
    if (strncasecmp(str, option, l))
        return NULL;

    /* Skip option name */
    str += l;

    /* First character must be space */
    if (!isspace(*str++))
        return NULL;
    /* Skip the rest of spaces */
    while(isspace(*str))
	str++;

    return str;
}

static char *join_string(int argc, char **argv)
{
    char *str, *s;
    int j;
    int x = 0;

    for (j = 0; j < argc; j++)
	x += (strlen(argv[j]) + 1);
    D(printf("[Init] Allocating %lu bytes for string\n", x));
    str = malloc(x);
    if (str) {
	s = str;
	for (j = 0; j < argc; j++) {
	    strcpy(s, argv[j]);
	    s += strlen(s);
	    *s++ = ' ';
	}
	s[-1] = 0;
	D(printf("[Init] Joined line: %s\n", str));
    }
    return str;
}

int main(int argc, char ** argv)
{
    char *error;
    unsigned long BadSyms;
    struct TagItem *t;
    int x;
    struct stat st;
    int i = 1;
    unsigned int memSize = 64;
    int def_memSize = 1;
    char *config = "boot/AROSBootstrap.conf";
    char *SystemVersion;
    char *KernelArgs = NULL;
    struct utsname sysinfo;
    char *nameparts[4];

    FILE *file;
    kernel_entry_fun_t kernel_addr;
    size_t kernel_size;

    /* Set current locale. This makes national characters to be output
       properly into the debug log */
    setlocale(LC_ALL, "");
    getcwd(bootstrapdir, sizeof(bootstrapdir));
    Kernel_ArgV = argv;

    while (i < argc) {
	if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            printf ("AROS for Windows\n"
		    "usage: %s [options] [kernel arguments]\n"
		    "Availible options:\n"
		    " -h                 show this page\n"
		    " -m <size>          allocate <size> Megabytes of memory for AROS\n"
		    "                    (default is 64M)\n"
		    " -c <file>          read configuration from <file>\n"
		    "                    (default is boot\\AROSBootstrap.conf)\n"
		    " --help             same as '-h'\n"
		    " --memsize <size>   same as '-m <size>'\n"
		    " --kernel <file>    same as '-k'\n"
		    "\n"
		    "Please report bugs to the AROS development team. http://www.aros.org/\n", argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "--memsize") || !strcmp(argv[i], "-m")) {
	    memSize = atoi(argv[++i]);
	    def_memSize = 0;
	    i++;
	} else if (!strcmp(argv[i], "--kernel") || !strcmp(argv[i], "-c")) {
	    config = argv[++i];
            i++;
        } else
	    break;
    }
    D(printf("[Bootstrap] %ld arguments processed\n", i));

    if (i < argc) {
	KernelArgs = join_string(argc - i, &argv[i]);
	D(printf("[Bootstrap] Kernel arguments: %s\n", KernelArgs));
    }

    uname(&sysinfo);
    nameparts[0] = sysinfo.sysname;
    nameparts[1] = sysinfo.machine;
    nameparts[2] = sysinfo.release;
    nameparts[3] = sysinfo.version;
    SystemVersion = join_string(4, nameparts);
    D(printf("[Bootstrap] OS version: %s\n", SystemVersion));

    /* If AROSBootstrap.exe is found in the current directory, this means the bootstrap
       was started in its own dir. Go one level up in order to reach the root */
    if (!stat("AROSBootstrap.exe", &st))
	chdir("..");

    file = fopen(config, "r");
    if (!file) {
	printf("Failed to load configuration file %s!\n", config);
	return -1;
    }

    /* Parse the configuration file */
    while (fgets(buf, sizeof(buf), file)) {
	char *c = strchr(buf, '\r');
      
	if (!c)
            c = strchr(buf, '\n');
	if (c)
	    *c = 0;

	c = GetConfigArg(buf, "module");
	if (c) {
	    AddKernelFile(c);
	    continue;
	}

	/* Command line argument overrides this */
	if (def_memSize) {
	    c = GetConfigArg(buf, "memory");
	    if (c) {
		memSize = atoi(c);
		continue;
	    }
	}
    }
    fclose(file);

    kernel_size = GetKernelSize();
    if (!kernel_size)
	return -1;

    kernel_addr = malloc(kernel_size);
    if (!kernel_addr) {
	printf("Failed to allocate %u bytes for the kernel!\n", kernel_size);
	return -1;
    }

    set_base_address(kernel_addr, __bss_track, &SysBase);
    if (!LoadKernel())
	return -1;

    FreeKernelList();

    D(printf("[Bootstrap] allocating working mem: %iMb\n",memSize));

    size_t memlen = memSize << 20;
    void * memory = malloc(memlen);

    if (!memory) {
	printf("[Bootstrap] Failed to allocate %i Mb of RAM for AROS!\n", memSize);
	return -1;
    }
    D(printf("[Bootstrap] RAM memory allocated: %p-%p (%lu bytes)\n", memory, memory + memlen, memlen));

    MemoryMap.addr = (IPTR)memory;
    MemoryMap.len  = memlen;

    km[0].ti_Data = (IPTR)kernel_addr;
    km[1].ti_Data = (IPTR)kernel_highest();
    km[2].ti_Data = (IPTR)KernelArgs;
    km[3].ti_Data = (IPTR)SystemVersion;

    printf("[Bootstrap] entering kernel@%p...\n", kernel_addr);
    int retval = kernel_addr(km);

    printf("kernel returned %i\n",retval);
    return retval;
}  
