REPORT=-vec-report

all: clean miclord voodoo runner

broken:
	icc $(REPORT) -openmp -std=c99 broken.c -o broken

miclord: libmiclord.c
	icc $(REPORT) -openmp -fPIC -shared -std=c99 libmiclord.c -o libmiclord.so 

voodoo: libvoodoo.c
	icc $(REPORT) -openmp -fPIC -shared -std=c99 libvoodoo.c -o libvoodoo.so

runner: runner.c
	icc $(REPORT) -L./ -I./ -lmiclord -openmp -std=c99 runner.c -o runner

complx: complx.c
	icc $(REPORT) -I./ -openmp -std=c99 complx.c -o complx

clean:
	rm -f runner
	rm -f complx
	rm -f libvoodoo.so
	rm -f libmiclord.so
