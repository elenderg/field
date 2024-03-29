;
; File: drivers/keyboard.inc
;
; Desri��o:
;     Driver de teclado em 32bit para o Boot Manager.
;
; Obs: Teclado � muito importante para o Shell e gerencia de arquivos.
;
; In this file:
;     + Defini��es e vari�veis.
;     + Tabelas de teclas min�sculas e mai�sculas.
;     + Keyboard interrupt handler. (irq1.)
;     + Rotinas de inicializa��o de teclado.
;     + Rotinas de reboot, via teclado.
;     + Rotinas de shutdown (@todo: Retirar daqui.).
;
; History:
;     2015 - Created by Fred Nora.
;


;
; Defines.
;


;Keys.
%define KEY_A           0x01
%define KEY_N           0x02
%define KEY_Z           0x04
%define KEY_ESC         0x08    ;Escape.
%define KEY_ENTER       0x10    ;Enter.
%define KEY_RIGHTSHIFT  0x20    ;Right Shift.
%define KEY_SPACEBAR    0x40    ;Space.

;Scancodes.
%define SCAN_A           0x1e
%define SCAN_N           0x31
%define SCAN_Z           0x2c
%define SCAN_ESC         0x01    ;Scape.
%define SCAN_ENTER       0x1c    ;Enter.
%define SCAN_RIGHTSHIFT  0x36    ;Right Shift.
%define SCAN_SPACEBAR    0x39    ;Space.



;
; Vari�vel global.
;
 
global _g_scancode
_g_scancode:    dd 0


;
; Vari�veis.
;

;Debug support.
debug_char:     db "k"
debug_color:    db 9


_key_status:        db 0    ;Novo char.
nova_tecla_status:  db 0    ;Status. Se h� uma nova tecla pressionada ou n�o.



;Min�stulas.
keylayoutlower:
	db 0x00,   
	db 0, 
	db '1', 
	db '2', 
	db '3', 
	db '4', 
	db '5', 
	db '6', 
	db '7', 
	db '8', 
	db '9', 
	db '0', 
	db '-', 
	db '=', 
	db 0x0e    ;BACKSPACE  
	db 0x0F,   ;TAB 
	db 'q', 
	db 'w', 
	db 'e', 
	db 'r', 
	db 't', 
	db 'y', 
	db 'u', 
	db 'i', 
	db 'o', 
	db 'p', 
	db '�', 
	db '[', 
	db 0x1c    ;ENTER 
	db 0x1d,   ;CNTROL 
	db 'a', 
	db 's', 
	db 'd', 
	db 'f', 
	db 'g', 
	db 'h', 
	db 'j', 
	db 'k', 
	db 'l', 
	db '�', 
	db '~', 
	db '.',    ;?
	db '~', 
	db ']'  
	db 'z', 
	db 'x', 
	db 'c', 
	db 'v', 
	db 'b', 
	db 'n', 
	db 'm', 
	db ',', 
	db '.', 
	db ';', 
	db KEY_RIGHTSHIFT, 
	db '*', 
	db 0x38,    ;alt grado
	db ' ',     ;space
	db 0x3A,    ;caps lock
	db 0x3B,    ;F1
	db 0x3C,
	db 0x3D,
	db 0x3E,
	db 0x3F,
	db 0x40,    ;F6
	db 0x41,
	db 0x42,
	db 0x43,
	db 0x44,    ;F10
	db 0x45,    ;Pause
	db 0x46,    ;scroll
	db '7',
	db '8',
	db '9',
	db '-',
	db '4',
	db '5',
	db '6',
	db '+',
	db '1',
	db '2',
	db '3',
	db '0',
	db 0x53,    ;delete no teclado numerico.
	db 0x54,    ;print
	db '.',
	db '\\',
	db 0x57,    ;F11
	db 0x58,    ;F12
	db '/',
	db 0



;Mai�sculas
keylayoutupper:
  db 0, 0, '!', '@', '#', '$', '%', '�', '&', '*', '(', ')', '_', '+', 0x0e 
  db    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0x1c 
  db    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '�', 0x22, '~' 
  db    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', ':', 0, 0, 0, ' ', 0



; 0e = backspace
; 1c = enter



;;==================================================================
; keyboard:
;     Keyboard interrupt. (IRQ 0x01), (INT 0x21)

;++
keyboard:
    ;nop
