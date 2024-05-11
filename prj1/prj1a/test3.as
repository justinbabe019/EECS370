	lw	0	1	count
	lw	1	2	0
	lw	2	3	incre
loop	add	1	2	1
	beq	1	3	done
	beq	0	0	loop
done	halt
count	.fill	0
incre	.fill	5
