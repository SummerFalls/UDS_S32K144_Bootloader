for /R %%f in (*.c;*.h) do AStyle.exe -A1 -s4 -xV -S -w -Y -M60 -f -p -xg -H -xe -k3 -W3 -xb -j -xf -xh -c -xC200 -n -v %%f
pause