_irq1:

    push eax
    push ebx

	;Debug.
    ;mov al, byte [debug_char]
    ;mov byte [0xB8002], al
    ;inc byte [debug_char]
	
	;For debug to see if system is still running.
	;call keyboard_debug

	;Get the scancode from the keyboard.
	
	xor eax, eax
	in al, 0x60
	
	;@todo: Escape, volta para o Shell.
	;cmp al, SCAN_ESC
	
	cmp al, 0x2A		   ;Left Shift Make.
	je keyboard_shift
	cmp al, 0x36		   ;Right Shift Make.
	je keyboard_shift
	cmp al, 0xAA		   ;Left Shift Break.
	je keyboard_noshift
	cmp al, 0xB6	       ;Right Shift Break.
	je keyboard_noshift
	
	;Testa o scan code para keyup ou keydown.
	
	;Key down.
	test al, 0x80
	jz keydown
	;Key up.
	jmp keyup
	
;=======
;Keydown.
keydown:
	cmp byte [key_shift], 0x00
	jne keyboard_lowercase
	jmp keyboard_uppercase
;Min�sculas.
keyboard_lowercase:
	mov ebx, keylayoutupper
	jmp keyboard_processkey
;Mai�sculas.
keyboard_uppercase:
	mov ebx, keylayoutlower
;Convert the scancode.
keyboard_processkey:
	add ebx, eax
	mov bl, [ebx]
	mov [key], bl    ;Pega a tecla.
	mov al, [key]
	jmp keyboard_done
	
;======
;Keyup.
keyup:
	jmp keyboard_done
;Aciona o shift.
keyboard_shift:
	mov byte [key_shift], 0x01
	jmp keyboard_done
;Desliga o shift.
keyboard_noshift:
	mov byte [key_shift], 0x00
	jmp keyboard_done
;Done.
keyboard_done:

    ;Put the typed character in the bottom right hand corner.
    mov byte [0xB8F9C], al

	;Debug.
	;mov al, byte "."
	;call write_char
	
	;Avisa que tem nova tecla.
	
    mov al, byte 1
    mov byte [nova_tecla_status], al

    mov al, 0x20
    out 0x20, al

    pop ebx
    pop eax
    iretd



;;=============================================
; get_key:
;     Pega mensagem do teclado.

get_key:

;Pega a nova tecla.

    mov al, byte [key]

;Muda o status.

    push bx
    mov bl, byte 0
    mov byte [nova_tecla_status], bl
    pop bx

    RET



;------------------------
; pega_status_nova_tecla:
;
	
pega_status_nova_tecla:

    ;V� o status.
    mov al, byte [nova_tecla_status]
	
	cmp al, byte 1
	je .nova_tecla
	
	xor ax, ax
	ret	
	
.nova_tecla:
    mov al, byte 1	
	ret

 
 
;------------------------------
; wait_key:
;     Espera pressionar uma tecla.
	
wait_key:  

	push edx
	
	;ve se tem uma nova tecla na fila.
    mov dl, byte [nova_tecla_status]

	;temos uma tecla
	cmp dl, byte 1
	je .do_exit
	
	;nao ha nova tecla
	cmp dl, byte 0
	je wait_key
	
	;;#bugbug
	;;tentando um default.
	
	jmp wait_key
	
.do_exit:
	;pega a tecla
	mov al, byte [key]
	; muda o status
	mov byte [nova_tecla_status], byte 0

    pop edx
    ret




;============================================================
; keyboardInit:
;     Set keyboard repeat rate to max.

	;; #bugbug
	;; Cancelado.
	;; N�o precisamos disso, pois vamos configurar o teclado 
	;; durante a inicializa��o do i8042 no kernel.

keyboardInit:
	;push ax
			
	;Set Typematic Rate/Delay.
	;mov al, 0xf3
	;out 0x60, al

	;30 cps and .25 second delay.
	;xor al, al
	;out 0x60, al

	;Set/Reset LEDs.
	;mov al, 0xed
	;out 0x60, al

	;All off.
	;xor al, al
	;out 0x60, al

	;pop ax
	RET


;===========================================================
;; reboot:
;;     Reboot the computer via keyboard.

reboot:
    ;nop
keyboardReboot:
    ;Wait for an empty Input Buffer.
    in al, 0x64
    test al, 00000010b
    jne reboot
	;Send the reboot call to the keyboard controller.
    mov al, 0xFE
    out 0x64, al
    jmp reboot
    jmp $


;
; End.
;

