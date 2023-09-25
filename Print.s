; Print.s
; Student names: Avinash Bhaskaran, Aryan Jain
; Last modification date: 3/29/22
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
; R0=0,    then output "0"
; R0=3,    then output "3"
; R0=89,   then output "89"
; R0=123,  then output "123"
; R0=9999, then output "9999"
; R0=4294967295, then output "4294967295"
count EQU 0
num   EQU 4
LCD_OutDec
      PUSH{R4-R8, LR}
      SUB SP, #8                ; allocate space
      MOV R11, SP               ; R11 - stack frame
      
      MOV R4, #0                ; 
      STR R4, [R11, #count]     ; count = 0
      STR R0, [R11, #num]       ; num = R0 
      MOV R5, #10               ; R5 divisor for mod
      
oloop LDR R4, [R11, #num]       
                                ; push num%10 on stack (ones place)
      UDIV R6, R4, R5           ; R6 = num/10 (new num)
      MUL R7, R6, R5            ; 
      SUB R7, R4, R7            ; R7 = num - quotient*10 (remainder)
      PUSH{R7-R8}               ; digit on top of stack
      
      STR R6, [R11, #num]       ; num /= 10
      LDR R7, [R11, #count]     ; count++  
      ADD R7, #1                           
      STR R7, [R11, #count]
      
      CMP R6, #0                ; stop when num = 0, no digits left
      BEQ next
      B oloop
      
next  LDR R4, [R11, #count]     ; if count = 0 finish
      CMP R4, #0
      BEQ done
      
      POP {R0,R8}               ; get new digit in R0
      ADD R0, #0x30             ; offset for ASCII
      BL ST7735_OutChar         ; write digit to display
      SUB R4, #1
      STR R4, [R11, #count]     ; count--
      B next
      
done  ADD SP, #8                ; deallocate variables
      POP{R4-R8, PC}            
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000"
;       R0=3,    then output "0.003"
;       R0=89,   then output "0.089"
;       R0=123,  then output "0.123"
;       R0=9999, then output "9.999"
;       R0>9999, then output "*.***"
; Invariables: This function must not permanently modify registers R4 to R11
; R1 NUMBER OF DIGITS DESIRED IN FORMAT
max    EQU 9999         ; MAX NUMBER TO BE DISPLAYED (could be 10^4 - 1)
; R2 WHERE TO PLACE FIXED POINT IE # NON DECIMAL DIGITS
star   EQU 42           ; '*'
dot    EQU 46           ; '.'
LCD_OutFix
        PUSH{R4-R10, LR}
        
        SUB SP, #8                  ; allocate space
        MOV R11, SP                 ; R11 - stack frame pointer
        
        MOV R8, R1
        MOV R9, R2
        MOV R4, R8                  ; count = #digits
        STR R4, [R11, #count]
        STR R0, [R11, #num]         ; num = R0
        MOV R5, #10                 ; for mod
        
        LDR R4, [R11, #num]         ; go to over if num > 9999
        MOV R0, #max
        CMP R4, R0
        BHI over
; This will place each ASCII digit on the stack (most significant on top)
floop   LDR R4, [R11, #num]                                           ; push num%10 on stack (ones place)
        UDIV R6, R4, R5             ; R6 = num/10 (new num)
        MUL R7, R6, R5            
        SUB R7, R4, R7              ; R7 = num - quotient*10 (remainder)
        ADD R7, #0x30               ; offset for ASCII
        PUSH{R7-R8}                 ; ASCII digit on top of stack
      
        STR R6, [R11, #num]         ; num /= 10
        LDR R7, [R11, #count]      
        SUBS R7, #1                           
        STR R7, [R11, #count]       ; count-- 
        
        BNE floop                   ; continue until all digits are on stack
; Writes values on stack to screen with added fixed point where specified      
out     LDR R4, [R11, #count]       ; if count = digits, finish
        CMP R4, R8            
        BEQ fin
      
        POP {R0,R8}                 ; get new digit in R0
        BL ST7735_OutChar           ; write digit to display
        
        ADDS R4, #1
        STR R4, [R11, #count]       ; count++
        
        CMP R4, R9                  ; put fixed point after #whole numbers
        BNE out
        
        MOV R0, #dot                 ; output '.'
        BL ST7735_OutChar
        B out
; Fills stack with #digit '*' so it can be output
over    MOV R0, #star
        PUSH{R0, R8}
        
        LDR R4, [R11, #count]       ; count--
        SUBS R4, #1
        STR R4, [R11, #count]
        
        BNE over
        B out
        
fin     ADD SP, #8                ; deallocate variables
        POP{R4-R10, PC}            
        
        
        
     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
