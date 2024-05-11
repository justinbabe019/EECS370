start	lw	0	1	four	reg1=4
	lw	0	0	6	reg0=1
	lw	2	2	ran
	add	0	1	1	reg1=5
loop	add	0	1	0	reg0=reg0+reg1
	nor	0	2	2
	noop
	beq	0	1	done
done	halt
	jalr	0	2
four	.fill	4
one	.fill	1
ran	.fill	four
