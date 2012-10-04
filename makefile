dwmstatus: dwmstatus.c
	gcc -g -o dwmstatus dwmstatus.c `pkg-config --cflags --libs x11 libmpdclient` -liw

clean:
	rm dwmstatus
