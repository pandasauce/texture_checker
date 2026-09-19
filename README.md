```
$ texcheck -h
Usage: texcheck [--help] [--version] [--no-compression] [--format] [--channels]
[--resolution] [--mips] [--size] [--bytes] [files]...

Report format, channel, compression, resolution, mip and size datasets for textures. The compression is shown unless switched off; the rest are opt-in.

Positional arguments:
  files             texture files or glob patterns [nargs: 0 or more]

Optional arguments:
  -h, --help        shows help message and exits
  -v, --version     prints version information and exits
  --no-compression  omit the compression, which is otherwise always shown
  --format          add the container format dataset
  --channels        add the channel layout dataset
  --resolution      add the pixel resolution dataset
  --mips            add the mip level count dataset
  --size            add the file size dataset
  --bytes           render sizes as exact byte counts instead of KiB/MiB
```
