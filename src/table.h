#ifndef TABLE_H
#define TABLE_H

#include "defs.h"
#include "newstring.h"

typedef	struct	_TabElt TabElt;

typedef	struct {
	TabElt	*t_front;
	TabElt	**t_end;
} Table;

/* Make this the first field in your structure */
struct _TabElt {
	String	t_name;
	TabElt	*t_next;
};

typedef	void	TableAction(TabElt *elem);

extern	void	t_init(Table *table);
extern	void	t_insert(Table *table, TabElt *element);
extern	void	t_delete(Table *table, TabElt *element);
extern	void	t_copy(Table *table1, const Table *table2);
extern	TabElt	*t_lookup(const Table *table, String name);
extern	void	t_foreach(const Table *table, TableAction *action);

#endif
