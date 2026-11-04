/* _Unwind_Frames_Extra with software shadow stack.
   Copyright (C) 2016-2026 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

#define LIBGCC2_UNITS_PER_WORD (__riscv_xlen / 8)

/* Unwind the shadow stack for EH.  */
#undef _Unwind_Frames_Extra
#define _Unwind_Frames_Extra(x)					\
    {								\
      _Unwind_Word gp = 0;					\
      asm volatile ("mv %0, gp" : "=r"(gp));			\
      if (gp != 0)						\
	{							\
          gp -= (_Unwind_Word) (x) * LIBGCC2_UNITS_PER_WORD;	\
          asm volatile ("mv gp, %0" : : "r"(gp));		\
	}							\
    }

/* Here, FRAMES represents the number of frames to unwind. This macro is
   included in a loop which runs for however many frames we need to unwind.

   We first unwind one frame, check if the ra matches the one stored on top
   of the shadow stack, then two, and so on; until we have unwound until the
   FDE with the matching catch block.

   Since this unwinder itself is also instrumented using shadow call stack,
   the functions _Unwind_RaiseException and _Unwind_RaiseException_Phase2
   have their return addresses stored on the gp, so we must look back by 2
   extra frames each loop for the return address.

   e.g. For the first pass, the context passed to this function is that of
   __cxa_throw, so context->ra points to the function that called __cxa_throw.

   So, gp looks like follows:
   [top of shadow stack]
   ra for _Unwind_RaiseException_Phase2
   ra for _Unwind_RaiseException
   ra for __cxa_throw     <-- context->ra should match with this
   ra for function that called __cxa_throw

   So during validation, we can't check for gp-4, we should check gp-4-2*(4).  */

#undef _Unwind_Frames_Increment
#define _Unwind_Frames_Increment(exc, context, frames)		\
    {								\
      if (exc->exception_class != 0				\
	  && _Unwind_GetIP (context) != 0			\
	  && !_Unwind_IsSignalFrame (context))			\
	{							\
	  _Unwind_Word gp;					\
	  asm volatile ("mv %0, gp" : "=r"(gp));		\
	  if (gp != 0)						\
	    {							\
	      gp -= LIBGCC2_UNITS_PER_WORD * (frames + 2);	\
	      _Unwind_Word ra = *(_Unwind_Word *) gp;		\
	      if (ra != _Unwind_GetIP (context))		\
		return _URC_FATAL_PHASE2_ERROR;			\
	    }							\
	}							\
      frames++;							\
    }
