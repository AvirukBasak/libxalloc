/* Produced by texiweb from libavl.w on 2002/01/06 at 19:10. */

/* libavl - library for manipulation of binary trees.
   Copyright (C) 1998-2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

   The author may be contacted at <blp@gnu.org> on the Internet, or
   as Ben Pfaff, 12167 Airport Rd, DeWitt MI 48820, USA through more
   mundane means.
*/

#include <stdio.h>

#define MAX_INPUT 1024

/* Returns the smallest |i| such that |array[i] == key|,
   or |-1| if |key| is not in |array[]|.
   |array[]| must be an array of |n int|s. */
int
seq_search (int array[], int n, int key)
{
  int i;

  for (i = 0; i < n; i++)
    if (array[i] == key)
      return i;
  return -1;
}


int
main (void)
{
  int array[MAX_INPUT];
  int n, i;

  for (n = 0; n < MAX_INPUT; n++)
    if (scanf ("%d", &array[n]) != 1)
      break;

  for (i = 0; i < n; i++)
    {
      int result = seq_search (array, n, array[i]);
      if (result != i)
        printf ("seq_search() returned %d looking for %d - expected %d\n",
                result, array[i], i);
    }

  return 0;
}
