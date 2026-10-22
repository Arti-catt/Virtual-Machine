in  ; enter number
popr ax
in
popr bx

pushr ax
pushr bx
call :2
pushr ax
out
hlt

:2
add
popr ax
ret