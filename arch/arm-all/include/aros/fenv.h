/*
    Copyright (C) 1995-2011, The AROS Development Team. All rights reserved.

    Desc: FPU-specific definitions for ARM processors
*/

#ifndef	_FENV_H_
#define	_FENV_H_

#ifdef __SOFTFP__
#include <aros/arm/fenv_soft.h>
#else
#include <aros/arm/fenv_vfp.h>
#endif

#endif	/* !_FENV_H_ */
