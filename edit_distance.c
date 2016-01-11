#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <mysql.h>
#include <limits.h>
#include <locale.h>

my_bool edit_distance_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  return 0;
}
  
void edit_distance_deinit(UDF_INIT *initid __attribute__((unused)))
{
}

#define DIST(ox,oy) (dists[((lx + 1) * (oy)) + (ox)])

uint32_t calc_edit_distance(char *x_start, char *x_end, char *y_start, char *y_end)
{
  uint32_t cx, lx, cy, ly;
  uint32_t *dists;
  uint32_t d = 0;
  char *px, *sx = x_start, *ex = x_end;
  char *py, *sy = y_start, *ey = y_end;
  unsigned int deletion_cost = 1;
  unsigned int insertion_cost = 1;
  unsigned int substitution_cost = 1;
  unsigned int transposition_cost = 1;
  setlocale(LC_ALL, "ja_JP.UTF-8");
  for (px = sx, lx = 0; px < ex && (cx = mblen(px, MB_LEN_MAX)); px += cx, lx++);
  for (py = sy, ly = 0; py < ey && (cy = mblen(py, MB_LEN_MAX)); py += cy, ly++);

  if ((dists = malloc((lx + 1) * (ly + 1) * sizeof(uint32_t)))) {
    uint32_t x, y;
    for (x = 0; x <= lx; x++) { DIST(x, 0) = x; }
    for (y = 0; y <= ly; y++) { DIST(0, y) = y; }
    for (x = 1, px = sx; x <= lx; x++, px += cx) {
      cx = mblen(px, MB_LEN_MAX);
      for (y = 1, py = sy; y <= ly; y++, py += cy) {
        cy = mblen(py, MB_LEN_MAX);
        if (cx == cy && !memcmp(px, py, cx)) {
          DIST(x, y) = DIST(x - 1, y - 1);
        } else {
          uint32_t a;
          uint32_t b;
          uint32_t c;
          a = DIST(x - 1, y) + deletion_cost;
          b = DIST(x, y - 1) + insertion_cost;
          c = DIST(x - 1, y - 1) + substitution_cost;
          DIST(x, y) = ((a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c));
          if (x > 1 && y > 1
              && cx == cy
              && memcmp(px, py - cy, cx) == 0
              && memcmp(px - cx, py, cx) == 0) {
            uint32_t t = DIST(x - 2, y - 2) + transposition_cost;
            DIST(x, y) = ((DIST(x, y) < t) ? DIST(x, y) : t);
          }
        }
      }
    }
    d = DIST(lx, ly);
    free(dists);
  }
  return d;
}

#define MAX_WORD_SIZE 64

uint32_t calc_edit_distance_bp(char *x_start, unsigned int x_len, char *y_start, unsigned int y_len)
{
  unsigned int i, j;
  const unsigned char *search;
  unsigned int search_length;
  const unsigned char *compared;
  unsigned int compared_length;
  uint64_t char_vector[UCHAR_MAX] = {0};
  uint64_t top;
  uint64_t VP = 0xFFFFFFFFFFFFFFFFULL;
  uint64_t VN = 0;
  uint32_t score = 0;
  if (x_len > MAX_WORD_SIZE || y_len > MAX_WORD_SIZE) {
    return calc_edit_distance(x_start, x_start+x_len, y_start, y_start + y_len);
  } else if (x_len >= y_len) {
    search = (const unsigned char *)x_start;
    search_length = x_len;
    compared = (const unsigned char *)y_start;
    compared_length = y_len;
  } else {
    search = (const unsigned char *)y_start;
    search_length = y_len;
    compared = (const unsigned char *)x_start;
    compared_length = x_len;
  }

  top = (1ULL << (search_length - 1));
  score = search_length;
  for (i = 0; i < search_length; i++) {
    char_vector[search[i]] |= (1ULL << i);
  }
  for (j = 0; j < compared_length; j++) {
    uint64_t D0 = 0, HP, HN;
    uint64_t PM[2];
    if (j > 0) {
      PM[0] = char_vector[compared[j - 1]];
    } else {
      PM[0] = 0;
    }
    PM[1] = char_vector[compared[j]];
    D0 = ((( ~ D0) & PM[1]) << 1ULL) & PM[0];
    D0 = D0 | (((PM[1] & VP) + VP) ^ VP) | PM[1] | VN;
    HP = VN | ~(D0 | VP);
    HN = VP & D0;
    if (HP & top) {
      score++;
    } else if (HN & top) {
      score--;
    }
    VP = (HN << 1ULL) | ~(D0 | ((HP << 1ULL) | 1ULL));
    VN = D0 & ((HP << 1ULL) | 1ULL);
  }
  return score;
}
  
char *edit_distance(UDF_INIT *initid __attribute__((unused)),
    UDF_ARGS *args, char *result, unsigned long *length,
    char *is_null, char *error __attribute__((unused)))
{
  if (!args->args[0] || !args->args[1]) {
    *is_null = 1;
    return NULL;
  }
  uint32_t d = 0;
  if (args->arg_count < 3) {
    d = calc_edit_distance(args->args[0], args->args[0] + args->lengths[0],
                           args->args[1], args->args[1] + args->lengths[1]);
  } else {
    d = calc_edit_distance_bp(args->args[0], args->lengths[0],
                           args->args[1], args->lengths[1]);
  }
  *is_null = 0;
  sprintf(result,"%d", d);
  *length = strlen(result);
  return result;
}
