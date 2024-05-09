start	lw	0	1	five
	lw	1	0	8
	sw	0	5	five
calc	add	0	1	2
	lw	2	3	neg1
	nor	2	3	0
	noop
	jalr	1	2
	jalr	0	0
	beq	0	0	neg1
neg1	.fill	-1
five	.fill	5
