#include <partition.h>
#include <stdint.h>
#include <platform/errno.h>
#include "pmt.h"
#include <string.h>
#include <malloc.h>
#include <printf.h>
#include <target.h>
#include <part_private.h>

//#include "part_private.h"

#define DBGMSG(...)
#define ERRMSG(...)
#ifdef MTK_EMMC_SUPPORT
#define PMT 1
#else
#define PMT 1
#endif

static part_dev_t *mt_part_dev;
extern void part_init_pmt(unsigned long totalblks,part_dev_t *dev);
extern int mmc_do_erase(int dev_num,u64 start_addr,u64 len);

//static 
extern part_t partition_layout[];
#ifdef MTK_EMMC_SUPPORT
static uchar *mt_part_buf;
#else
__attribute__ ((aligned(64))) static uchar mt_part_buf[16384];
#endif

#ifdef MTK_EMMC_SUPPORT
void mt_part_dump(void)
{
    part_t *part = &partition_layout[0];
    
    dprintf(INFO,"\nPart Info from compiler.(1blk=%dB):\n", BLK_SIZE);
    dprintf(INFO,"\nPart Info.(1blk=%dB):\n", BLK_SIZE);
    while (part->name) {
    	 dprintf(INFO,"[0x%016llx-0x%016llx] (%.8ld blocks): \"%s\"\n", 
               (u64)part->startblk * BLK_SIZE, 
              (u64)(part->startblk + part->blknum) * BLK_SIZE - 1, 
			   				part->blknum, part->name);
        part++;
    }
    dprintf(INFO,"\n");
}
#else
void mt_part_dump(void)
{
    part_t *part = &partition_layout[0];
    
    dprintf(INFO,"\nPart Info from compiler.(1blk=%dB):\n", BLK_SIZE);
    dprintf(INFO,"\nPart Info.(1blk=%dB):\n", BLK_SIZE);
    while (part->name) {
        dprintf(INFO,"[0x%.8x-0x%.8x] (%.8ld blocks): \"%s\"\n", 
               part->startblk * BLK_SIZE, 
              (part->startblk + part->blknum) * BLK_SIZE - 1, 
			   part->blknum, part->name);
        part++;
    }
    dprintf(INFO,"\n");
}
#endif
void mt_part_init(unsigned long totalblks)
{
    part_t *part = &partition_layout[0];
    unsigned long lastblk;

 //   mt_part_buf = (uchar*)malloc(BLK_SIZE * 2);
 //   dprintf(INFO,"[mt_part_init]malloc %d : %x\n",(BLK_SIZE * 2),mt_part_buf);
    if (!totalblks) return;

    /* updater the number of blks of first part. */
    if (totalblks <= part->blknum)
        part->blknum = totalblks;

    totalblks -= part->blknum;
	if(part->type == TYPE_LOW)
       	lastblk = part->startblk + part->blknum*2;
	else
		lastblk = part->startblk + part->blknum;
    

    while(totalblks) {
        part++;
        if (!part->name)
            break;

        if (part->flags & PART_FLAG_LEFT || totalblks <= part->blknum)
            part->blknum = totalblks;

        part->startblk = lastblk;
        totalblks -= part->blknum;
        if(part->type == TYPE_LOW)
       		lastblk = part->startblk + part->blknum*2;
		else
			lastblk = part->startblk + part->blknum;
		printf ("[%s] part->startblk %x\n", __FUNCTION__, part->startblk);
    }
}

