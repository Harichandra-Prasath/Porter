run: build
	@./builddir/Porterd 

build:
	@ninja -C ./builddir
