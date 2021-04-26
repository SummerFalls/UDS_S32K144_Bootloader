for /R %%f in (*.c;*.h) do AStyle.exe -A3 -s4 -xV -S -w -Y -M60 -f -p -xg -H -k3 -W3 -xb -j -xf -xh -c -xC200 -n -v %%f
pause