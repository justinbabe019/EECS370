	lw	0	1	neg1
	add	0	1	1
	nor	0	1	1
	beq	1	1	here
	add	1	1	1
here	noop
	sw	0	1	5
	lw	0	2	neg1
	add	0	1	2
	add	2	1	3
	add	2	3	5
	add	0	1	2
	add	2	1	3
	add	1	1	2
	add	0	1	2
	noop
	add	2	1	3
	add	1	1	2
	add	0	1	2
	noop
	noop
	add	2	1	3
	add	1	1	2
	add	0	1	2
	noop
	noop
	noop
	add	0	1	2
	lw	0	1	neg1
	lw	0	2	neg2
	beq	1	2	end
end	halt
neg1	.fill	-1
neg2	.fill	-2
