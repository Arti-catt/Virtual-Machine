in
popr ax
call :Factorial
out
hlt

:Factorial     ; Calculate factorial(ax)
pushr ax
push 1
jbe :1

pushr ax
pushr ax
push 1
sub
popr ax
call :Factorial
mul
ret
:1              ; if (ax < 1)
push 1
ret ;