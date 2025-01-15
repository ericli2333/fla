; This example program checks if the input string is a binary palindrome.
; Input: a string of 0's and 1's, e.g. '1001001'

; the finite set of states
#Q = {cp,cpb,ml,movel,mover,copy,accept,clear,move_error,move_error1,clear_error,i,il,ill,ille,illeg,illega,illegal,illegal_,illegal_i,illegal_in,illegal_inp,illegal_inpu,illegal_input,halt,error}

; the finite set of input symbols
#S = {a,b}

; the complete set of tape symbols
#G = {a,b,i,l,e,g,p,n,u,t}

; the start state
#q0 = cp

; the blank symbol
#B = _

; the set of final states
#F = {accept,error}

; the number of tapes
#N = 3

; the transition functions

;copy transitions
cp a__ a__ r** cp
cp b__ _b_ rr* cpb
cpb b__ _b_ rr* cpb
cpb a__ a__ l** move_error
cpb ___ ___ ll* movel

; move left to find the first a
move_error ___ a__ l** move_error
move_error a__ a__ l** move_error1
move_error1 a__ a__ l** move_error1
move_error1 ___ ___ r** clear_error
clear_error a__ ___ r** clear_error
clear_error ___ ___ *** i

; write illegal_input to the first tape
i ___ i__ r** il
il ___ l__ r** ill
ill ___ l__ r** ille
ille ___ e__ r** illeg
illeg ___ g__ r** illega
illega ___ a__ r** illegal
illegal ___ l__ r** illegal_
illegal_ ___ ___ r** illegal_i
illegal_i ___ i__ r** illegal_in
illegal_in ___ n__ r** illegal_inp
illegal_inp ___ p__ r** illegal_inpu
illegal_inpu ___ u__ r** illegal_input
illegal_input ___ t__ *** error

; move left to find the first a
movel _b_ _b_ l** movel
movel ab_ ab_ *** ml

; multiply transitions
ml ab_ abc l*r ml
ml _b_ ___ rl* mover

; move a to the rightest
mover a__ ___ r*l clear
mover ab_ ab_ r** mover
mover _b_ _b_ l** ml

; clear tape 1
clear a_c __c r** clear
clear __c __c *** copy

; copy the string to the first tape
copy a__ ___ r** copy
copy __c c__ r*l copy
copy ___ ___ *** accept
