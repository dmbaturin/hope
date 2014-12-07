#include "defs.h"
#include "table.h"
#include "newstring.h"

global void
t_init(Table *table)
{
	table->t_front = NULL;
	table->t_end = &(table->t_front);
}

global void
t_insert(table, element)
	Table	*table;
	TabElt	*element;
{
	*(table->t_end) = element;
	table->t_end = &(element->t_next);
	element->t_next = NULL;
}

global void
t_delete(table, element)
	Table	*table;
	TabElt	*element;
{
	TabElt	**ep;

	for (ep = &(table->t_front); *ep != NULL; ep = &((*ep)->t_next))
		if (*ep == element) {
			*ep = (*ep)->t_next;
			if (*ep == NULL)
				table->t_end = ep;
			return;
		}
}

global void
t_copy(Table *table1, const Table *table2)
{
	if (table2->t_front == NULL)
		t_init(table1);
	else {
		table1->t_front = table2->t_front;
		table1->t_end = table2->t_end;
	}
}

global TabElt *
t_lookup(const Table *table, String name)
{
	TabElt	*elem;

	for (elem = table->t_front; elem != NULL; elem = elem->t_next)
		if (elem->t_name == name)
			return elem;
	return NULL;
}

global void
t_foreach(const Table *table, TableAction *action)
{
	TabElt	*elem;

	for (elem = table->t_front; elem != NULL; elem = elem->t_next)
		(*action)(elem);
}
