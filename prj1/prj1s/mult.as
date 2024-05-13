	lw	0	3	sixt
	lw	0	4	one
	lw	0	6	one
	lw	0	1	mplier
	lw	0	7	mcand
loop	beq	2	3	end
	nor	1	1	1	nor mplier
	nor	4	4	6	nor dig
	nor	1	6	3	the nor operations
	lw	0	1	mplier	restore the val of mplier
	lw	0	6	one	restore value of reg6 to one
if	beq	4	3	addcur
endif	add	4	4	4	dig<<1
	add	7	7	7	cur<<1
	lw	0	3	sixt	reload values for registers
	add	2	6	2	increment 2
	beq	2	2	loop
end	add	0	5	1
	halt
addcur	add	7	5	5
	beq	2	2	endif
mcand	.fill	6203			0b1100000111011
mplier	.fill	1429			0b10110010101
zero	.fill	0
one	.fill	1
neg1	.fill	-1
sixt	.fill	4
