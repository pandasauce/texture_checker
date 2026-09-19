```
$ texcheck -h
Usage: texcheck [--help] [--version] [--no-format] [--channels] [--compression]
[--resolution] [--mips] [--size] [--bytes] [files]...

Report format, channel, compression, resolution, mip and size datasets for textures. The format is shown unless switched off; the rest are opt-in.

Positional arguments:
  files          texture files or glob patterns [nargs: 0 or more]

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits
  --no-format    omit the container format, which is otherwise always shown
  --channels     add the channel layout dataset
  --compression  add the compression dataset
  --resolution   add the pixel resolution dataset
  --mips         add the mip level count dataset
  --size         add the file size dataset
  --bytes        render sizes as exact byte counts instead of KiB/MiB
```
