dwmstatus: dwmstatus.c dwmstatus.h
	gcc -g -o dwmstatus dwmstatus.c -lX11 `pkg-config --cflags --libs libmpdclient`

clean:
	rm dwmstatus