#ifdef PMT
part_t tempart;
pt_resident lastest_part[PART_MAX_COUNT];
#endif
part_t* mt_part_get_partition(char *name)
{
	int index=0;
	dprintf (INFO,"[%s] %s\n", __FUNCTION__, name);
    part_t *part = &partition_layout[0];
	if(!strcmp(name, "para"))
		name = PART_MISC;
	printf("[pmt]%s\n", name);
	while (part->name)
	{
	    printf("[pmt]part name: %s\n", (char *)part->name);
		if (!strnicmp (name, (char *)part->name, strlen(name)))
		{
#ifdef PMT
		tempart.name=part->name;
		//when download get partitin used new,orther wise used latest
		{
			tempart.startblk = BLK_NUM(lastest_part[index].offset);
			tempart.blknum=BLK_NUM(lastest_part[index].size);
		}
		tempart.flags=part->flags;
		printf ("[%s] %lx\n", __FUNCTION__, tempart.startblk);
		return &tempart;
#endif
		return part;
		}
		index++;
		part++;
	}
    return NULL;
}
#ifdef MTK_EMMC_SUPPORT
int mt_part_generic_read(part_dev_t *dev, u64 src, uchar *dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
		u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        ERRMSG("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) 
		return 0;

	end = src + size;
	
	part_start    = src &  ((u64)BLK_SIZE - 1);
	part_end      = end &  ((u64)BLK_SIZE - 1);
	aligned_start = src & ~((u64)BLK_SIZE - 1);
	aligned_end   = end & ~((u64)BLK_SIZE - 1);
 
	if (part_start) {
	    blknr = aligned_start >> BLK_BITS;	
			part_len = BLK_SIZE - part_start;
		    if(part_len-size>0) //To fix build warning, so use reduction instead of comparison between u64 and int
			{
				part_len = size;
			}
			if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1)
				return -EIO;
			memcpy(dst, buf + part_start, part_len);
			dst += part_len;
      src += part_len;
    }
    
	aligned_start = src & ~((u64)BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;
	
	if(blkcnt!=0)
	{	
		if ((blkdev->block_read(dev_id, blknr, blkcnt, (unsigned long *)(dst),0)) != blkcnt)
			return -EIO;
	}
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

	if (part_end && src < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1)
			return -EIO;
		memcpy(dst, buf, part_end);
	}
	return size;
}

static int mt_part_generic_write(part_dev_t *dev, uchar *src, u64 dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
		u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        ERRMSG("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) 
		return 0;

	end = dst + size;
	
	part_start    = dst &  ((u64)BLK_SIZE - 1);
	part_end      = end &  ((u64)BLK_SIZE - 1);
	aligned_start = dst & ~((u64)BLK_SIZE - 1);
	aligned_end   = end & ~((u64)BLK_SIZE - 1);
 
	if (part_start) {
	    blknr = aligned_start >> BLK_BITS;	
		part_len = BLK_SIZE - part_start;
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1)
			return -EIO;
		memcpy(buf + part_start, src, part_len);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1)
        	return -EIO;
		dst += part_len;
        src += part_len;
    }
    
	aligned_start = dst & ~((u64)BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;

	if ((blkdev->block_write(dev_id, blknr, blkcnt, (unsigned long *)(src),0)) != blkcnt)
		return -EIO;
    
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

	if (part_end && dst < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1) {
			return -EIO;
		}
		memcpy(buf, src, part_end);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1) {
            return -EIO;
    	}
	}
	return size;
}

int mt_part_register_device(part_dev_t *dev)
{
	dprintf(INFO,"[mt_part_register_device]\n");
    	if (!mt_part_dev) {
		if (!dev->read)
		    dev->read = mt_part_generic_read;
		if (!dev->write)
		    dev->write = mt_part_generic_write;
		mt_part_dev = dev;

	mt_part_buf = (uchar*)malloc(BLK_SIZE * 2);
	dprintf(INFO,"[mt_part_register_device]malloc %d : %x\n",(BLK_SIZE * 2),mt_part_buf);

	#ifdef PMT
		part_init_pmt((unsigned long)dev->blkdev->lba,dev);
	#else
		mt_part_init((unsigned long)dev->blkdev->lba);
	#endif

    }
    return 0;
}
#else
int mt_part_generic_read(part_dev_t *dev, ulong src, uchar *dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        ERRMSG("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) 
		return 0;

	end = src + size;
	
	part_start    = src &  (BLK_SIZE - 1);
	part_end      = end &  (BLK_SIZE - 1);
	aligned_start = src & ~(BLK_SIZE - 1);
	aligned_end   = end & ~(BLK_SIZE - 1);
 
	if (part_start) {
	    blknr = aligned_start >> BLK_BITS;	
		part_len = BLK_SIZE - part_start;
		if(part_len>(ulong)size)
		{
			part_len = size;
		}
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1)
			return -EIO;
		memcpy(dst, buf + part_start, part_len);
		dst += part_len;
        src += part_len;
    }
    
	aligned_start = src & ~(BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;
	
	if(blkcnt!=0)
	{
		if ((blkdev->block_read(dev_id, blknr, blkcnt, (unsigned long *)(dst),0)) != blkcnt)
			return -EIO;
	}

    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

	if (part_end && src < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1)
			return -EIO;
		memcpy(dst, buf, part_end);
	}
	return size;
}

