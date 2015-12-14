changes:

ff.c, line 4070:
	replaced constant '128 with '((sfd) ? 64 : 128)', to allow ramfs usage on targets with < 64kB ram.