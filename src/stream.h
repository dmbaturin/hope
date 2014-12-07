#ifndef STREAM_H
#define STREAM_H

#include "defs.h"

extern	Cell	*open_stream(Cell *arg);
extern	Cell	*read_stream(Cell *cell);
extern	void	reset_streams(void);
extern	void	close_streams(void);

#endif
