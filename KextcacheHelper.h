/*
 *  kextcacheHelper.h
 *  kextcacheHelper
 *
 *  Created by Meklort on 10/21/10.
 *  Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */
#ifndef _KEXTCACHE_HELPER_H
#define _KEXTCACHE_HELPER_H

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

extern "C" struct execsw {
	int (*ex_imgact)(struct image_params *);
	const char *ex_name;
} execsw[]; 

class KextcacheHelper : public IOService
{
	OSDeclareDefaultStructors(KextcacheHelper)
	

private:
	static int		exec_KextcacheHelper_imgact(struct image_params *imgp);
	static int		copy_in_ptr(user_addr_t froma, user_addr_t *toptr, int ptr_size);
	static int		copy_out_ptr(user_addr_t fromptr, user_addr_t toma, int ptr_size);

public:
	virtual bool	start(IOService *provider);
	virtual void	stop();
	virtual void	free();

};
#endif /* _KEXTCACHE_HELPER_H */