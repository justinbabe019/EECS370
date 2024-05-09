	lw	0	1	five
	lw	1	2	neg1
start	add	1	2	1
	beq	0	1	done
	beq	0	0	start
	noop
done	halt
five	.fill	5
neg1	.fill	-1
stAddr	.fill	start
	lw	3	4	array
	lw	4	5	0
	add	5	5	1
	sw	3	5	0
	halt
array	.fill	10
