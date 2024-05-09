	lw	0	1	count
	lw	1	1	count
	lw	2	3	incre
	lw	3	4	array
loop	add	1	2	1
	nor	3	2	1
	beq	1	3	done
	beq	0	0	loop
	jalr	4	5
	noop
	sw	5	4	0
	noop
done	halt
count	.fill	1
array	.fill	count
incre	.fill	5
