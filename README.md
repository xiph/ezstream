# About

Ezstream is a command line source client for media streams, primarily for
streaming to Icecast servers.

It allows the creation of media streams based on input from files or standard
input that is piped through an optional external de- and encoder. As every
part of this chain is highly configurable, ezstream can be useful in a large
number of streaming setups.

It uses libshout to communicate with streaming servers and currently supports
Ogg, MP3, WebM, and Matroska streams using the HTTP, ICY, and RoarAudio
protocols. It uses TagLib to read and manage metadata in numerous media
files.

Ezstream is free software and licensed under the GNU General Public License.
See the `COPYING` file for details.


# Dependencies

Ezstream depends on:

 - [libshout](https://www.icecast.org/download/) version 2.2.x or newer
 - [libxml](http://xmlsoft.org/) version 2.x
 - [TagLib](https://taglib.github.io/) for C version 1.x (1.4 or newer
   recommended)

Ezstream optionally uses:

 - [libiconv](https://www.gnu.org/software/libiconv/) on systems where
   `iconv()` is not available in libc, for basic non-ASCII charset support in
   metadata and filenames

Building ezstream depends on:

 - [check](https://libcheck.github.io/check) unit testing framework for C

# Installation

The ezstream software uses the GNU auto-tools to configure, build, and
install on a variety of systems. Several (additional) configuration options
are available.

Run `./configure --help` to get an overview.

The compilation and installation process then boils down to the usual

```console
    $ ./configure --help | less         # Skim over the available options
    $ ./configure [options] && make && [sudo] make install
                                        # Configure, build and install
                                        # [as root] the software
```

If this procedure is unfamiliar to you, please consult the `INSTALL` file for
more detailed instructions.

On systems where the libshout installation does not include the required
shout.pc file for `pkg-config(1)`, the non-standard `shout-config` utility
is available. However, the ezstream build system does not support the latter.

If this is an issue, configure ezstream with

```console
    $ ./configure \
        LIBSHOUT_CPPFLAGS="$(shout-config --cppflags)" \
        LIBSHOUT_CFLAGS="$(shout-config --cflags-only)" \
        LIBSHOUT_LIBS="$(shout-config --libs)"
```

If needed, verbose configuration error messages can be found in the
`config.log` file.

When working on a fresh checkout from source control, the `autogen.sh` script
must be run first. It requires automake, autoconf, libtool, and gettext.


# Usage

Once ezstream is installed successfully, type `man ezstream` on the command
line for a comprehensive manual. This distribution package also comes with
example configuration files that can be used as a guide to configure
ezstream.

As ezstream is a source client, a media streaming server like Icecast
must be available to distribute the stream to listening clients. See
(https://www.icecast.org/) for more information and resources.


# External Decoders/Encoders

Ezstream should be able to work with any media decoder and encoder that
fulfills the following requirements:

 1. Must be an executable that can be run from the command line without the
    need for a graphical display or terminal
 2. Decoding software must be able to either
    1. write raw audio data to standard output, or
    2. write a properly encoded stream ready for distribution to standard
       output
 3. Encoding software must be able to read raw audio data from standard input

The following incomplete list of programs shows a few that are known to work.
These are also used in the example configuration files:

 - MP3
   - Decoder: madplay (https://www.underbit.com/products/mad/)
   - Encoder: lame (https://lame.sourceforge.io/)

 - Ogg Vorbis (https://xiph.org/vorbis/):
   - Decoder: oggdec
   - Encoder: oggenc

 - FLAC (https://xiph.org/flac/):
   - Decoder: flac
   - Encoder: flac (requires Ogg FLAC output)

 - Ogg Theora:
   - Decoder/Encoder: ffmpeg2theora (http://v2v.cc/~j/ffmpeg2theora/)


# Development

 - Xiph.org Gitlab: https://gitlab.xiph.org/xiph/ezstream/
 - GitHub mirror: https://github.com/xiph/ezstream/
 - Travis CI: https://travis-ci.org/xiph/ezstream/
 - CircleCI: https://circleci.com/gh/xiph/ezstream/
 - Codecov: https://codecov.io/gh/xiph/ezstream/
 - Coverity: https://scan.coverity.com/projects/xiph-ezstream/