static int mt_part_generic_write(part_dev_t *dev, uchar *src, ulong dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        ERRMSG("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) 
		return 0;

	end = dst + size;
	
	part_start    = dst &  (BLK_SIZE - 1);
	part_end      = end &  (BLK_SIZE - 1);
	aligned_start = dst & ~(BLK_SIZE - 1);
	aligned_end   = end & ~(BLK_SIZE - 1);
 
	if (part_start) {
	    blknr = aligned_start >> BLK_BITS;	
		part_len = BLK_SIZE - part_start;
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1)
			return -EIO;
		memcpy(buf + part_start, src, part_len);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1)
        	return -EIO;
		dst += part_len;
        src += part_len;
    }
    
	aligned_start = dst & ~(BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;

	if ((blkdev->block_write(dev_id, blknr, blkcnt, (unsigned long *)(src),0)) != blkcnt)
		return -EIO;
    
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

	if (part_end && dst < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1) {
			return -EIO;
		}
		memcpy(buf, src, part_end);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf,0)) != 1) {
            return -EIO;
    	}
	}
	return size;
}

int mt_part_register_device(part_dev_t *dev)
{
    dprintf(INFO,"[mt_part_register_device]\n");
    if (!mt_part_dev) {
        if (!dev->read)
            dev->read = mt_part_generic_read;
        if (!dev->write)
            dev->write = mt_part_generic_write;
        mt_part_dev = dev;
		//mt_part_buf = (uchar*)malloc(8192);
    	dprintf(INFO,"[mt_part_register_device]malloc %d : %x\n",(8192),mt_part_buf);
        
#ifdef PMT
		 
	    part_init_pmt(BLK_NUM(1 * GB),dev);
#else
            mt_part_init(BLK_NUM(1 * GB));
#endif 
        }
	
    return 0;
}
#endif
part_dev_t *mt_part_get_device(void)
{
    if (mt_part_dev && !mt_part_dev->init && mt_part_dev->init_dev) 
    {
        mt_part_dev->init_dev(mt_part_dev->id);
        mt_part_dev->init = 1;
    }
    return mt_part_dev;
}

/**/
/*fastboot*/
/**/
unsigned int write_partition(unsigned size, unsigned char *partition)
{
    return 0;
}
int partition_get_index(const char * name)
{
	int index;

	for(index = 0;index<PART_MAX_COUNT;index++){
		if(!strcmp(name,g_part_name_map[index].fb_name)){
			dprintf(INFO,"[%s]find %s %s index %d\n",__FUNCTION__,name,g_part_name_map[index].r_name,g_part_name_map[index].partition_idx);
			return g_part_name_map[index].partition_idx;
		}
	}
	if(index == PART_MAX_COUNT)
		return -1;
	return -1;
}

u64 partition_get_offset(int index)
{
	part_t *p = mt_part_get_partition(g_part_name_map[index].r_name);
	if(p == NULL)
		return -1;
	return (u64)p->startblk*BLK_SIZE;
}

u64 partition_get_size(int index)
{
	part_t *p = mt_part_get_partition(g_part_name_map[index].r_name);
	if(p == NULL)
		return -1;
	return (u64)p->blknum*BLK_SIZE;
}
int partition_get_type(int index, char **p_type)
{
	*p_type = g_part_name_map[index].partition_type;
	return 0;

}
int partition_get_name(int index, char **p_name)
{
	*p_name = g_part_name_map[index].fb_name;
	return 0;
}
int is_support_erase(int index)
{
	return g_part_name_map[index].is_support_erase;
}
int is_support_flash(int index)
{
	return g_part_name_map[index].is_support_dl;
}

unsigned long partition_reserve_size(void)
{
	unsigned long size = 0;	
	if(target_is_emmc_boot()){
#ifdef MTK_EMMC_SUPPORT_OTP
		size += PART_SIZE_OTP;
#endif
		size += PART_SIZE_BMTPOOL*(128*1024);
	}

	return size;		
}

