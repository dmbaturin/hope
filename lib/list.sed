/^data list/c\
data listShape alpha beta == nil ++ alpha :: beta;\
type list alpha == listShape alpha (list alpha);
