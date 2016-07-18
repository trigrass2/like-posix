changes:

ff.c, line 3994:
	downcast FIL* type to FATFS* to avoid warning
	
	
ff.c, line 5275:
	replaced constant '128' with '((opt & FM_SFD) ? 64 : 128)', to allow ramfs usage on targets with < 64kB ram.
	