in
popr ax
call :1
out
hlt

:1          ; Calculate factorial(ax)
pushr ax
push 1
jbe :2

pushr ax
pushr ax
push 1
sub
popr ax
call :1
mul
ret
:2          ; if (ax < 1)
push 1
ret