#ifndef __NWY_CAMERA_H__
#define __NWY_CAMERA_H__
#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned char        uint8_t;
typedef unsigned short int   uint16_t;
typedef signed char          int8_t;
//typedef unsigned int         uint32_t;

enum CAMERA_IMAGE_FORMAT
{
	YUV_FORMAT,
	JPG_FORMAT,
	UNKOWN_FORMAT
};

enum CAMERA_IMAGE_STORAGE_MODE
{
	FS_MODE,
#ifdef CONFIG_FS_MOUNT_SDCARD
	SD_MODE,
#endif
	UNKOWN_MODE
};

enum CAMERA_CAPTURE_RESULT
{
	CAPTURE_SET_FAIL = -1, //set capture mode fail
	CAPTURE_STORAGE_OK,    //take picture ok
	IMAGE_NULL,            //no take photo
	IMAGE_FORMAT_UNKOWN,   //image format invaild
	IMAGE_SAVE_UNKOWN,     //image storage medium invaild
	IMAGE_PATH_INVAILD,    //image path invaild
	FS_SPACE_NOENOUGH,     //internal flash no enough
	SD_NODETECTED,         //sdcard no detected
	SD_SPACE_NOENOUGH,     //sd flash no enough,no support
	IMAGE_CROP_ERROR,      //image crop error
	OTHER_ERROR
};

typedef struct
{
	uint32_t start_x;     //crop img start x_location
	uint32_t start_y;     //crop img start y_location
	uint32_t des_width;   //crop img actual width
	uint32_t des_height;  //crop img actual height
} crop_image_info_t;
int nwy_camera_open(void);
void nwy_camera_close(void);
int nwy_camera_start_preview(void);
void nwy_camera_get_preview(uint16_t **imagebuff);
void nwy_camera_close_preview(void);
int nwy_camera_capture_image(uint32_t image_format, uint32_t image_save, char *image_path);
int nwy_camera_crop_capture_image(uint32_t image_format, uint32_t image_save, char *image_path, crop_image_info_t *crop_image_info);
#ifdef __cplusplus
   }
#endif

#endif /* __NWY_CAMERA_H__ */
