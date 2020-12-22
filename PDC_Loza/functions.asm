.CODE

; We have Microsoft x64 calling convention, so arguments are in:
; 1 - input row pointer (RCX)
; 2 - input matrix pointer (RDX)
; 3 - output row pointer (R8)

; Functions for BYTE matrices

multX64 PROC
	mov QWORD PTR [r8], 0 ; initialize output row with zeros
	xor r9, r9 ; outer loop index - R9 = 0
loop_outer:
	xor r10, r10 ; inner loop index - R10 = 0

loop_inner:
	; mov r11b, BYTE PTR [r8+r10]
	mov al, BYTE PTR [rcx+r9]
	mul BYTE PTR [rdx+r10]
	add BYTE PTR [r8+r10], al
	inc r10
	cmp r10, 7
	jl loop_inner

	add rdx, 8
	inc r9
	cmp r9, 7
	jl loop_outer

	ret
multX64 ENDP


multMMX PROC
	xor rax, rax
	xor r9, r9 ; outer loop index - R9 = 0
	pxor mm0, mm0 ; zero register
	pxor mm5, mm5 ; first 4 sums
	pxor mm6, mm6 ; last 4 sums (3 + 1 redundant)
loop_outer:
	movq mm1, [rdx]
	movq mm2, mm1
	punpckhbw mm1, mm0
	punpcklbw mm2, mm0

	mov al, [rcx+r9]
	movq mm3, rax
	pshufw mm3, mm3, 0
	
	pmullw mm1, mm3
	pmullw mm2, mm3
	paddusw mm5, mm1
	paddusw mm6, mm2

	add rdx, 8
	inc r9
	cmp r9, 7
	jl loop_outer

	packuswb mm6, mm5
	movq [r8], mm6
	ret
multMMX ENDP


multSSE PROC
	mov rax, 0100010001000100h
	movq xmm7, rax
	pinsrq xmm7, rax, 1

	xor rax, rax
	xor r9, r9 ; outer loop index - R9 = 0
	pxor xmm0, xmm0 ; zero register
	pxor xmm5, xmm5 ; all 8 sums (7 + 1 redundant)
loop_outer:
	movd xmm1, QWORD PTR [rdx] ; movq - not working, but movd can work for qword(64)
	punpcklbw xmm1, xmm0
	
	mov al, [rcx+r9]
	movd xmm3, eax
	pshufb xmm3, xmm7
	
	pmullw xmm1, xmm3
	paddsw xmm5, xmm1

	add rdx, 8
	inc r9
	cmp r9, 7
	jl loop_outer

	packuswb xmm5, xmm5
	movd QWORD PTR [r8], xmm5
	ret
multSSE ENDP

; Functions for FLOAT matrices

fmultX64 PROC
	pxor xmm0, xmm0 ; zero register
	movaps [r8], xmm0 ; initialize output row with zeros
	movaps [r8+16], xmm0 ; ..
	xor r9, r9 ; outer loop index - R9 = 0
loop_outer:
	xor r10, r10 ; inner loop index - R10 = 0

loop_inner:
	movss xmm1, DWORD PTR [r8+4*r10]
	movss xmm2, DWORD PTR [rcx+4*r9]
	mulss xmm2, DWORD PTR [rdx+4*r10]
	addss xmm1, xmm2
	movss DWORD PTR [r8+4*r10], xmm1
	
	inc r10
	cmp r10, 7
	jl loop_inner

	add rdx, 32
	inc r9
	cmp r9, 7
	jl loop_outer

	ret
fmultX64 ENDP


fmultSSE PROC
	pxor xmm0, xmm0 ; zero register
	movaps [r8], xmm0 ; initialize output row with zeros
	movaps [r8+16], xmm0 ; ..
	xor r9, r9 ; outer loop index - R9 = 0
loop_outer:
	movaps xmm1, [r8]
	movaps xmm2, [r8+16]

	movss xmm3, DWORD PTR [rcx+4*r9]
	shufps xmm3, xmm3, 0 ; <32bit>dest[i]=<32bit>src[index[i]]
	movaps xmm4, xmm3

	mulps xmm3, [rdx]
	mulps xmm4, [rdx+16]
	addps xmm1, xmm3
	addps xmm2, xmm4
	movaps [r8], xmm1
	movaps [r8+16], xmm2

	add rdx, 32
	inc r9
	cmp r9, 7
	jl loop_outer

	ret
fmultSSE ENDP

END