edit_distance:
	gcc -Wall -fPIC -I/usr/local/mysql/include -shared edit_distance.c -o edit_distance.so
install:
	install -m 755 edit_distance.so /usr/local/mysql/lib/plugin
uninstall:
	rm -f /usr/local/mysql/lib/plugin/edit_distance.so
clean:
	rm -f edit_distance.so
