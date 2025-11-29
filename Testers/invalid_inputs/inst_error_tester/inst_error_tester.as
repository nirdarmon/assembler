;general label dec
check: .data 1
;invalid mov
mov r1
mov r1, #5
;valid mov
mov r1, r2
;cmp all is valid
cmp check, check
;invalid add
add r1 , #-5
;valid add
add r1, r2
;invalid sub
sub r1 , #-5
;valid sub
sub r1, r2
;invalid leas
lea #3, r1
lea r3, r1
lea *r3, r1
lea 
;invalid clr || -> dec
clr r1, r2
clr #3
clr r1, r2
jmp r5
jmp #4
bne r5
bne #0
red #9
prn r1, r2
rts check
stop r1

;check if use of label that dosent exsist
mov #10, nir
mov #10, check


