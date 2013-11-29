	.file	"SegServerIORttm.cpp"
	.section	.text._ZStorSt13_Ios_OpenmodeS_,"axG",@progbits,_ZStorSt13_Ios_OpenmodeS_,comdat
	.weak	_ZStorSt13_Ios_OpenmodeS_
	.type	_ZStorSt13_Ios_OpenmodeS_, @function
_ZStorSt13_Ios_OpenmodeS_:
.LFB640:
	pushq	%rbp
.LCFI0:
	movq	%rsp, %rbp
.LCFI1:
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	-4(%rbp), %edx
	movl	-8(%rbp), %eax
	orl	%edx, %eax
	leave
	ret
.LFE640:
	.size	_ZStorSt13_Ios_OpenmodeS_, .-_ZStorSt13_Ios_OpenmodeS_
	.text
	.type	_Z41__static_initialization_and_destruction_0ii, @function
_Z41__static_initialization_and_destruction_0ii:
.LFB1575:
	pushq	%rbp
.LCFI2:
	movq	%rsp, %rbp
.LCFI3:
	subq	$16, %rsp
.LCFI4:
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	cmpl	$1, -4(%rbp)
	jne	.L5
	cmpl	$65535, -8(%rbp)
	jne	.L5
	movl	$_ZStL8__ioinit, %edi
	call	_ZNSt8ios_base4InitC1Ev
	movl	$_ZNSt8ios_base4InitD1Ev, %edi
	movl	$__dso_handle, %edx
	movl	$_ZStL8__ioinit, %esi
	call	__cxa_atexit
.L5:
	leave
	ret
.LFE1575:
	.size	_Z41__static_initialization_and_destruction_0ii, .-_Z41__static_initialization_and_destruction_0ii
	.type	_GLOBAL__I__ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE, @function
_GLOBAL__I__ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE:
.LFB1585:
	pushq	%rbp
.LCFI5:
	movq	%rsp, %rbp
.LCFI6:
	movl	$65535, %esi
	movl	$1, %edi
	call	_Z41__static_initialization_and_destruction_0ii
	leave
	ret
.LFE1585:
	.size	_GLOBAL__I__ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE, .-_GLOBAL__I__ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE
	.section	.ctors,"aw",@progbits
	.align 8
	.quad	_GLOBAL__I__ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE
	.section	.rodata
.LC0:
	.string	"SPKR-INFO "
.LC1:
	.string	" 1 <NA> <NA> <NA> unknown "
.LC2:
	.string	" <NA>"
.LC4:
	.string	"SPEAKER "
.LC5:
	.string	" 1 "
.LC6:
	.string	" "
.LC7:
	.string	" <NA> <NA> "
.LC8:
	.string	"Cannot write RTTM file \""
.LC9:
	.string	"\"."
	.align 8
.LC10:
	.string	"/local_disk/agecanonix/AudioVideoSpkSeg/Tools/SpkMoulinette/src/SegServerIORttm.cpp"
.globl _Unwind_Resume
	.text
	.align 2
.globl _ZN15SegServerIORttm14writeSegServerERN5alize9SegServerERNS0_11LabelServerE
	.type	_ZN15SegServerIORttm14writeSegServerERN5alize9SegServerERNS0_11LabelServerE, @function
_ZN15SegServerIORttm14writeSegServerERN5alize9SegServerERNS0_11LabelServerE:
.LFB1469:
	pushq	%rbp
.LCFI7:
	movq	%rsp, %rbp
.LCFI8:
	pushq	%r12
.LCFI9:
	pushq	%rbx
.LCFI10:
	subq	$816, %rsp
.LCFI11:
	movq	%rdi, -664(%rbp)
	movq	%rsi, -672(%rbp)
	movq	%rdx, -680(%rbp)
	movl	$32, %esi
	movl	$16, %edi
	call	_ZStorSt13_Ios_OpenmodeS_
	movl	%eax, %ebx
	movq	-664(%rbp), %rax
	leaq	8(%rax), %rdi
.LEHB0:
	call	_ZNK5alize6String5c_strEv
	movq	%rax, %rsi
	leaq	-656(%rbp), %rdi
	movl	%ebx, %edx
	call	_ZNSt14basic_ofstreamIcSt11char_traitsIcEEC1EPKcSt13_Ios_Openmode
.LEHE0:
	leaq	-656(%rbp), %rdi
.LEHB1:
	call	_ZNSt14basic_ofstreamIcSt11char_traitsIcEE7is_openEv
	testb	%al, %al
	je	.L9
	movq	$0, -48(%rbp)
	jmp	.L10
.L11:
	movq	-48(%rbp), %rsi
	movq	-680(%rbp), %rdi
	call	_ZNK5alize11LabelServer8getLabelEm
	movq	%rax, %rdi
	call	_ZNK5alize5Label9getStringEv
	movq	%rax, -816(%rbp)
	movq	-664(%rbp), %rax
	addq	$40, %rax
	movq	%rax, -808(%rbp)
	leaq	-656(%rbp), %rdi
	movl	$.LC0, %esi
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
	movq	%rax, %rdi
	movq	-808(%rbp), %rsi
	call	_ZlsRSoRKN5alize6StringE
	movq	%rax, %rdi
	movl	$.LC1, %esi
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
	movq	%rax, %rdi
	movq	-816(%rbp), %rsi
	call	_ZlsRSoRKN5alize6StringE
	movq	%rax, %rdi
	movl	$.LC2, %esi
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
	movq	%rax, %rdi
	movl	$_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_, %esi
	call	_ZNSolsEPFRSoS_E
	addq	$1, -48(%rbp)
.L10:
	movq	-680(%rbp), %rdi
	call	_ZNK5alize11LabelServer4sizeEv
	cmpq	-48(%rbp), %rax
	seta	%al
	testb	%al, %al
	jne	.L11
	movq	$0, -40(%rbp)
	jmp	.L12
