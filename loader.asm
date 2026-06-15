.set MAGIC,    0x1BADB002
.set FLAGS,    0x00000003
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .text
.global loader
.extern kmain

loader:
    mov $kernel_stack, %esp
    push %eax
    push %ebx
    call kmain

_stop:
    cli
    hlt
    jmp _stop

.section .note.GNU-stack,"",@progbits

.section .bss
.align 16
.skip 16384 # 16KB de pilha é suficiente para começar
kernel_stack: