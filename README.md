```
$ ./texcheck.exe -h
Usage: texcheck [--help] [--version] [--no-channels] [--format] [--compression]
[--resolution] [--mips] [--size] [--bytes] [files]...

Report channel, compression, format, resolution, mip and size datasets for textures. Channels are shown unless switched off; the rest are opt-in.

Positional arguments:
  files          texture files or glob patterns [nargs: 0 or more]

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits
  --no-channels  omit the channel layout, which is otherwise always shown
  --format       add the container format dataset
  --compression  add the compression dataset
  --resolution   add the pixel resolution dataset
  --mips         add the mip level count dataset
  --size         add the file size dataset
  --bytes        render sizes as exact byte counts instead of KiB/MiB
```