.L17:
	movq	-40(%rbp), %rsi
	movq	-672(%rbp), %rdi
	call	_ZNK5alize9SegServer6getSegEm
	movq	%rax, -32(%rbp)
	movq	-32(%rbp), %rax
	movq	(%rax), %rax
	addq	$40, %rax
	movq	(%rax), %rax
	movq	-32(%rbp), %rdi
	call	*%rax
	movq	%rax, -800(%rbp)
	cmpq	$0, -800(%rbp)
	js	.L13
	cvtsi2sdq	-800(%rbp), %xmm0
	movsd	%xmm0, -792(%rbp)
	jmp	.L14
.L13:
	movq	-800(%rbp), %rax
	shrq	%rax
	movq	-800(%rbp), %rdx
	andl	$1, %edx
	orq	%rdx, %rax
	cvtsi2sdq	%rax, %xmm0
	movapd	%xmm0, %xmm1
	addsd	%xmm0, %xmm1
	movsd	%xmm1, -792(%rbp)
.L14:
	movsd	.LC3(%rip), %xmm0
	mulsd	-792(%rbp), %xmm0
	unpcklpd	%xmm0, %xmm0
	cvtpd2ps	%xmm0, %xmm0
	movss	%xmm0, -24(%rbp)
	movq	-32(%rbp), %rax
	movq	(%rax), %rax
	addq	$48, %rax
	movq	(%rax), %rax
	movq	-32(%rbp), %rdi
	call	*%rax
	movq	%rax, -784(%rbp)
	cmpq	$0, -784(%rbp)
	js	.L15
	cvtsi2sdq	-784(%rbp), %xmm0
	movsd	%xmm0, -776(%rbp)
	jmp	.L16
.L15:
	movq	-784(%rbp), %rax
	shrq	%rax
	movq	-784(%rbp), %rdx
	andl	$1, %edx
	orq	%rdx, %rax
	cvtsi2sdq	%rax, %xmm0
	movapd	%xmm0, %xmm1
	addsd	%xmm0, %xmm1
	movsd	%xmm1, -776(%rbp)
.L16:
	movsd	.LC3(%rip), %xmm0
	mulsd	-776(%rbp), %xmm0
	unpcklpd	%xmm0, %xmm0
	cvtpd2ps	%xmm0, %xmm0
	movss	%xmm0, -20(%rbp)
	movq	-32(%rbp), %rdi
	call	_ZNK5alize11SegAbstract9labelCodeEv
	movq	%rax, %rsi
	movq	-680(%rbp), %rdi
	call	_ZNK5alize11LabelServer8getLabelEm
	movq	%rax, %rdi
	call	_ZNK5alize5Label9getStringEv
	movq	%rax, -768(%rbp)
	movq	-664(%rbp), %rax
	addq	$40, %rax
	movq	%rax, -760(%rbp)
	leaq	-656(%rbp), %rdi
	movl	$.LC4, %esi
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
	movq	%rax, %rdi
	movq	-760(%rbp), %rsi
	call	_ZlsRSoRKN5alize6StringE
	movq	%rax, %rdi
	movl	$.LC5, %esi
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
	movq	%rax, %rdi
	movss	-24(%rbp), %xmm0
	call	_ZNSolsEf
	movq	%rax, %rdi
	movl	$.LC6, %esi
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
	movq	%rax, %rdi
	movss	-20(%rbp), %xmm0
	call	_ZNSolsEf
	movq	%rax, %rdi
	movl	$.LC7, %esi
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
	movq	%rax, %rdi
	movq	-768(%rbp), %rsi
	call	_ZlsRSoRKN5alize6StringE
	movq	%rax, %rdi
	movl	$.LC2, %esi
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
	movq	%rax, %rdi
	movl	$_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_, %esi
	call	_ZNSolsEPFRSoS_E
	addq	$1, -40(%rbp)
.L12:
	movq	-672(%rbp), %rdi
	call	_ZNK5alize9SegServer11getSegCountEv
	cmpq	-40(%rbp), %rax
	seta	%al
	testb	%al, %al
	jne	.L17
	leaq	-656(%rbp), %rdi
	call	_ZNSt14basic_ofstreamIcSt11char_traitsIcEE5closeEv
.LEHE1:
	leaq	-656(%rbp), %rdi
.LEHB2:
	call	_ZNSt14basic_ofstreamIcSt11char_traitsIcEED1Ev
.LEHE2:
	addq	$816, %rsp
	popq	%rbx
	popq	%r12
	leave
	ret
.L9:
	movq	-664(%rbp), %rax
	leaq	8(%rax), %rdx
	leaq	-112(%rbp), %rdi
	movl	$.LC8, %esi
.LEHB3:
	call	_ZplPKcRKN5alize6StringE
.LEHE3:
	leaq	-144(%rbp), %rdi
	leaq	-112(%rbp), %rsi
	movl	$.LC9, %edx
.LEHB4:
	call	_ZNK5alize6StringplEPKc
.LEHE4:
	leaq	-144(%rbp), %rax
	movq	%rax, -752(%rbp)
	leaq	-80(%rbp), %rdi
	movl	$.LC10, %esi
.LEHB5:
	call	_ZN5alize6StringC1EPKc
.LEHE5:
	leaq	-80(%rbp), %rbx
	movl	$112, %edi
	call	__cxa_allocate_exception
	movq	%rax, -736(%rbp)
	movq	-736(%rbp), %rdi
	movl	$101, %ecx
	movq	%rbx, %rdx
	movq	-752(%rbp), %rsi
.LEHB6:
	call	_ZN5alize9ExceptionC1ERKNS_6StringES3_i
.LEHE6:
	leaq	-80(%rbp), %rax
	movq	%rax, -744(%rbp)
	movq	-744(%rbp), %rdi
.LEHB7:
	call	_ZN5alize6StringD1Ev
.LEHE7:
	jmp	.L50
.L45:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
.L19:
	movl	-824(%rbp), %r12d
	movq	-832(%rbp), %rbx
	movq	-736(%rbp), %rdi
	call	__cxa_free_exception
	movq	%rbx, -832(%rbp)
	movslq	%r12d,%r12
	movq	%r12, -824(%rbp)
