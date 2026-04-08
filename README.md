# smokenrg

From http://sed.free.fr/

Because you never know when free will erase a useful resource, here's smokenrg

    A little hack to extract audio files from an NRG file (proprietary format of the Nero software).

Released under Public Domain

# compiling

I added a makefile, type make, or compile the C file directly. It works.

# How to use the resulting raw files?

Raw files are raw audio files. Convert to the format of your liking (wav, flac, mp3, etc) with ffmpeg:

    ffmpeg -f s16le -vn -ar 44100 -ac 2 -i 001.raw 001.wav

