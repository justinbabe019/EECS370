	lw	0	1	ArrayS
	lw	0	2	Num
	lw	0	3	Fcall
	jalr	3	7
Find	lw	0	6	NegOne	// r6=-1
	add	0	2	5	// r5=Num
Top	add	2	6	2	// Decrement Num
	add	1	2	4	// 4 is address
	lw	4	3	0	// ld array element
	beq	3	0	Skip	// is  element 0?
	halt
	add	5	6	5	// if not sub 1
Skip	beq	0	2	Done	// if Num=0 we are done
	beq	0	0	Top	// next iteration
Done	jalr	7	3		// return
Array	.fill	0
NegOne	.fill	-1
Fcall	.fill	Find
ArrayS	.fill	Array
Num	.fill	8
	.fill	1
	.fill	2222
	.fill	777
	.fill	-5