.L46:
.L20:
	movl	-824(%rbp), %eax
	movl	%eax, -716(%rbp)
	movq	-832(%rbp), %rax
	movq	%rax, -728(%rbp)
	leaq	-80(%rbp), %rax
	movq	%rax, -744(%rbp)
	movq	-744(%rbp), %rdi
.LEHB8:
	call	_ZN5alize6StringD1Ev
.LEHE8:
	movq	-728(%rbp), %rax
	movq	%rax, -832(%rbp)
	movslq	-716(%rbp),%rax
	movq	%rax, -824(%rbp)
	jmp	.L24
.L44:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
	cmpq	$-1, -824(%rbp)
	jne	.L24
.L21:
	call	_ZSt9terminatev
.L50:
	leaq	-144(%rbp), %rdi
.LEHB9:
	call	_ZN5alize6StringD1Ev
.LEHE9:
	jmp	.L51
.L43:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
	cmpq	$-1, -824(%rbp)
	jne	.L24
.L23:
	call	_ZSt9terminatev
.L47:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
.L24:
	movl	-824(%rbp), %eax
	movl	%eax, -700(%rbp)
	movq	-832(%rbp), %rax
	movq	%rax, -712(%rbp)
	leaq	-144(%rbp), %rdi
.LEHB10:
	call	_ZN5alize6StringD1Ev
.LEHE10:
	movq	-712(%rbp), %rax
	movq	%rax, -832(%rbp)
	movslq	-700(%rbp),%rax
	movq	%rax, -824(%rbp)
	jmp	.L28
.L42:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
	cmpq	$-1, -824(%rbp)
	jne	.L28
.L25:
	call	_ZSt9terminatev
.L51:
	leaq	-112(%rbp), %rdi
.LEHB11:
	call	_ZN5alize6StringD1Ev
.LEHE11:
	jmp	.L52
.L41:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
	cmpq	$-1, -824(%rbp)
	jne	.L28
.L27:
	call	_ZSt9terminatev
.L48:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
.L28:
	movl	-824(%rbp), %eax
	movl	%eax, -684(%rbp)
	movq	-832(%rbp), %rax
	movq	%rax, -696(%rbp)
	leaq	-112(%rbp), %rdi
.LEHB12:
	call	_ZN5alize6StringD1Ev
.LEHE12:
	movq	-696(%rbp), %rax
	movq	%rax, -832(%rbp)
	movslq	-684(%rbp),%rax
	movq	%rax, -824(%rbp)
	jmp	.L31
.L40:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
	cmpq	$-1, -824(%rbp)
	jne	.L31
.L29:
	call	_ZSt9terminatev
.L52:
	movl	$_ZN5alize9ExceptionD1Ev, %edx
	movl	$_ZTIN5alize9ExceptionE, %esi
	movq	-736(%rbp), %rdi
.LEHB13:
	call	__cxa_throw
.LEHE13:
.L39:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
	cmpq	$-1, -824(%rbp)
	jne	.L31
.L30:
	call	_ZSt9terminatev
.L49:
	movq	%rax, -832(%rbp)
	movq	%rdx, -824(%rbp)
.L31:
	movl	-824(%rbp), %r12d
	movq	-832(%rbp), %rbx
	leaq	-656(%rbp), %rdi
	call	_ZNSt14basic_ofstreamIcSt11char_traitsIcEED1Ev
	movq	%rbx, -832(%rbp)
	movslq	%r12d,%r12
	movq	-832(%rbp), %rdi
.LEHB14:
	call	_Unwind_Resume
.LEHE14:
.LFE1469:
	.size	_ZN15SegServerIORttm14writeSegServerERN5alize9SegServerERNS0_11LabelServerE, .-_ZN15SegServerIORttm14writeSegServerERN5alize9SegServerERNS0_11LabelServerE
.globl __gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
	.align 4
.LLSDA1469:
	.byte	0xff
	.byte	0x3
	.uleb128 .LLSDATT1469-.LLSDATTD1469
.LLSDATTD1469:
	.byte	0x1
	.uleb128 .LLSDACSE1469-.LLSDACSB1469
.LLSDACSB1469:
	.uleb128 .LEHB0-.LFB1469
	.uleb128 .LEHE0-.LEHB0
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 .LEHB1-.LFB1469
	.uleb128 .LEHE1-.LEHB1
	.uleb128 .L49-.LFB1469
	.uleb128 0x0
	.uleb128 .LEHB2-.LFB1469
	.uleb128 .LEHE2-.LEHB2
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 .LEHB3-.LFB1469
	.uleb128 .LEHE3-.LEHB3
	.uleb128 .L49-.LFB1469
	.uleb128 0x0
	.uleb128 .LEHB4-.LFB1469
	.uleb128 .LEHE4-.LEHB4
	.uleb128 .L48-.LFB1469
	.uleb128 0x0
	.uleb128 .LEHB5-.LFB1469
	.uleb128 .LEHE5-.LEHB5
	.uleb128 .L47-.LFB1469
	.uleb128 0x0
	.uleb128 .LEHB6-.LFB1469
	.uleb128 .LEHE6-.LEHB6
	.uleb128 .L45-.LFB1469
	.uleb128 0x0
	.uleb128 .LEHB7-.LFB1469
	.uleb128 .LEHE7-.LEHB7
	.uleb128 .L43-.LFB1469
	.uleb128 0x3
	.uleb128 .LEHB8-.LFB1469
	.uleb128 .LEHE8-.LEHB8
	.uleb128 .L44-.LFB1469
	.uleb128 0x3
	.uleb128 .LEHB9-.LFB1469
	.uleb128 .LEHE9-.LEHB9
	.uleb128 .L41-.LFB1469
	.uleb128 0x3
	.uleb128 .LEHB10-.LFB1469
	.uleb128 .LEHE10-.LEHB10
	.uleb128 .L42-.LFB1469
	.uleb128 0x3
	.uleb128 .LEHB11-.LFB1469
	.uleb128 .LEHE11-.LEHB11
	.uleb128 .L39-.LFB1469
	.uleb128 0x3
	.uleb128 .LEHB12-.LFB1469
	.uleb128 .LEHE12-.LEHB12
	.uleb128 .L40-.LFB1469
	.uleb128 0x3
	.uleb128 .LEHB13-.LFB1469
	.uleb128 .LEHE13-.LEHB13
	.uleb128 .L49-.LFB1469
	.uleb128 0x0
	.uleb128 .LEHB14-.LFB1469
	.uleb128 .LEHE14-.LEHB14
	.uleb128 0x0
	.uleb128 0x0
