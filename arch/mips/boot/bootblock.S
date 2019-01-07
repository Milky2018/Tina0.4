.text
.global main

main:
	la $a0, msg
	jal 0x8007b980
	li $t0, 0xa08001e0
	lw $a2, 0($t0)
	li $t1, 0x200
	mul $a2, $a2, $t1
	li $a0, 0xa0800200 
	li $a1, 0x200 
	jal 0x8007b1cc
	j 0xa0800200
stop:
	j stop
.data

msg: .ascii "It's a bootloader...\n"

read_sd_card: .word 0x8007b1cc

printstr: .word 0x8007b980

printch: .word 0x8007ba00

kernel : .word 0xa0800200

kernel_main : .word 0xa0800200
