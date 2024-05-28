	beq	0	0	jump
	noop
done	halt
Noop	noop
jump	noop
	beq	1	1	done
	beq	2	2	Noop
	beq	3	3	T
T	.fill	0