.LLSDACSE1469:
	.byte	0x0
	.byte	0x0
	.byte	0x7f
	.byte	0x7d
	.align 4
.LLSDATT1469:
	.byte	0x0
	.text
	.section	.rodata
.LC11:
	.string	"SPKR-INFO"
.LC12:
	.string	""
.LC13:
	.string	"SPEAKER"
	.text
	.align 2
.globl _ZN15SegServerIORttm13readSegServerERN5alize9SegServerERNS0_11LabelServerE
	.type	_ZN15SegServerIORttm13readSegServerERN5alize9SegServerERNS0_11LabelServerE, @function
_ZN15SegServerIORttm13readSegServerERN5alize9SegServerERNS0_11LabelServerE:
.LFB1468:
	pushq	%rbp
.LCFI12:
	movq	%rsp, %rbp
.LCFI13:
	pushq	%r12
.LCFI14:
	pushq	%rbx
.LCFI15:
	subq	$384, %rsp
.LCFI16:
	movq	%rdi, -312(%rbp)
	movq	%rsi, -320(%rbp)
	movq	%rdx, -328(%rbp)
	movl	$104, %edi
.LEHB15:
	call	_Znwm
.LEHE15:
	movq	%rax, -360(%rbp)
	movq	-312(%rbp), %rax
	leaq	8(%rax), %rsi
	movq	-360(%rbp), %rdi
.LEHB16:
	call	_ZN5alize5XListC1ERKNS_6StringE
.LEHE16:
	movq	-360(%rbp), %rax
	movq	%rax, -88(%rbp)
	jmp	.L54
.L79:
	movq	%rax, -376(%rbp)
	movq	%rdx, -368(%rbp)
.L55:
	movl	-368(%rbp), %r12d
	movq	-376(%rbp), %rbx
	movq	-360(%rbp), %rdi
	call	_ZdlPv
	movq	%rbx, -376(%rbp)
	movslq	%r12d,%r12
	movq	-376(%rbp), %rdi
.LEHB17:
	call	_Unwind_Resume
.L69:
	movq	-80(%rbp), %rdi
	movl	$1, %edx
	movl	$0, %esi
	call	_ZNK5alize5XLine10getElementEmb
	movq	%rax, %rsi
	leaq	-272(%rbp), %rdi
	call	_ZN5alize6StringC1ERKS0_
.LEHE17:
	leaq	-272(%rbp), %rdi
	movl	$.LC11, %esi
.LEHB18:
	call	_ZNK5alize6StringeqEPKc
	testb	%al, %al
	je	.L56
	movq	-80(%rbp), %rdi
	movl	$1, %edx
	movl	$7, %esi
	call	_ZNK5alize5XLine10getElementEmb
	movq	%rax, %rsi
	leaq	-304(%rbp), %rdi
	call	_ZN5alize6StringC1ERKS0_
.LEHE18:
	leaq	-304(%rbp), %rsi
	leaq	-240(%rbp), %rdi
.LEHB19:
	call	_ZN5alize5LabelC1ERKNS_6StringE
.LEHE19:
	leaq	-240(%rbp), %rsi
	movq	-328(%rbp), %rdi
	movl	$0, %edx
.LEHB20:
	call	_ZN5alize11LabelServer8addLabelERKNS_5LabelEb
.LEHE20:
	movq	%rax, -72(%rbp)
	leaq	-240(%rbp), %rax
	movq	%rax, -352(%rbp)
	movq	-352(%rbp), %rdi
.LEHB21:
	call	_ZN5alize5LabelD1Ev
.LEHE21:
	jmp	.L80
.L76:
	movq	%rax, -376(%rbp)
	movq	%rdx, -368(%rbp)
.L58:
	movl	-368(%rbp), %r12d
	movq	-376(%rbp), %rbx
	leaq	-240(%rbp), %rax
	movq	%rax, -352(%rbp)
	movq	-352(%rbp), %rdi
	call	_ZN5alize5LabelD1Ev
	movq	%rbx, -376(%rbp)
	movslq	%r12d,%r12
	movq	%r12, -368(%rbp)
	jmp	.L64
.L80:
	leaq	-160(%rbp), %rdi
	movl	$.LC12, %esi
.LEHB22:
	call	_ZN5alize6StringC1EPKc
.LEHE22:
	leaq	-128(%rbp), %rdi
	movl	$.LC12, %esi
.LEHB23:
	call	_ZN5alize6StringC1EPKc
.LEHE23:
	movq	-72(%rbp), %rsi
	leaq	-160(%rbp), %rcx
	leaq	-128(%rbp), %rdx
	movq	-320(%rbp), %rdi
.LEHB24:
	call	_ZN5alize9SegServer13createClusterEmRKNS_6StringES3_
.LEHE24:
	leaq	-128(%rbp), %rax
	movq	%rax, -336(%rbp)
	movq	-336(%rbp), %rdi
.LEHB25:
	call	_ZN5alize6StringD1Ev
.LEHE25:
	jmp	.L81
.L74:
	movq	%rax, -376(%rbp)
	movq	%rdx, -368(%rbp)
