pathName = "PATH=$$PATH:"
pathName := $(pathName)$(CURDIR)
myDir = /home/$(USER)/ShellFiles
pathAddtion = /home/$(USER)/.bashrc


all: setPath makeDirectory
	gcc -std=c11 -o ShellInterface main.c Command.c Utility.c Parser.c Constants.c System.c Expression.c
	

setPath:
	@echo $(pathName) >> $(pathAddtion)
	@echo $(pathName)

makeDirectory:
	mkdir -p $(myDir)
	touch $(myDir)/history.txt
	

clean:
	rm -r $(myDir)
	rm Shell
	head -n -1 $(pathAddtion) > temp.txt 
	mv temp.txt $(pathAddtion) 
