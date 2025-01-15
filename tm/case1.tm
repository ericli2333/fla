; This example program checks if the input string is a binary palindrome.
; Input: a string of 0's and 1's, e.g. '1001001'

; the finite set of states
#Q = {cp,ml,movel,mover,copy,accept,clear}

; the finite set of input symbols
#S = {a,b}

; the complete set of tape symbols
#G = {a,b,c}

; the start state
#q0 = cp

; the blank symbol
#B = _

; the set of final states
#F = {accept}

; the number of tapes
#N = 3

; the transition functions

;copy transitions
cp a__ a__ r** cp
cp b__ _b_ rr* cp
cp ___ ___ ll* movel

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
