in
popr ax
in
popr bx
in
popr cx
call :SolveSq

:SolveSq
pushr ax
push 0
je :AXisZero
call :SolveQuadroSq


:AXisZero
pushr bx
push 0
je :AXandBXisZero
call :SolveLineSq

:AXandBXisZero
pushr cx
push 0
je :InfinityRoots
hlt

:SolveLineSq
push 0
pushr cx
sub
pushr bx
div
out
hlt

:SolveQuadroSq
pushr bx
pushr bx
mul
push 4
pushr ax
pushr cx
mul
mul
sub
popr dx

pushr dx
push 0
jb :ZeroRoots

pushr dx
push 0
je :OneRoot

pushr dx
push 0
ja :TwoRoots

:InfinityRoots
push inf
out
hlt

:ZeroRoots
push 0
out
hlt

:OneRoot
push 1
out

push -1
pushr bx
push 2
pushr ax
mul
div
mul

out
hlt

:TwoRoots
pushr dx
sqrt
popr dx

push 2
out

push 0
pushr bx
sub
pushr dx
sub
push 2
pushr ax
mul
div
out

push 0
pushr bx
sub
pushr dx
add
push 2
pushr ax
mul
div
out

hlt