; This example program checks if the input string is a binary palindrome.
; Input: a string of 0's and 1's, e.g. '1001001'

; the finite set of states
#Q = {substract1_1,substract1_2,increase,move,double,double1,substract,accept,accept2,accept3,accept4,halt_accept,reject,reject2,reject3,reject4,reject5,halt_reject}

; the finite set of input symbols
#S = {1}

; the complete set of tape symbols
#G = {1,_,t,r,u,e,f,a,l,s}

; the start state
#q0 = substract1_1

; the blank symbol
#B = _

; the set of final states
#F = {halt_accept}

; the number of tapes
#N = 3

; the transition functions

; State substract1: substract 1 from the 1st tape
substract1_1 1__ ___ r** substract1_2
substract1_2 1__ 1__ *** increase
substract1_2 ___ ___ *** accept

; increase transitions

increase 1__ 1_1 *** move

; move the third tape to the leftmost place
move 1_1 1_1 **l move
move 1__ 1__ **r double


; double transitions
double 1_1 111 *r* double1
double1 1_1 111 *rr double 
double 1__ 11_ *** substract

; substract transitions
substract 11_ ___ rl* substract
substract _1_ ___ *** reject
substract ___ ___ *** accept
substract 1__ 1__ *** increase

; State accept*: write 'true' on 1st tape
accept ___ t__ r** accept2
accept2 ___ r__ r** accept3
accept3 ___ u__ r** accept4
accept4 ___ e__ *** halt_accept

; State reject*: write 'false' on 1st tape
reject ___ f__ r** reject2
reject2 ___ a__ r** reject3
reject3 ___ l__ r** reject4
reject4 ___ s__ r** reject5
reject5 ___ e__ *** halt_reject
reject _1_ f1_ r** reject2
reject2 _1_ a1_ r** reject3
reject3 _1_ l1_ r** reject4
reject4 _1_ s1_ r** reject5
reject5 _1_ e1_ *** halt_reject