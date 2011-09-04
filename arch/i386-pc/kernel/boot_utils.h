#include <utility/tagitem.h>

void *RelocateBootMsg(const struct TagItem *msg, struct TagItem *dest);
void RelocateTagData(struct TagItem *tag, unsigned long size);
void RelocateStringData(struct TagItem *tag);
void RelocateBSSData(struct TagItem *tag);
