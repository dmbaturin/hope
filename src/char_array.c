#include "defs.h"
#include "char_array.h"
#include "memory.h"
#include "char.h"

struct _CharArray {
	Natural		ca_size;	/* no. of indices assigned to */
	SChar		*ca_index;	/* ordered array of length size */
	ArrayElement	*ca_value;	/* corresponding array of values */
	ArrayElement	ca_default;	/* value of the rest */
};

#define	MIN_POWER	1
#define	MIN_SIZE	(1<<MIN_POWER)

/*
 * Allocation strategy:
 * The allocated size of the array is not explicitly stored, but is
 * computed as follows: it is the smallest power of 2 that is >= MIN_SIZE
 * and >= ca_size (the used size).  When the array is full, we allocate
 * a new array twice as long as the old one, and discard the old array.
 * This means an average space wastage of 177%, but tree schemes are
 * even worse.  This could be ameliorated by free lists, and/or by
 * changing the step factor, but that's a bit too tricky for me.
 */

local	Natural	ca_lookup(SChar *array, Natural size, Char c);
local	void	ca_insert(CharArray *array, Char c, ArrayElement x);

global CharArray *
ca_new(ArrayElement x)
{
	CharArray	*array;

	array = NEW(CharArray);
	array->ca_size = 0;
	array->ca_index = NEWARRAY(SChar, MIN_SIZE);
	array->ca_value = NEWARRAY(ArrayElement, MIN_SIZE);
	array->ca_default = x;
	return array;
}

global CharArray *
ca_copy(CharArray *array)
{
	CharArray	*new_array;
	Natural	n;

	new_array = NEW(CharArray);
	new_array->ca_size = array->ca_size;
	for (n = MIN_SIZE; n < array->ca_size; n *= 2)
		;
	new_array->ca_index = NEWARRAY(SChar, n);
	new_array->ca_value = NEWARRAY(ArrayElement, n);
	for (n = 0; n < array->ca_size; n++) {
		new_array->ca_index[n] = array->ca_index[n];
		new_array->ca_value[n] = array->ca_value[n];
	}
	new_array->ca_default = array->ca_default;
	return new_array;
}

/*
 *	Binary search for element in ordered array.
 *	Returns index of c in array if successful, otherwise size.
 */
local Natural
ca_lookup(SChar *array, Natural size, Char c)
{
	Natural	low, mid, high;

	low = 0;
	high = size;
	while (low != high) {
		/*
		 * Invariant:
		 *	0 <= n < low => array[n] < c
		 *	high <= n < size => array[n] <= c
		 * Bound function: high - low
		 */
		mid = (low+high)/2;
		if (array[mid] < c)
			low = mid+1;
		else
			high = mid;
	}
	return low < size && array[low] == c ? low : size;
}

global ArrayElement
ca_index(CharArray *array, Char c)
{
	Natural	n;

	n = ca_lookup(array->ca_index, array->ca_size, c);
	return n == array->ca_size ? array->ca_default : array->ca_value[n];
}

local void
ca_insert(CharArray *array, Char c, ArrayElement x)
{
	Natural	size;
	Natural	n;
	Byte	*new_index;
	ArrayElement	*new_value;

	/*
	 * The array is full if the size is >= MIN_SIZE and is a power
	 * of 2, which is what the following tricky '&' tests.
	 * In this case, and we double its allocated size.
	 */
	size = array->ca_size;
	if (size >= MIN_SIZE && (size&(size-1)) == 0) {
		new_index = NEWARRAY(SChar, size*2);
		new_value = NEWARRAY(ArrayElement, size*2);
		for (n = 0; n < size; n++) {
			new_index[n] = array->ca_index[n];
			new_value[n] = array->ca_value[n];
		}
		array->ca_index = new_index;
		array->ca_value = new_value;
	}
	/*
	 * Now that there is room, insert the new element in order.
	 */
	for (n = size; n > 0 && array->ca_index[n-1] > c; n--) {
		array->ca_index[n] = array->ca_index[n-1];
		array->ca_value[n] = array->ca_value[n-1];
	}
	array->ca_index[n] = c;
	array->ca_value[n] = x;
	array->ca_size = size+1;
}

global void
ca_assign(CharArray *array, Char c, ArrayElement x)
{
	Natural	n;

	n = ca_lookup(array->ca_index, array->ca_size, c);
	if (n < array->ca_size)
		array->ca_value[n] = x;
	else
		ca_insert(array, c, x);
}

global void
ca_map(CharArray *array, EltMap *f)
{
	Natural	n;

	for (n = 0; n < array->ca_size; n++)
		array->ca_value[n] = (*f)(array->ca_value[n]);
	array->ca_default = (*f)(array->ca_default);
}
