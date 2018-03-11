# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

TARGET=lab3b

$(TARGET): lab3b.py
	rm -rf lab3b
	ln lab3b.py lab3b
	chmod +x lab3b

dist: lab3b.py Makefile README
	tar -cvzf lab3b-804598987.tar.gz $^

clean:
	rm -rf lab3b-804598987.tar.gz lab3b
