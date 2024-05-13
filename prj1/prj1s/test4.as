start	lw	0	1	bad
	lw	1	2	bad
	sw	2	2	start
	add	0	1	0
	nor	0	1	0
	beq	0	1	bruh
bruh	beq	1	1	bruh2
	jalr	0	1
bruh2	beq	2	1	bruh3
bruh3	noop
	halt
one	.fill	1
bad	.fill	-9999
idk	.fill	start
