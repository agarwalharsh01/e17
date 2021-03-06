/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "eina_suite.h"
#include "Eina.h"
#include "eina_safety_checks.h"

typedef struct _Eina_Test_Inlist Eina_Test_Inlist;
struct _Eina_Test_Inlist
{
   int i;
   EINA_INLIST;
};

static Eina_Test_Inlist *
_eina_test_inlist_build(int i)
{
   Eina_Test_Inlist *tmp;

   tmp = malloc(sizeof(Eina_Test_Inlist));
   fail_if(!tmp);
   tmp->i = i;

   return tmp;
}

START_TEST(eina_inlist_simple)
{
   Eina_Inlist *lst = NULL;
   Eina_Test_Inlist *tmp;
   Eina_Test_Inlist *prev;
   int i = 0;

   tmp = _eina_test_inlist_build(42);
   lst = eina_inlist_append(lst, EINA_INLIST_GET(tmp));
   fail_if(!lst);

   lst = eina_inlist_remove(lst, EINA_INLIST_GET(tmp));
   lst = eina_inlist_prepend(lst, EINA_INLIST_GET(tmp));

   tmp = _eina_test_inlist_build(1664);
   lst = eina_inlist_append_relative(lst, EINA_INLIST_GET(tmp), lst);
   fail_if(!lst);
   fail_if(EINA_INLIST_CONTAINER_GET(lst, Eina_Test_Inlist)->i != 42);

   prev = tmp;
   tmp = _eina_test_inlist_build(3227);
   lst = eina_inlist_prepend_relative(lst, EINA_INLIST_GET(
                                         tmp), EINA_INLIST_GET(prev));
   fail_if(!lst);
   fail_if(EINA_INLIST_CONTAINER_GET(lst, Eina_Test_Inlist)->i != 42);

   lst = eina_inlist_remove(lst, EINA_INLIST_GET(tmp));

   lst = eina_inlist_append_relative(lst, EINA_INLIST_GET(tmp), lst);
   lst = eina_inlist_remove(lst, EINA_INLIST_GET(tmp));

   lst = eina_inlist_prepend_relative(lst, EINA_INLIST_GET(tmp), lst);

   tmp = _eina_test_inlist_build(27);
   lst = eina_inlist_prepend_relative(lst, EINA_INLIST_GET(tmp), NULL);

   tmp = _eina_test_inlist_build(81);
   lst = eina_inlist_append_relative(lst, EINA_INLIST_GET(tmp), NULL);

   EINA_INLIST_FOREACH(lst, tmp)
   {
      switch (i)
        {
         case 0: fail_if(tmp->i != 27); break;

         case 1: fail_if(tmp->i != 3227); break;

         case 2: fail_if(tmp->i != 42); break;

         case 3: fail_if(tmp->i != 1664); break;

         case 4: fail_if(tmp->i != 81); break;
        }

      ++i;
   }

#ifdef EINA_SAFETY_CHECKS
   fprintf(stderr, "you should have a safety check failure below:\n");
   {
      Eina_Inlist *tmp2 = eina_inlist_remove(NULL, EINA_INLIST_GET(tmp));
      fail_if(tmp2 != NULL);
      fail_if(eina_error_get() != EINA_ERROR_SAFETY_FAILED);
   }

      fprintf(stderr, "you should have a safety check failure below:\n");
   lst = eina_inlist_remove(lst, NULL);
   fail_if(eina_error_get() != EINA_ERROR_SAFETY_FAILED);
#endif

   tmp = EINA_INLIST_CONTAINER_GET(lst, Eina_Test_Inlist);
   lst = eina_inlist_demote(lst, lst);
   fail_if(EINA_INLIST_CONTAINER_GET(lst, Eina_Test_Inlist) == tmp);

   lst = eina_inlist_promote(lst, EINA_INLIST_GET(tmp));
   fail_if(lst != EINA_INLIST_GET(tmp));

   tmp = EINA_INLIST_CONTAINER_GET(eina_inlist_find(lst, EINA_INLIST_GET(
                                                       prev)), Eina_Test_Inlist);
   lst = eina_inlist_remove(lst, EINA_INLIST_GET(tmp));
   prev = (Eina_Test_Inlist *)eina_inlist_find(lst, EINA_INLIST_GET(tmp));
   tmp = prev ? EINA_INLIST_CONTAINER_GET(prev, Eina_Test_Inlist) : NULL;
   fail_if(tmp != NULL);

   while (lst)
      lst = eina_inlist_remove(lst, lst);
}
END_TEST

void
eina_test_inlist(TCase *tc)
{
   tcase_add_test(tc, eina_inlist_simple);
}
