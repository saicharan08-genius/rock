#include <kernel/drivers/ahci.h>
#include <kernel/mm/kHeap.h>
#include <kernel/fs/vfs.h>
#include <lib/output.h>
#include <stddef.h>

namespace kernel {

void readPartitions() {
    if(partitions == NULL) {
        partitions = new partition[4];
    }

    void *sector0 = new uint8_t[0x200];
    ahci.sataRW(&ahci.drives[0], 0, 1, sector0, 0);

    mbrPartitionEntry *mbrEntries = (mbrPartitionEntry*)((uint64_t)sector0 + 0x1BE);
    
    cout + "[KDEBUG]" << mbrEntries[0].startingSector << "\n";

    partitions[0] = (partition) { EXT2, mbrEntries[0] };
}

}