/* Copyright (c) 2011 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse
     or promote products derived from this software without specific
     prior written permission.

   This software is provided by the copyright holders and contributors
   "AS IS" and any express or implied warranties, including, but not
   limited to, the implied warranties of merchantability and fitness for
   a particular purpose are disclaimed.  In no event shall Red Hat
   incorporated be liable for any direct, indirect, incidental, special,
   exemplary, or consequential damages (including, but not limited to,
   procurement of substitute goods or services; loss of use, data, or
   profits; or business interruption) however caused and on any theory of
   liability, whether in contract, strict liability, or tort (including
   negligence or otherwise) arising in any way out of the use of this
   software, even if advised of the possibility of such damage.  */


	.section ".vec","a"
	.short	_PowerON_Reset

	.section ".ivec","a"
#define IV(x) .weak _##x##_handler | .short _##x##_handler
#define IVx() .short 0

	.section ".csstart", "ax"
	.global __csstart

__csstart:
	br	!!_PowerON_Reset
	.text

	.global _PowerON_Reset
	.type	_PowerON_Reset, @function
_PowerON_Reset:
	movw	sp, #__stack


;; block move to initialize .data

	;; we're copying from 00:[_romdatastart] to 0F:[_datastart]
	;; and our data is not in the mirrored area.
	mov	es, #0

	sel	rb0		; bank 0
	movw	hl, #__datastart
	movw	de, #__romdatastart
	sel	rb1		; bank 1
	movw	ax, #__romdatacopysize
	shrw	ax,1
1:
	cmpw	ax, #0
	bz	$1f
	decw	ax
	sel	rb0		; bank 0
	movw	ax, es:[de]
	movw	[hl], ax
	incw	de
	incw	de
	incw	hl
	incw	hl
	sel	rb1
	br	$1b
1:
	sel	rb0		; bank 0


;; block fill to .bss

	sel	rb0		; bank 0
	movw	hl, #__bssstart
	movw	ax, #0
	sel	rb1		; bank 1
	movw	ax, #__bsssize
	shrw	ax,1
1:
	cmpw	ax, #0
	bz	$1f
	decw	ax
	sel	rb0		; bank 0
	movw	[hl], ax
	incw	hl
	incw	hl
	sel	rb1
	br	$1b
1:
	sel	rb0		; bank 0


/* call the hardware initialiser */
	call	!!_HardwareSetup
	nop

	call	!!__rl78_init

#ifdef PROFILE_SUPPORT	/* Defined in gcrt0.S.  */
	movw	ax, # _start
	push	ax
	movw	ax, # _etext
	push	ax
	call	!!__monstartup
#endif


/* start user program */

	movw	ax, #0
	push	ax	/* envp */
	push	ax	/* argv */
	push	ax	/* argc */
	call	!!_main
.LFE2:

	push	ax

#ifdef PROFILE_SUPPORT
	call	!!__mcleanup
#endif

	call	!!_exit

	.size	_PowerON_Reset, . - _PowerON_Reset

	.global	_rl78_run_preinit_array
	.type	_rl78_run_preinit_array,@function
_rl78_run_preinit_array:
	movw	hl, #__preinit_array_start
	movw	de, #__preinit_array_end
	movw	bc, #-2
	br	$_rl78_run_inilist

	.global	_rl78_run_init_array
	.type	_rl78_run_init_array,@function
_rl78_run_init_array:
	movw	hl, #__init_array_start
	movw	de, #__init_array_end
	movw	bc, #2
	br	$_rl78_run_inilist

	.global	_rl78_run_fini_array
	.type	_rl78_run_fini_array,@function
_rl78_run_fini_array:
	movw	hl, #__fini_array_start
	movw	de, #__fini_array_end
	movw	bc, #-2
	/* fall through */

	;; HL = start of list
	;; DE = end of list
	;; BC = step direction (+2 or -2)
_rl78_run_inilist:
next_inilist:
	movw	ax, hl
	cmpw	ax, de
	bz	$done_inilist
	movw	ax, [hl]
	cmpw	ax, #-1
	bz	$skip_inilist
	cmpw	ax, #0
	bz	$skip_inilist
	push	ax
	push	bc
	push	de
	push	hl
	call	ax
	pop	hl
	pop	de
	pop	bc
	pop	ax
skip_inilist:
	movw	ax, hl
	addw	ax, bc
	movw	hl, ax
	br	$next_inilist
done_inilist:
	ret

	.section	.init,"ax"

	.global __rl78_init
__rl78_init:

	.section	.fini,"ax"

	.global __rl78_fini
__rl78_fini:
	call	!!_rl78_run_fini_array

	.section .data
	.global ___dso_handle
	.weak   ___dso_handle
___dso_handle:
	.long	0

	.text

