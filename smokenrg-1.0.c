/* Public domain shit.
 *
 * Extract audio files from a nero "archive," dump in local directory
 * as 001.raw, 002.raw, ...
 *
 * http://sed.free.fr
 *
 * Only nero v2 files, only DAOX, only basic audio 44100Hz, 16b, stereo,
 * CD stuff. I try to print track names, but well, that plain sucks
 * (a supplementary track may be displayed with empty name; let's say
 * it's a bug in the code I'm too lazy to fix but not too much to describe
 * here, poor punk of me).
 *
 * Why do people use this crappy format, I don't know.
 *
 * To convert raw files, use for example ffmpeg:
 * ffmpeg -f s16le -vn -ar 44100 -ac 2 -i 001.raw 001.wav
 *
 * Fuck you nero.
 *
 * Do you like my coding style? Fuck coding styles.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define A printf("%s:%d\n", __FUNCTION__, __LINE__);

#define log(...) printf(__VA_ARGS__)

unsigned g32(unsigned char *b)
{
  return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

unsigned long g64(unsigned char *b)
{
  if (b[0] || b[1] || b[2] || b[3]) { printf("sorry I can't\n"); exit(1); }
  return g32(b+4);
}

char buf[4096];
void save_track(FILE *in, int i, unsigned long tstart, unsigned long tend)
{
  char n[32];
  FILE *out;
  unsigned long len = tend - tstart;
  long pos;

  pos = ftell(in);

  fseek(in, tstart, SEEK_SET);

  sprintf(n, "%3.3d.raw", i);
  out = fopen(n, "wb"); if (!out) { perror(n); exit(1); }
  while (len) {
    unsigned long d = len;
    if (d > 4096) d = 4096;
    if (fread(buf, d, 1, in) == -1) { perror("input file"); exit(1); }
    if (fwrite(buf, d, 1, out) == -1) { perror(n); exit(1); }
    len -= d;
  }
  if (fclose(out) == -1) { perror(n); exit(1); }

  fseek(in, pos, SEEK_SET);
}

int main(int n, char **v)
{
  FILE *in;
  unsigned char b[64];
  unsigned long offset;
  unsigned int size;

  if (n != 2) {
    printf("smokenrg <nero shit file>\n");
    return 1;
  }

  if (!(in = fopen(v[1], "rb"))) { A; perror(v[1]); return 1; }
  if (fseek(in, -12, SEEK_END) == -1) { A; perror(v[1]); return 1; }

  if (fread(b, 12, 1, in) != 1) { A; perror(v[1]); return 1; }

  if (b[0] != 'N' || b[1] != 'E' || b[2] != 'R' || b[3] != '5') {
    fprintf(stderr, "%s: unrecognized nrg file\n", v[1]);
    return 1;
  }

  offset = g64(b+4);
  log("offset of first chunk: %lu (0x%8.8lx)\n", offset, offset);

  if (fseek(in, offset, SEEK_SET) == -1) { A; perror(v[1]); return 1; }
  if (fread(b, 8, 1, in) != 1) { A; perror(v[1]); return 1; }
  size = g32(b+4);
  log("first chunk: %c%c%c%c (size %u)\n", b[0], b[1], b[2], b[3], size);

more:
  offset += size + 8;

  if (fseek(in, offset, SEEK_SET) == -1) { A; perror(v[1]); return 1; }
  if (fread(b, 8, 1, in) != 1) { A; perror(v[1]); return 1; }
  size = g32(b+4);
  log("chunk: %c%c%c%c (size %u)\n", b[0], b[1], b[2], b[3], size);

  b[4] = 0;

  if (!strcmp((const char *)b, "SINF")) {
    int val;
    if (fread(b, 4, 1, in) != 1) { A; perror(v[1]); return 1; }
    val = g32(b);
    log("# tracks = %d\n", val);
  } else if (!strcmp((const char *)b, "DAOX")) {
    int i;
    int tmin, tmax;
    if (fread(b, 22, 1, in) != 1) { A; perror(v[1]); return 1; }
    tmin = b[20];
    tmax = b[21];
    log("first track %d last track %d\n", tmin, tmax);
    for (i = tmin; i <= tmax; i++) {
      unsigned long tstart, tend;
      unsigned long pregap;
      unsigned int mode;
      unsigned int sector_size;
      if (fread(b, 42, 1, in) != 1) { A; perror(v[1]); return 1; }
      tstart = g64(b+10+4+4+8);
      tend = g64(b+10+4+4+8+8);
      pregap = g64(b+10+4+4);
      mode = g32(b+10+4);
      sector_size = g32(b+10);
      log("  track %d: start %lu end %lu mode %8.8x sect %u gap %lu\n",
          i, tstart, tend, mode, sector_size, pregap);
      save_track(in, i, tstart, tend);
    }
  } else if (!strcmp((const char *)b, "CDTX")) {
    int ss = size;
    int track = -1;
    int pos = ftell(in);
    while (ss > 0) {
      if (fread(b, 18, 1, in) != 1) { A; perror(v[1]); return 1; }
      log("type %2.2x %2.2x %2.2x %2.2x %c%c%c%c%c%c%c%c%c%c%c%c\n",
          b[0], b[1], b[2], b[3],
          b[4] == 0 ? '*' : b[4], b[5] == 0 ? '*' : b[5],
          b[6] == 0 ? '*' : b[6], b[7] == 0 ? '*' : b[7],
          b[8] == 0 ? '*' : b[8], b[9] == 0 ? '*' : b[9],
          b[10] == 0 ? '*' : b[10], b[11] == 0 ? '*' : b[11],
          b[12] == 0 ? '*' : b[12], b[13] == 0 ? '*' : b[13],
          b[14] == 0 ? '*' : b[14], b[15] == 0 ? '*' : b[15]);
      ss -= 18;
    }
    fseek(in, pos, SEEK_SET);
    ss = size;
    /* we suppose titles in order: album title, track 1, track 2, ... */
    while (ss > 0) {
      int i;
      if (fread(b, 18, 1, in) != 1) { A; perror(v[1]); return 1; }
      if (b[0] != 0x80) { ss -= 18; continue; }
      if (track == -1) { log("Album title: "); track = 0; }
      for (i = 4; i < 16; i++) {
        if (b[i] == 0) log("\nTrack %d: ", ++track);
        else log("%c", b[i]);
      }
      ss -= 18;
    }
    log("\n");
  } else if (!strcmp((const char *)b, "END!")) goto done;

  goto more;

done:

  return 0;
}