.L60:
	movl	-368(%rbp), %r12d
	movq	-376(%rbp), %rbx
	leaq	-128(%rbp), %rax
	movq	%rax, -336(%rbp)
	movq	-336(%rbp), %rdi
	call	_ZN5alize6StringD1Ev
	movq	%rbx, -376(%rbp)
	movslq	%r12d,%r12
	movq	%r12, -368(%rbp)
	jmp	.L62
.L81:
	leaq	-160(%rbp), %rax
	movq	%rax, -344(%rbp)
	movq	-344(%rbp), %rdi
.LEHB26:
	call	_ZN5alize6StringD1Ev
.LEHE26:
	jmp	.L82
.L75:
	movq	%rax, -376(%rbp)
	movq	%rdx, -368(%rbp)
.L62:
	movl	-368(%rbp), %r12d
	movq	-376(%rbp), %rbx
	leaq	-160(%rbp), %rax
	movq	%rax, -344(%rbp)
	movq	-344(%rbp), %rdi
	call	_ZN5alize6StringD1Ev
	movq	%rbx, -376(%rbp)
	movslq	%r12d,%r12
	movq	%r12, -368(%rbp)
	jmp	.L64
.L82:
	leaq	-304(%rbp), %rdi
.LEHB27:
	call	_ZN5alize6StringD1Ev
.LEHE27:
	jmp	.L65
.L77:
	movq	%rax, -376(%rbp)
	movq	%rdx, -368(%rbp)
.L64:
	movl	-368(%rbp), %r12d
	movq	-376(%rbp), %rbx
	leaq	-304(%rbp), %rdi
	call	_ZN5alize6StringD1Ev
	movq	%rbx, -376(%rbp)
	movslq	%r12d,%r12
	movq	%r12, -368(%rbp)
	jmp	.L68
.L56:
	leaq	-272(%rbp), %rdi
	movl	$.LC13, %esi
.LEHB28:
	call	_ZNK5alize6StringeqEPKc
	testb	%al, %al
	je	.L65
	movq	-80(%rbp), %rdi
	movl	$1, %edx
	movl	$7, %esi
	call	_ZNK5alize5XLine10getElementEmb
	movq	%rax, %rsi
	leaq	-304(%rbp), %rdi
	call	_ZN5alize6StringC1ERKS0_
.LEHE28:
	leaq	-304(%rbp), %rsi
	movq	-328(%rbp), %rdi
.LEHB29:
	call	_ZNK5alize11LabelServer21getLabelIndexByStringERKNS_6StringE
	movq	%rax, -64(%rbp)
	cmpq	$-1, -64(%rbp)
	je	.L66
	movq	-80(%rbp), %rdi
	movl	$1, %edx
	movl	$3, %esi
	call	_ZNK5alize5XLine10getElementEmb
	movq	%rax, %rdi
	call	_ZNK5alize6String8toDoubleEv
	movsd	%xmm0, -56(%rbp)
	movq	-80(%rbp), %rdi
	movl	$1, %edx
	movl	$4, %esi
	call	_ZNK5alize5XLine10getElementEmb
	movq	%rax, %rdi
	call	_ZNK5alize6String8toDoubleEv
	movsd	%xmm0, -48(%rbp)
	movsd	.LC3(%rip), %xmm0
	movq	-56(%rbp), %rax
	movapd	%xmm0, %xmm1
	movq	%rax, -392(%rbp)
	movsd	-392(%rbp), %xmm0
	call	_Z14timeToFrameIdxdd
	movq	%rax, -40(%rbp)
	movsd	.LC3(%rip), %xmm0
	movq	-48(%rbp), %rax
	movapd	%xmm0, %xmm1
	movq	%rax, -392(%rbp)
	movsd	-392(%rbp), %xmm0
	call	_Z14timeToFrameIdxdd
	movq	%rax, -32(%rbp)
	movq	-64(%rbp), %rsi
	movq	-320(%rbp), %rdi
	call	_ZNK5alize9SegServer10getClusterEm
	movq	%rax, -24(%rbp)
	movq	-312(%rbp), %rax
	leaq	40(%rax), %rdx
	movq	-64(%rbp), %rcx
	leaq	-304(%rbp), %rax
	movq	-32(%rbp), %rsi
	movq	-40(%rbp), %rdi
	movq	-320(%rbp), %r10
	movq	%rdx, %r9
	movq	%rax, %r8
	movq	%rsi, %rdx
	movq	%rdi, %rsi
	movq	%r10, %rdi
	call	_ZN5alize9SegServer9createSegEmmmRKNS_6StringES3_
	movq	%rax, %rsi
	movq	-24(%rbp), %rdi
	call	_ZN5alize10SegCluster3addERNS_11SegAbstractE
.LEHE29:
.L66:
	leaq	-304(%rbp), %rdi
.LEHB30:
	call	_ZN5alize6StringD1Ev
.LEHE30:
	jmp	.L65
.L73:
	movq	%rax, -376(%rbp)
	movq	%rdx, -368(%rbp)
.L67:
	movl	-368(%rbp), %r12d
	movq	-376(%rbp), %rbx
	leaq	-304(%rbp), %rdi
	call	_ZN5alize6StringD1Ev
	movq	%rbx, -376(%rbp)
	movslq	%r12d,%r12
	movq	%r12, -368(%rbp)
	jmp	.L68
.L65:
	leaq	-272(%rbp), %rdi
.LEHB31:
	call	_ZN5alize6StringD1Ev
.LEHE31:
	jmp	.L54
.L78:
	movq	%rax, -376(%rbp)
	movq	%rdx, -368(%rbp)
.L68:
	movl	-368(%rbp), %r12d
	movq	-376(%rbp), %rbx
	leaq	-272(%rbp), %rdi
	call	_ZN5alize6StringD1Ev
	movq	%rbx, -376(%rbp)
	movslq	%r12d,%r12
	movq	-376(%rbp), %rdi
.LEHB32:
	call	_Unwind_Resume
