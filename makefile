LIBS = -lglad -lglfw
NAME = test

test: test.o load_obj.o makefile stb_image.h
	g++ test.o load_obj.o -o $(NAME) $(LIBS) -g
	systemctl stop keyd
	./test
	rm test
	systemctl start keyd

load_obj.o: load_obj.cpp
	g++ load_obj.cpp -c

test.o: test.cpp
	g++ test.cpp -c

stb_image.h:
	wget https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h

load_obj.h:
	wget https://raw.githubusercontent.com/hugocotoflorez/crane/refs/heads/main/load_obj.h

load_obj.cpp:
	wget https://raw.githubusercontent.com/hugocotoflorez/crane/refs/heads/main/load_obj.cpp

setShaders.h:
	wget https://raw.githubusercontent.com/hugocotoflorez/crane/refs/heads/main/setShaders.h

