#ifndef DISKIO_H
#define DISKIO_H

/*
 * Disk geometry constants
 */
#define TRACKS            32
#define SECTORS_PER_TRACK 64
#define SECTOR_SIZE       256
#define TRACK_SIZE        (SECTORS_PER_TRACK * SECTOR_SIZE)

#endif /* DISKIO_H */
