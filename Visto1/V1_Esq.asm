* Coloque aqui sua matrícula e seu nome
* Sol = x, Coloque aqui a solução pretendida e a nota almejada

***************************
*** Visto 1 - Esqueleto ***
***************************

;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer


;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------

VISTO1:		MOV			#MSG_CLARA,R5
			MOV			#MSG_CIFR,R6
			CALL		#ENIGMA			;Cifrar
			;
			CALL		#RESETE			;Restaurar posição original
			;
			MOV			#MSG_CIFR,R5
			MOV			#MSG_DECIFR,R6
			CALL		#ENIGMA			;Decifrar
			JMP			$
			NOP

;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
; Coloque aqui sua sub-rotina ENIGMA %
;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
ENIGMA: ...
			;
			;

*********************************
******    Área de dados    ******
*********************************
		.data

; Chave = A, B, C, D, E, F, G
;A = número do rotor à esquerda e B = sua configuração;
;C = número do rotor central e    D = sua configuração;
;E = número do rotor à direita e  F = sua configuração;
;G = número do refletor.
;               A B   C D    E F   G
CHAVE:	.word	2,4,  5,8,   3,3,  2		;<<<===========

***********************************************
*** Área dos dados do Enigma. Não os altere ***
***********************************************
RT_TAM:	.word	26		;Tamanho
RT_QTD:	.word	05		;Quantidade de Rotores
RF_QTD:	.word	03		;Quantidade de Refletores

VAZIO:	.space 12		;Para facilitar endereço do rotor 1

;Rotores com 26 posições
ROTORES:
RT1:	.byte	20,  6, 21, 25, 11, 15, 16, 18,  0,  7,  1, 22,  9, 17, 24,  5,  8, 23, 19, 13, 12, 14,  3,  2, 10,  4
RT2:	.byte	12, 18, 25, 22,  2, 23,  9,  5,  3,  6, 15, 14, 24, 11, 19,  4,  8, 21, 17,  7, 16,  1,  0, 10, 13, 20
RT3:	.byte	23, 21, 18,  2, 15, 14,  0, 25,  3,  8,  4, 17,  7, 24,  5, 10, 11, 20, 22,  1, 12,  9, 16,  6, 19, 13
RT4:	.byte	22, 21,  7,  0, 16,  3,  4,  8,  2,  9, 23, 20,  1, 11, 25,  5, 24, 14, 12,  6, 18, 13, 10, 19, 17, 15
RT5:	.byte	20, 17, 13, 11, 25, 16, 23,  3, 19,  4, 24,  5,  1, 12,  8,  9, 15, 22,  6,  0, 21,  7, 14, 18,  2, 10

;Refletores com 26 posições
REFLETORES:
RF1:	.byte	14, 11, 25,  4,  3, 22, 20, 18, 15, 13, 12,  1, 10,  9,  0,  8, 24, 23,  7, 21,  6, 19,  5, 17, 16,  2
RF2:	.byte	 1,  0, 16, 25,  6, 24,  4, 23, 14, 13, 17, 18, 19,  9,  8, 22,  2, 10, 11, 12, 21, 20, 15,  7,  5,  3
RF3:	.byte	21,  7,  5, 19, 18,  2, 16,  1, 14, 22, 24, 17, 20, 25,  8, 23,  6, 11,  4,  3, 12,  0,  9, 15, 10, 13

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Área da mesagem em claro, cifrada e decifrada ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MSG_CLARA:
		.byte	"UMA NOITE DESTAS, VINDO DA CIDADE PARA O ENGENHO NOVO,"
		.byte	" ENCONTREI NO TREM DA CENTRAL UM RAPAZ AQUI DO BAIRRO,"
		.byte 	" QUE EU CONHECO DE VISTA E DE CHAPEU.@MACHADO\ASSIS",0

MSG_CIFR:
		.byte	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		.byte	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		.byte 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",0

MSG_DECIFR:
		.byte	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"
		.byte	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"
		.byte 	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Coloque aqui suas Variáveis ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;


;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