.L54:
	movq	-88(%rbp), %rdi
	call	_ZNK5alize5XList7getLineEv
	movq	%rax, -80(%rbp)
	cmpq	$0, -80(%rbp)
	setne	%al
	testb	%al, %al
	jne	.L69
	cmpq	$0, -80(%rbp)
	je	.L70
	movq	-80(%rbp), %rax
	movq	(%rax), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movq	-80(%rbp), %rdi
	call	*%rax
.L70:
	cmpq	$0, -88(%rbp)
	je	.L72
	movq	-88(%rbp), %rax
	movq	(%rax), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movq	-88(%rbp), %rdi
	call	*%rax
.LEHE32:
.L72:
	addq	$384, %rsp
	popq	%rbx
	popq	%r12
	leave
	ret
.LFE1468:
	.size	_ZN15SegServerIORttm13readSegServerERN5alize9SegServerERNS0_11LabelServerE, .-_ZN15SegServerIORttm13readSegServerERN5alize9SegServerERNS0_11LabelServerE
	.section	.gcc_except_table
.LLSDA1468:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1468-.LLSDACSB1468
.LLSDACSB1468:
	.uleb128 .LEHB15-.LFB1468
	.uleb128 .LEHE15-.LEHB15
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 .LEHB16-.LFB1468
	.uleb128 .LEHE16-.LEHB16
	.uleb128 .L79-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB17-.LFB1468
	.uleb128 .LEHE17-.LEHB17
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 .LEHB18-.LFB1468
	.uleb128 .LEHE18-.LEHB18
	.uleb128 .L78-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB19-.LFB1468
	.uleb128 .LEHE19-.LEHB19
	.uleb128 .L77-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB20-.LFB1468
	.uleb128 .LEHE20-.LEHB20
	.uleb128 .L76-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB21-.LFB1468
	.uleb128 .LEHE21-.LEHB21
	.uleb128 .L77-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB22-.LFB1468
	.uleb128 .LEHE22-.LEHB22
	.uleb128 .L77-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB23-.LFB1468
	.uleb128 .LEHE23-.LEHB23
	.uleb128 .L75-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB24-.LFB1468
	.uleb128 .LEHE24-.LEHB24
	.uleb128 .L74-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB25-.LFB1468
	.uleb128 .LEHE25-.LEHB25
	.uleb128 .L75-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB26-.LFB1468
	.uleb128 .LEHE26-.LEHB26
	.uleb128 .L77-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB27-.LFB1468
	.uleb128 .LEHE27-.LEHB27
	.uleb128 .L78-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB28-.LFB1468
	.uleb128 .LEHE28-.LEHB28
	.uleb128 .L78-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB29-.LFB1468
	.uleb128 .LEHE29-.LEHB29
	.uleb128 .L73-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB30-.LFB1468
	.uleb128 .LEHE30-.LEHB30
	.uleb128 .L78-.LFB1468
	.uleb128 0x0
	.uleb128 .LEHB31-.LFB1468
	.uleb128 .LEHE31-.LEHB31
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 .LEHB32-.LFB1468
	.uleb128 .LEHE32-.LEHB32
	.uleb128 0x0
	.uleb128 0x0
.LLSDACSE1468:
	.text
	.align 2
.globl _ZN15SegServerIORttmD1Ev
	.type	_ZN15SegServerIORttmD1Ev, @function
_ZN15SegServerIORttmD1Ev:
.LFB1467:
	pushq	%rbp
.LCFI17:
	movq	%rsp, %rbp
.LCFI18:
	subq	$16, %rsp
.LCFI19:
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	$_ZTV15SegServerIORttm+16, (%rax)
	movq	-8(%rbp), %rdi
	call	_ZN11SegServerIOD2Ev
	leave
	ret
.LFE1467:
	.size	_ZN15SegServerIORttmD1Ev, .-_ZN15SegServerIORttmD1Ev
	.align 2
.globl _ZN15SegServerIORttmD2Ev
	.type	_ZN15SegServerIORttmD2Ev, @function
_ZN15SegServerIORttmD2Ev:
.LFB1466:
	pushq	%rbp
.LCFI20:
	movq	%rsp, %rbp
.LCFI21:
	subq	$16, %rsp
.LCFI22:
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	$_ZTV15SegServerIORttm+16, (%rax)
	movq	-8(%rbp), %rdi
	call	_ZN11SegServerIOD2Ev
	leave
	ret
.LFE1466:
	.size	_ZN15SegServerIORttmD2Ev, .-_ZN15SegServerIORttmD2Ev
	.align 2
.globl _ZN15SegServerIORttmC1EN5alize6StringEPNS0_6ConfigE
	.type	_ZN15SegServerIORttmC1EN5alize6StringEPNS0_6ConfigE, @function
_ZN15SegServerIORttmC1EN5alize6StringEPNS0_6ConfigE:
.LFB1464:
	pushq	%rbp
.LCFI23:
	movq	%rsp, %rbp
.LCFI24:
	pushq	%r12
.LCFI25:
	pushq	%rbx
.LCFI26:
	subq	$64, %rsp
.LCFI27:
	movq	%rdi, -56(%rbp)
	movq	%rsi, -64(%rbp)
	movq	%rdx, -72(%rbp)
	movq	-64(%rbp), %rsi
	leaq	-48(%rbp), %rdi
.LEHB33:
	call	_ZN5alize6StringC1ERKS0_
.LEHE33:
	movq	-56(%rbp), %rdi
	movq	-72(%rbp), %rdx
	leaq	-48(%rbp), %rsi
.LEHB34:
	call	_ZN11SegServerIOC2EN5alize6StringEPNS0_6ConfigE
.LEHE34:
	leaq	-48(%rbp), %rdi
.LEHB35:
	call	_ZN5alize6StringD1Ev
.LEHE35:
	movq	-56(%rbp), %rax
	movq	$_ZTV15SegServerIORttm+16, (%rax)
	addq	$64, %rsp
	popq	%rbx
	popq	%r12
	leave
	ret
.L92:
	movq	%rax, -80(%rbp)
