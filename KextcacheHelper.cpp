/*
 *  kextcacheHelper.cpp
 *  kextcacheHelper
 *
 *  Created by Meklort on 10/21/10.
 *  Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */
#include "kextcacheHelper.h"
#include "loader.h"

#define super IOService
OSDefineMetaClassAndStructors(KextcacheHelper, IOService);


static int (*KextcacheHelper_handler)(struct image_params *imgp);


bool KextcacheHelper::start(IOService *provider)
{
	// Replace first img handler with our own, that way we are run first.
	KextcacheHelper_handler = execsw->ex_imgact;
	execsw->ex_imgact = &KextcacheHelper::exec_KextcacheHelper_imgact;

	return true;
}

void KextcacheHelper::stop()
{
	execsw->ex_imgact = KextcacheHelper_handler;	// Put old handler back
}	

void KextcacheHelper::free( void )
{
	execsw->ex_imgact = KextcacheHelper_handler;	// Put old handler back

    super::free();
}


int KextcacheHelper::exec_KextcacheHelper_imgact(struct image_params *imgp)
{
	bool kextcache = false;
	bool loop = true;
	size_t len;
	char temp[256];
	
	user_addr_t argv = (imgp->ip_user_argv);
	int		ptr_size = (imgp->ip_flags & IMGPF_WAS_64BIT) ? 8 : 4;
	
	
	int shift = ptr_size;
	int index = 0;
	while (loop && argv != 0LL)
	{
		user_addr_t     arg;
		
		if(!copy_in_ptr(argv, &arg, ptr_size))
		{
			argv += ptr_size;
			
			if(!index)
			{
				if(!copyinstr(arg, temp, sizeof(temp), &len))
				{
					if((strlen(temp) == strlen("kextcache")) && // todo: sizeof - 1
					   (strcmp(temp, "kextcache") == 0))
					{
						// arg0 = kextcache
						kextcache = true;
					}
					else if(strcmp(temp + strlen(temp) - strlen("/kextcache"), "/kextcache") == 0)
					{
						// arg0 = xxx/kextcache (ex: /usr/sbin/kextcache)
						kextcache = true;
					}
					else
					{
						// this program is not kextcache
						// debug
						/*printf("Ignoring executable (%s); ", temp);
						printf("strcmp(%s, %s) == %d; ", temp, 
							   "kextcache", 
							   strcmp(temp, "kextcache")
							   );
						printf("strcmp(%s, %s) == %d; ", temp + strlen(temp) - strlen("/kextcache"),
							   "/kextcache",
							   strcmp(temp + strlen(temp) - strlen("/kextcache"), "/kextcache")
							   );
						 */
					}
				}
				else
				{
					loop = false;;
				}
			}
			else
			{
				if(!copyinstr(arg, temp, sizeof(temp), &len))
				{
					//printf(" %s", temp);
					
					if(kextcache && (
									 strncmp(temp, "-l", strlen("-l")) == 0 ||
									 strncmp(temp, "-local-root", strlen("-local-root")) == 0
									 // TODO: check if -local-root-all should be striped (and -L)
									 ))
					{
						//printf("Shifting... %d\n", ptr_size);
						shift += ptr_size;
						// Located -l, remove it;
						// Shift pointer
					}
					else
					{
						if(copy_out_ptr(arg, (user_addr_t)argv-shift, ptr_size))
						{
							loop = false;
						}						
					}
				}
				else
				{
					loop = false;
				}
			}
			
		}
		else 
		{
			loop = false;	
		}
		index++;
	}   
	//printf("\n");
	// Copy out null pointer
	if(kextcache) 
	{
		copy_out_ptr(USER_ADDR_NULL, (user_addr_t)argv-shift, ptr_size);
	}
	
	// We've modified the arguments that would be passed to the program. Now go an run the handler we overrode.
	// NOTE: we *could* have just tested macho file with each image handlers, and only replaced that one
	// and *then* only replace the file arguments if it's a macho file (this way it's not replaced on scripts).
	if(KextcacheHelper_handler) return (*KextcacheHelper_handler)(imgp);
	return -2;
}


// Copied from kern_exec.c
int KextcacheHelper::copy_in_ptr(user_addr_t froma, user_addr_t *toptr, int ptr_size)
{
	int error;
	
	if (ptr_size == 4) {
		/* 64 bit value containing 32 bit address */
		unsigned int i;
		
		error = copyin(froma, &i, 4);
		*toptr = CAST_USER_ADDR_T(i);	/* SAFE */
	} else {
		error = copyin(froma, toptr, 8);
	}
	return (error);
}

int KextcacheHelper::copy_out_ptr(user_addr_t ua, user_addr_t ptr, int ptr_size)
{
	int error;
	
	if (ptr_size == 4) {
		/* 64 bit value containing 32 bit address */
		unsigned int i = CAST_DOWN_EXPLICIT(unsigned int,ua);	/* SAFE */
		
		error = copyout(&i, ptr, 4);
	} else {
		error = copyout(&ua, ptr, 8);
	}
	return (error);
}