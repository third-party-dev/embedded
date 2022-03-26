	.arch armv8-a+crc
	.file	"main.c"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB0:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	mov	x29, sp
	str	x19, [sp, 16]
	.cfi_offset 19, -32
	str	wzr, [sp, 44]
	bl	fee
	mov	w1, w0
	mov	w0, 43981
	add	w19, w1, w0
	bl	fie
	add	w19, w19, w0
	bl	foe
	add	w0, w19, w0
	str	w0, [sp, 44]
	ldr	w0, [sp, 44]
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 19
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Buildroot 2021.08.1) 10.3.0"
	.section	.note.GNU-stack,"",@progbits
