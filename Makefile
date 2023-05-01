# Set the make command to run silently in the terminal
.SILENT:

EXT=c
PNAME1=ttt
PNAME2=ttts

BINDIR=bin

# DEP1=lib/ttt_game
# DEP2=lib/server_controller
# DEP3=lib/client_controller

# Define a default target, to be built when invoking make
.PHONY: all
all: clean $(PNAME1) $(PNAME2)
	
# Define how to compile main
$(PNAME1): src/$(PNAME1).$(EXT)
	clang -o $(BINDIR)/$(PNAME1) src/$(PNAME1).${EXT} lib/*.c 

# ${DEP1}.c ${DEP3}.c

$(PNAME2): src/$(PNAME2).$(EXT)
	clang -o $(BINDIR)/$(PNAME2) src/$(PNAME2).${EXT} lib/*.c
# ${DEP2}.c

# Clean the workspace by removing output files 
.PHONY: clean
clean:
	rm -f $(BINDIR)/*

.PHONY: run_client
run_client:
	./$(BINDIR)/$(PNAME1)

.PHONY: run_server
run_server:
	./$(BINDIR)/$(PNAME2)

.PHONY: test
test:
	clang -o $(BINDIR)/test src/test.c lib/*.c lib/test/*.c
	./$(BINDIR)/test