
all:
	doxygen Doxyfile

all-x86:
	( cat Doxyfile ; echo "PREDEFINED=__attribute__(x)=1 x86" ) | doxygen -

all-x86_64:
	( cat Doxyfile ; echo "PREDEFINED=__attribute__(x)=1 x86_64" ) | doxygen -

clean:
	if test -d output; then make -C ./output/latex clean; fi
	-rm -f doxygen_warnings.log

fclean: clean
	-rm -rf output/