.L90:
	movl	%edx, %r12d
	movq	-80(%rbp), %rbx
	leaq	-48(%rbp), %rdi
	call	_ZN5alize6StringD1Ev
	movq	%rbx, -80(%rbp)
	movslq	%r12d,%rdx
	movq	-80(%rbp), %rdi
.LEHB36:
	call	_Unwind_Resume
.LEHE36:
.LFE1464:
	.size	_ZN15SegServerIORttmC1EN5alize6StringEPNS0_6ConfigE, .-_ZN15SegServerIORttmC1EN5alize6StringEPNS0_6ConfigE
	.section	.gcc_except_table
.LLSDA1464:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1464-.LLSDACSB1464
.LLSDACSB1464:
	.uleb128 .LEHB33-.LFB1464
	.uleb128 .LEHE33-.LEHB33
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 .LEHB34-.LFB1464
	.uleb128 .LEHE34-.LEHB34
	.uleb128 .L92-.LFB1464
	.uleb128 0x0
	.uleb128 .LEHB35-.LFB1464
	.uleb128 .LEHE35-.LEHB35
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 .LEHB36-.LFB1464
	.uleb128 .LEHE36-.LEHB36
	.uleb128 0x0
	.uleb128 0x0
.LLSDACSE1464:
	.text
	.align 2
.globl _ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE
	.type	_ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE, @function
_ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE:
.LFB1463:
	pushq	%rbp
.LCFI28:
	movq	%rsp, %rbp
.LCFI29:
	pushq	%r12
.LCFI30:
	pushq	%rbx
.LCFI31:
	subq	$64, %rsp
.LCFI32:
	movq	%rdi, -56(%rbp)
	movq	%rsi, -64(%rbp)
	movq	%rdx, -72(%rbp)
	movq	-64(%rbp), %rsi
	leaq	-48(%rbp), %rdi
.LEHB37:
	call	_ZN5alize6StringC1ERKS0_
.LEHE37:
	movq	-56(%rbp), %rdi
	movq	-72(%rbp), %rdx
	leaq	-48(%rbp), %rsi
.LEHB38:
	call	_ZN11SegServerIOC2EN5alize6StringEPNS0_6ConfigE
.LEHE38:
	leaq	-48(%rbp), %rdi
.LEHB39:
	call	_ZN5alize6StringD1Ev
.LEHE39:
	movq	-56(%rbp), %rax
	movq	$_ZTV15SegServerIORttm+16, (%rax)
	addq	$64, %rsp
	popq	%rbx
	popq	%r12
	leave
	ret
.L96:
	movq	%rax, -80(%rbp)
.L94:
	movl	%edx, %r12d
	movq	-80(%rbp), %rbx
	leaq	-48(%rbp), %rdi
	call	_ZN5alize6StringD1Ev
	movq	%rbx, -80(%rbp)
	movslq	%r12d,%rdx
	movq	-80(%rbp), %rdi
.LEHB40:
	call	_Unwind_Resume
.LEHE40:
.LFE1463:
	.size	_ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE, .-_ZN15SegServerIORttmC2EN5alize6StringEPNS0_6ConfigE
	.section	.gcc_except_table
.LLSDA1463:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1463-.LLSDACSB1463
.LLSDACSB1463:
	.uleb128 .LEHB37-.LFB1463
	.uleb128 .LEHE37-.LEHB37
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 .LEHB38-.LFB1463
	.uleb128 .LEHE38-.LEHB38
	.uleb128 .L96-.LFB1463
	.uleb128 0x0
	.uleb128 .LEHB39-.LFB1463
	.uleb128 .LEHE39-.LEHB39
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 .LEHB40-.LFB1463
	.uleb128 .LEHE40-.LEHB40
	.uleb128 0x0
	.uleb128 0x0
.LLSDACSE1463:
	.text
	.weak	_ZTV15SegServerIORttm
	.section	.rodata._ZTV15SegServerIORttm,"aG",@progbits,_ZTV15SegServerIORttm,comdat
	.align 32
	.type	_ZTV15SegServerIORttm, @object
	.size	_ZTV15SegServerIORttm, 32
_ZTV15SegServerIORttm:
	.quad	0
	.quad	_ZTI15SegServerIORttm
	.quad	_ZN15SegServerIORttm13readSegServerERN5alize9SegServerERNS0_11LabelServerE
	.quad	_ZN15SegServerIORttm14writeSegServerERN5alize9SegServerERNS0_11LabelServerE
	.weak	_ZTS15SegServerIORttm
	.section	.rodata._ZTS15SegServerIORttm,"aG",@progbits,_ZTS15SegServerIORttm,comdat
	.align 16
	.type	_ZTS15SegServerIORttm, @object
	.size	_ZTS15SegServerIORttm, 18
_ZTS15SegServerIORttm:
	.string	"15SegServerIORttm"
	.weak	_ZTI15SegServerIORttm
	.section	.rodata._ZTI15SegServerIORttm,"aG",@progbits,_ZTI15SegServerIORttm,comdat
	.align 16
	.type	_ZTI15SegServerIORttm, @object
	.size	_ZTI15SegServerIORttm, 24
