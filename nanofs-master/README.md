# nanofs
nano file system: Operating System Design example of minimum file system

## Compile
  * make clean
  * make

## Execute included example
  * dd if=/dev/zero of=tmpfile.dat bs=1024 count=32
  * ./nanofs

```bash
     * nanofs_mkfs() -> 1
     * nanofs_mount() -> 1
     * nanofs_creat('test1.txt') -> 0
     * nanofs_write(0,'hola mundo...',13) -> 13
     * nanofs_close(13) -> 1
     * nanofs_open('test1.txt') -> 0
     * nanofs_read(0,'',13) -> 13 (hola mundo...)
     * nanofs_close(13) -> 1
     * nanofs_unlink('test1.txt') -> 1
     * nanofs_umount() -> 1

