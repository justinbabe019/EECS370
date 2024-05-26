	lw	0	1	five
	lw	3	4	Array
	lw	1	2	neg1
	sw	3	4	SubAdr
Start	add	1	2	1
	beq	0	1	done
	beq	0	0	Start
	beq	0	0	Jump
NOOP	noop
done	halt
	lw	4	5	0
Add	add	5	5	1
	sw	3	5	Do
Halt	halt
Jump	noop
	beq	0	0	NOOP
array	.fill	10
five	.fill	5
SubAdr	.fill	8
IDK	.fill	4
