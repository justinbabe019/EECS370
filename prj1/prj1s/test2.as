	lw	0	1	count
	lw	1	2	0
	lw	2	3	incre
	lw	3	4	array
	add	5	6	7
	add	5	2	2
	add	1	1	1
	nor	1	0	1
loop	add	1	2	1
	add	3	2	1
	beq	1	3	done
	beq	0	0	loop
	jalr	4	5
	noop
	sw	5	4	0
	noop
done	halt
count	.fill	0
incre	.fill	5
array	.fill	0
