! $XConsortium: sparcstack.s,v 1.2 92/04/08 17:19:18 keith Exp $
	.seg	"text"
	.proc	16
	.globl	getStackPointer
getStackPointer:
	retl
	mov	%sp,%o0
	.globl	getFramePointer
getFramePointer:
	retl
	mov	%fp,%o0
