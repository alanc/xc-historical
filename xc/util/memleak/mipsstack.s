	.globl	getReturnAddress
	.ent	getReturnAddress
getReturnAddress:
	.frame	$sp, 0, $31
	move	$2,$31
	j	$31
	.end	getReturnAddress

	.globl	getStackPointer
	.ent	getStackPointer
getStackPointer:
	.frame	$sp, 0, $31
	move	$2,$29
	j	$31
	.end	getStackPointer
