zero	lw	0	1	neg
	lw	1	2	neg2
	lw	2	1	three
	jalr	1	1
	beq	0	1	neg
	beq	2	2	0
	nor	2	1	3
	noop
	nor	4	4	4
	sw	3	1	neg
	beq	4	1	neg
	halt
neg	.fill	-1
neg2	.fill	-2
three	.fill	3
