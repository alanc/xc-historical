/* reordering instructions would be fatal here */
	.set	noreorder
/*
 * This macro uses registers:
 *	2	temp
 *	8	switch table base, must contain the address of $label
 *	9	mask value 0xfffffff0
 */
/* variables */

#define SBase		$8
#define SMask		$9
#define STemp		$2
#define CatComma(a,b)	a, b

#define StippleCases(addr,value,done,do_l1,do_l2,o)	\
	do_l2			/* 0 */	;\
	nop				;\
	done				;\
	nop				;\
					\
	do_l2			/* 1 */	;\
	sb	value, o+0(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* 2 */	;\
	sb	value, o+1(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* 3 */	;\
	sh	value, o+0(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* 4 */	;\
	sb	value, o+2(addr)	;\
	done				;\
	nop				;\
					\
	sb	value, o+0(addr)/* 5 */	;\
	do_l1				;\
	sb	value, o+2(addr)	;\
	nop				;\
					\
	sb	value, o+1(addr)/* 6 */	;\
	do_l1				;\
	sb	value, o+2(addr)	;\
	nop				;\
					\
	do_l2			/* 7 */	;\
	swl	value, o+2(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* 8 */	;\
	sb	value, o+3(addr)	;\
	done				;\
	nop				;\
					\
	sb	value, o+0(addr)/* 9 */	;\
	do_l1				;\
	sb	value, o+3(addr)	;\
	nop				;\
					\
	sb	value, o+1(addr)/* a */	;\
	do_l1				;\
	sb	value, o+3(addr)	;\
	nop				;\
					\
	sh	value, o+0(addr)/* b */	;\
	do_l1				;\
	sb	value, o+3(addr)	;\
	nop				;\
					\
	do_l2			/* c */	;\
	sh	value, o+2(addr)	;\
	done				;\
	nop				;\
					\
	sb	value, o+0(addr)/* d */	;\
	do_l1				;\
	sh	value, o+2(addr)	;\
	nop				;\
					\
	do_l2			/* e */	;\
	swr	value, o+1(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* f */	;\
	sw	value, o+0(addr)	;\
	done				;\
	nop

#define Stipple(addr,bits,value,label,done,l1,l2)	\
	and	STemp, bits, 15		;\
	sll	STemp, STemp, 4		;\
	addu	STemp, STemp, SBase	;\
	j	STemp			;\
	and	bits, bits, SMask	;\
$l1:					;\
	bnez	bits,$l2		;\
	nop				;\
	done				;\
	nop				;\
$l2:					;\
	addu	addr, addr, 4		;\
	and	STemp, bits, 0xf0	;\
	addu	STemp, STemp, SBase	;\
	srl	bits, bits, 4		;\
	j	STemp			;\
	and	bits, bits, SMask	;\
$label:					;\
	StippleCases(addr,value,done,b $l1,CatComma(bnez bits, $l2),0)

	.text	
	.align	2
/*
 * stippleone(addr, bits, value)
 *             4     5     6
 */
/* arguments */
#define addr	$4
#define bits	$5
#define value	$6

	.globl	stippleone
	.ent	stippleone 2
stippleone:
	.frame	$sp, 0, $31
	la	SBase, $100
	li	SMask, 0xfffffff0
	Stipple(addr,bits,value,100,j $31,101,102)
	j	$31
	nop
	.end	stippleone
#undef addr
#undef bits
#undef value

/*
 * stipplestack(addr, stipple, value, stride, count, shift)
 *               4       5       6      7     16(sp) 20(sp)
 *
 *  Apply successive 32-bit stipples starting at addr, addr+stride, ...
 *
 */
/* arguments */
#define addr	$4
#define stipple	$5
#define value	$6
#define stride	$7
#define Count	16($sp)
#define Shift	20($sp)

/* local variables */
#define count	$14
#define shift	$13
#define atemp	$12
#define bits	$11

	.globl	stipplestack
	.ent	stipplestack 2
stipplestack:
	.frame	$sp, 0, $31
	lw	count, Count
	la	SBase,$201
	lw	shift, Shift
	li	SMask,0xfffffff0
$200:
	lw	bits, 0(stipple)
	move	atemp, addr
	sll	bits, bits, shift
	Stipple(atemp,bits,value,201,b $204,202,203)
$204:
	addu	count, count, -1
	addu	addr, addr, stride
	bnez	count, $200
	addu	stipple, stipple, 4
	j	$31

	.end	stipplestack

/*
 * Used when the stipple is > 28 pixels wide to avoid troubles with the
 * shift
 */

#define tbits	$10
#define TBase	$3
#define mshift	$15

	.globl	stipplestackwide
	.ent	stipplestackwide 2
stipplestackwide:
	.frame	$sp, 0, $31
	lw	count, Count
	la	SBase,$251
	la	TBase, $260
	lw	shift, Shift
	li	SMask,0xfffffff0
	li	mshift, 28
	subu	mshift, mshift, shift
$250:
	lw	bits, 0(stipple)
	move	atemp, addr
	sll	tbits, bits, shift
	Stipple(atemp,tbits,value,251,b $254,252,253)
$254:
	srl	tbits, bits, mshift
	and	tbits, tbits, 0xf0
	addu	STemp, TBase, tbits
	j	STemp
	addu	count, count, -1
$260:
	StippleCases(addr,value,nop,b $264,b $264,28)
$264:
	addu	addr, addr, stride
	bnez	count, $250
	addu	stipple, stipple, 4
	j	$31

	.end	stipplestackwide

#undef addr
#undef stipple
#undef value
#undef stride
#undef Count
#undef Shift
#undef count
#undef shift
#undef atemp
#undef bits

/*
 * stipplespan32(addr,bits,value, leftmask,rightmask,nlw)
 *                $4   $5    $6       $7     16($sp) 20($sp) 		
 *
 * Fill a span with bits from a 32-bit stipple.
 */
/* arguments */
#define addr		$4
#define bits		$5
#define value		$6
#define leftmask	$7
#define Rightmask	16($sp)
#define Nlw		20($sp)
/* local variables */
#define rightmask	$14
#define nlw		$13
#define btemp		$12
#define atemp		$11

	.globl	stipplespan32
	.ent	stipplespan32 2
stipplespan32:
	.frame	$sp, 0, $31

	lw	nlw,20($sp)
	la	SBase, $402
	li	SMask, 0xfffffff0

	/*
 	 * Compute left edge stipple bits and
 	 * jump into the middle of the loop
	 */

	beqz	leftmask,$405
	and	btemp,bits,leftmask
	b	$401
	nop
	/* while (nlw--) */
$400:
	move	btemp,bits
$401:
	move	atemp, addr
	addu	addr, addr, 32
	Stipple(atemp,btemp,value,402,b $405,403,404)
$405:
	bnez	nlw,$400
	addu	nlw,nlw,-1

	/* And do the right edge as well */
	lw	rightmask,Rightmask
	la	SBase, $406
	and	btemp, bits, rightmask

	Stipple(addr,btemp,value,406,j $31,407,408)

	j	$31
	nop
	.end	stipplespan32

#undef addr
#undef bits
#undef value
#undef leftmask
#undef Rightmask
#undef Nlw
#undef rightmask
#undef nlw
#undef btemp
#undef atemp
