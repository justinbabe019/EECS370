	lw	0	0	zero	reg 0 = 1 = digcount
	lw	0	1	sixt	reg 1 = 16
	lw	0	4	one	reg 4 = 1 = incre
	lw	0	5	one	reg 5 is the 1 in diff digits that nors b
	lw	0	2	mcand
	lw	0	3	mplier	shifting by one is adding same n twice
	lw	0	7	zero	reg 7 is the current total
loop	beq	0	1	end	0 and 1 r conditions of loop, #oper=#digit
	nor	5	3	6	if reg 6 is 1, right most digit of b is one
	add	4	5	5	leftshift one for reg 5
	beq	6	4	addcur	if reg6 is one, adds current to total
	add	4	0	0	digcount += 1
end	halt
addcur	add	2	7	7
sw	
mcand	.fill	6203
mplier	.fill	1429
zero	.fill	0
one	.fill	1
sixt	.fill	16
