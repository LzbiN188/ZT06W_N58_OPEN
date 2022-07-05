#ifndef __NWY_FILE_H__
#define __NWY_FILE_H__

#include "nwy_common.h"
#include <sys/stat.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum{
    NWY_RDONLY   = 0x0,
    NWY_WRONLY   = 0x1,
    NWY_RDWR     = 0x2,
    NWY_APPEND   = 0x8,
    NWY_CREAT    = 0x0200,
    NWY_TRUNC    = 0x0400,
}nwy_file_action_e;

typedef enum{
    NWY_SEEK_SET   = 0,
    NWY_SEEK_CUR   = 1,
    NWY_SEEK_END   = 2
}nwy_fseek_offset_e;

typedef struct
{
    uint16 fs_index;  ///< internal fs index
    uint16 _reserved; ///< reserved
} nwy_dir;

#define NWY_DT_UNKOWN 0 ///< unknown type
#define NWY_DT_DIR 4    ///< directory
#define NWY_DT_REG 8    ///< regular file
typedef struct
{
    int d_ino;            ///< inode number, file system implementation can use it for any purpose
    uint8 d_type; ///< type of file
    char d_name[256];     ///< file name
}nwy_dirent;


int nwy_sdk_fopen(const char *path, nwy_file_action_e flags);
int nwy_sdk_fclose(int fd);
long nwy_sdk_fread(int fd, void *dst, long size);
long nwy_sdk_fwrite(int fd, const void *data, long size);
int nwy_sdk_fseek(int fd, int offset, nwy_fseek_offset_e mode);
long nwy_sdk_fsize(const char *path);
long nwy_sdk_fsize_fd(int fd);
bool nwy_sdk_fexist(const char *path);
int nwy_sdk_fsync(int fd);
int nwy_sdk_get_stat_path(const char *path, struct stat *st);
int nwy_sdk_get_stat_fd(int fd, struct stat *st);
int nwy_sdk_ftrunc_fd(int fd, long len);
int nwy_sdk_ftrunc_path(const char *path, long len);
int nwy_sdk_file_unlink(const char *path);
int nwy_sdk_frename(const char *oldpath, const char *newpath);
nwy_dir *nwy_sdk_vfs_opendir(const char *name);
nwy_dirent *nwy_sdk_vfs_readdir(nwy_dir *pdir);
long nwy_sdk_vfs_telldir(nwy_dir *pdir);
void nwy_sdk_vfs_seekdir(nwy_dir *pdir, long loc);
void nwy_sdk_vfs_rewinddir(nwy_dir *pdir);
int nwy_sdk_vfs_closedir(nwy_dir *pdir);
int nwy_sdk_vfs_mkdir(const char *name);
int nwy_sdk_vfs_rmdir(const char *name);
int nwy_sdk_vfs_rmdir_recursive(const char *name);
unsigned long nwy_sdk_vfs_ls(void);
unsigned long nwy_sdk_vfs_free_size(const char *path);
int nwy_sdk_sfile_init(const char *path);
long nwy_sdk_sfile_read(const char *path, void *dst, long size);
long nwy_sdk_sfile_write(const char *path, void *data, long size);
long nwy_sdk_sfile_size(const char *path);
long nwy_sdk_fread_path(const char *path, void *dst, long size);
long nwy_sdk_fwrite_path(const char *path, void *data, long size);
bool nwy_sdk_sdcard_mount();
void nwy_sdk_sdcard_unmount();
int nwy_read_sdcart_status();
void nwy_format_sdcard(void);

#if defined(FEATURE_NWY_SD_CONFIG)
// freq <= 25000000
void nwy_sdk_set_sd_clk_freq(unsigned int freq);
// freq <= 50000000
void nwy_sdk_set_sdhc_clk_freq(unsigned int freq);
unsigned int nwy_sdk_get_sd_clk_freq(void);
unsigned int nwy_sdk_get_sdhc_clk_freq(void);
#endif

//read&write flash
#define NAME_FLASH_1  "SFL1"
#define SPI_FLASH_OFFSET(address)    ((uint32_t)(address)&0x00ffffff)
typedef struct nwySpiFlash nwyspiflash_t;
nwyspiflash_t *nwy_flash_open(char *flash_name);
bool nwy_flash_erase(nwyspiflash_t *d, uint32_t flash_addr, size_t size);
bool nwy_flash_write(nwyspiflash_t *d, uint32_t flash_addr, const void *data, size_t size);
bool nwy_flash_read(nwyspiflash_t *d, uint32_t flash_addr, void *data, size_t size);
bool nwy_flash_read_check(nwyspiflash_t *d, uint32_t flash_addr, const void *data, size_t size);

#ifdef __cplusplus
   }
#endif

#endif