_ZTI15SegServerIORttm:
	.quad	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.quad	_ZTS15SegServerIORttm
	.quad	_ZTI11SegServerIO
	.local	_ZStL8__ioinit
	.comm	_ZStL8__ioinit,1,1
	.weakref	_ZL20__gthrw_pthread_oncePiPFvvE,pthread_once
	.weakref	_ZL27__gthrw_pthread_getspecificj,pthread_getspecific
	.weakref	_ZL27__gthrw_pthread_setspecificjPKv,pthread_setspecific
	.weakref	_ZL22__gthrw_pthread_createPmPK14pthread_attr_tPFPvS3_ES3_,pthread_create
	.weakref	_ZL22__gthrw_pthread_cancelm,pthread_cancel
	.weakref	_ZL26__gthrw_pthread_mutex_lockP15pthread_mutex_t,pthread_mutex_lock
	.weakref	_ZL29__gthrw_pthread_mutex_trylockP15pthread_mutex_t,pthread_mutex_trylock
	.weakref	_ZL28__gthrw_pthread_mutex_unlockP15pthread_mutex_t,pthread_mutex_unlock
	.weakref	_ZL26__gthrw_pthread_mutex_initP15pthread_mutex_tPK19pthread_mutexattr_t,pthread_mutex_init
	.weakref	_ZL30__gthrw_pthread_cond_broadcastP14pthread_cond_t,pthread_cond_broadcast
	.weakref	_ZL25__gthrw_pthread_cond_waitP14pthread_cond_tP15pthread_mutex_t,pthread_cond_wait
	.weakref	_ZL26__gthrw_pthread_key_createPjPFvPvE,pthread_key_create
	.weakref	_ZL26__gthrw_pthread_key_deletej,pthread_key_delete
	.weakref	_ZL30__gthrw_pthread_mutexattr_initP19pthread_mutexattr_t,pthread_mutexattr_init
	.weakref	_ZL33__gthrw_pthread_mutexattr_settypeP19pthread_mutexattr_ti,pthread_mutexattr_settype
	.weakref	_ZL33__gthrw_pthread_mutexattr_destroyP19pthread_mutexattr_t,pthread_mutexattr_destroy
	.section	.rodata
	.align 8
.LC3:
	.long	1202590843
	.long	1065646817
	.section	.eh_frame,"a",@progbits
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0x0
	.byte	0x1
	.string	"zPLR"
	.uleb128 0x1
	.sleb128 -8
	.byte	0x10
	.uleb128 0x7
	.byte	0x3
	.long	__gxx_personality_v0
	.byte	0x3
	.byte	0x3
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90
	.uleb128 0x1
	.align 8
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1
.LASFDE1:
	.long	.LASFDE1-.Lframe1
	.long	.LFB640
	.long	.LFE640-.LFB640
	.uleb128 0x4
	.long	0x0
	.byte	0x4
	.long	.LCFI0-.LFB640
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI1-.LCFI0
	.byte	0xd
	.uleb128 0x6
	.align 8
.LEFDE1:
.LSFDE3:
	.long	.LEFDE3-.LASFDE3
.LASFDE3:
	.long	.LASFDE3-.Lframe1
	.long	.LFB1575
	.long	.LFE1575-.LFB1575
	.uleb128 0x4
	.long	0x0
	.byte	0x4
	.long	.LCFI2-.LFB1575
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI3-.LCFI2
	.byte	0xd
	.uleb128 0x6
	.align 8
.LEFDE3:
.LSFDE5:
	.long	.LEFDE5-.LASFDE5
.LASFDE5:
	.long	.LASFDE5-.Lframe1
	.long	.LFB1585
	.long	.LFE1585-.LFB1585
	.uleb128 0x4
	.long	0x0
	.byte	0x4
	.long	.LCFI5-.LFB1585
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI6-.LCFI5
	.byte	0xd
	.uleb128 0x6
	.align 8
.LEFDE5:
.LSFDE7:
	.long	.LEFDE7-.LASFDE7
.LASFDE7:
	.long	.LASFDE7-.Lframe1
	.long	.LFB1469
	.long	.LFE1469-.LFB1469
	.uleb128 0x4
	.long	.LLSDA1469
	.byte	0x4
	.long	.LCFI7-.LFB1469
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI8-.LCFI7
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI11-.LCFI8
	.byte	0x83
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x3
	.align 8
.LEFDE7:
.LSFDE9:
	.long	.LEFDE9-.LASFDE9
.LASFDE9:
	.long	.LASFDE9-.Lframe1
	.long	.LFB1468
	.long	.LFE1468-.LFB1468
	.uleb128 0x4
	.long	.LLSDA1468
	.byte	0x4
	.long	.LCFI12-.LFB1468
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI13-.LCFI12
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI16-.LCFI13
	.byte	0x83
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x3
	.align 8
.LEFDE9:
.LSFDE11:
	.long	.LEFDE11-.LASFDE11
.LASFDE11:
	.long	.LASFDE11-.Lframe1
	.long	.LFB1467
	.long	.LFE1467-.LFB1467
	.uleb128 0x4
	.long	0x0
	.byte	0x4
	.long	.LCFI17-.LFB1467
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI18-.LCFI17
	.byte	0xd
	.uleb128 0x6
	.align 8
.LEFDE11:
.LSFDE13:
	.long	.LEFDE13-.LASFDE13
.LASFDE13:
	.long	.LASFDE13-.Lframe1
	.long	.LFB1466
	.long	.LFE1466-.LFB1466
	.uleb128 0x4
	.long	0x0
	.byte	0x4
	.long	.LCFI20-.LFB1466
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI21-.LCFI20
	.byte	0xd
	.uleb128 0x6
	.align 8
.LEFDE13:
.LSFDE15:
	.long	.LEFDE15-.LASFDE15
.LASFDE15:
	.long	.LASFDE15-.Lframe1
	.long	.LFB1464
	.long	.LFE1464-.LFB1464
	.uleb128 0x4
	.long	.LLSDA1464
	.byte	0x4
	.long	.LCFI23-.LFB1464
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI24-.LCFI23
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI27-.LCFI24
	.byte	0x83
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x3
	.align 8
.LEFDE15:
.LSFDE17:
	.long	.LEFDE17-.LASFDE17
.LASFDE17:
	.long	.LASFDE17-.Lframe1
	.long	.LFB1463
	.long	.LFE1463-.LFB1463
	.uleb128 0x4
	.long	.LLSDA1463
	.byte	0x4
	.long	.LCFI28-.LFB1463
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI29-.LCFI28
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI32-.LCFI29
	.byte	0x83
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x3
	.align 8
.LEFDE17:
	.ident	"GCC: (Debian 4.3.2-1.1) 4.3.2"
	.section	.note.GNU-stack,"",@progbits
