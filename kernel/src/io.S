.global inb
.global outb 

.text
inb:
  xor %eax, %eax
  mov 4(%esp), %edx
  inb %dx, %al 
  ret

outb:
  mov 4(%esp), %eax
  mov 8(%esp), %edx
  outb %al, %dx
  ret
