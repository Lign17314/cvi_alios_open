
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <aos/aos.h>
#include "aos/cli.h"
#include <aos/kernel.h>
#include "vfs.h"
#if 0
#include "core/utils/vpss_helper.h"
#include "cvi_ive.h"
#include "cvi_ive_interface.h"
#include "cvi_tdl.h"
#include "cvi_tdl_media.h"
#include "cvi_tpu_interface.h"
// #include "sample_comm.h"
// #include "sample_utils.h"

/* SAMPLE_COMM_FRAME_LoadFromFile:
 *   Load data to frame, whose data loaded from given filename.
 *
 * [in]filename: file to read.
 * [in]pstVideoFrame: the video-frame to store data from file.
 * [in]stSize: size of image.
 * [in]enPixelFormat: format of image
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_FRAME_LoadFromFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame,
                                       SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat)
{
    VB_BLK blk;
    int fp = 0;
    CVI_U32 u32len;
    VB_CAL_CONFIG_S stVbCalConfig;

    COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

    memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
    pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
    pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
    pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
    pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
    pstVideoFrame->stVFrame.u32Width = stSize->u32Width;
    pstVideoFrame->stVFrame.u32Height = stSize->u32Height;
    pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
    pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
    pstVideoFrame->stVFrame.u32TimeRef = 0;
    pstVideoFrame->stVFrame.u64PTS = 0;
    pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

    blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
    if (blk == VB_INVALID_HANDLE)
    {
        printf("Can't acquire vb block\n");
        return CVI_FAILURE;
    }

    // open data file & fread into the mmap address
    // fp = fopen(filename, "r");
    fp = aos_open(filename, O_RDONLY);
    if (fp < 0)
    {
        printf("open data file error\n");
        return CVI_FAILURE;
    }

    pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
    pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
    pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
    pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
    pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0] + ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
    if (stVbCalConfig.plane_num == 3)
    {
        pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
        pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
        pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1] + ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
    }

    printf("length of buffer(%d, %d, %d)\n", pstVideoFrame->stVFrame.u32Length[0], pstVideoFrame->stVFrame.u32Length[1], pstVideoFrame->stVFrame.u32Length[2]);
    printf("phy addr(%llx, %llx, %llx)\n", pstVideoFrame->stVFrame.u64PhyAddr[0], pstVideoFrame->stVFrame.u64PhyAddr[1], pstVideoFrame->stVFrame.u64PhyAddr[2]);

    for (int i = 0; i < stVbCalConfig.plane_num; ++i)
    {
        if (pstVideoFrame->stVFrame.u32Length[i] == 0)
            continue;
        // pstVideoFrame->stVFrame.pu8VirAddr[i]
        //	= CVI_SYS_MmapCache(pstVideoFrame->stVFrame.u64PhyAddr[i],
        //			    pstVideoFrame->stVFrame.u32Length[i]);
        // if (pstVideoFrame->stVFrame.pu8VirAddr[i] == CVI_NULL) {
        //	printf("mmap plane%d error\n", i);
        //	return CVI_FAILURE;
        // }

        // u32len = fread((void *)pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i], 1, fp);
        u32len = aos_read(fp, (void *)pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
        if (u32len <= 0)
        {
            printf("fread plane%d error %d %d\n", i, u32len, pstVideoFrame->stVFrame.u32Length[i]);
            return CVI_FAILURE;
        }
        printf("fread plane%d %d %d\n", i, u32len, pstVideoFrame->stVFrame.u32Length[i]);
        // CVI_SYS_IonInvalidateCache(pstVideoFrame->stVFrame.u64PhyAddr[i],
        //			   pstVideoFrame->stVFrame.pu8VirAddr[i],
        //			   pstVideoFrame->stVFrame.u32Length[i]);
        // CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
    }

    aos_close(fp);

    printf("read file done and send out frame.\n");

    return CVI_SUCCESS;
}

#define WIDTH 1920
#define HEIGHT 1080

int process_image_file(cvitdl_handle_t tdl_handle, const char *imgf, cvtdl_object_t *p_obj)
{
    VIDEO_FRAME_INFO_S stVideoFrame;
    memset(&stVideoFrame, 0, sizeof(stVideoFrame));
    CVI_S32 s32Ret = CVI_SUCCESS;

    SIZE_S stSize = {
        .u32Width = WIDTH,
        .u32Height = HEIGHT,
    };

    s32Ret =
        SAMPLE_COMM_FRAME_LoadFromFile(imgf, &stVideoFrame, &stSize, PIXEL_FORMAT_RGB_888);
    if (s32Ret != CVI_SUCCESS)
    {
        printf("cvi_tdl read image :%s failed.\n", imgf);
        return CVI_TDL_FAILURE;
    }

    s32Ret = CVI_TDL_Detection(tdl_handle, &stVideoFrame, CVI_TDL_SUPPORTED_MODEL_YOLOV5, p_obj);
    if (s32Ret != CVI_SUCCESS)
    {
        printf("CVI_TDL_ScrFDFace failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    VB_BLK blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);
    s32Ret = CVI_VB_ReleaseBlock(blk);
    if (s32Ret != CVI_SUCCESS)
    {
        printf("video frame release failed! \n");
        return CVI_FAILURE;
    }
    return s32Ret;
}

void *yolov5_main(void *arg)
{
    // char **argv = (char **)arg;
    cvi_tpu_init();
    aos_msleep(500);
    CVI_S32 ret = 0;
    // int vpssgrp_width = WIDTH;
    // int vpssgrp_height = HEIGHT;
    // CVI_S32 ret = MMF_INIT_HELPER2(vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888_PLANAR, 3,
    //                                vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888_PLANAR, 1);
    // if (ret != CVI_SUCCESS)
    // {
    //     printf("Init sys failed with %#x!\n", ret);
    //     pthread_exit(NULL);
    // }
    cvitdl_handle_t tdl_handle = NULL;

    ret = CVI_TDL_CreateHandle(&tdl_handle);
    if (ret != CVI_SUCCESS)
    {
        printf("Create tdl handle failed with %#x!\n", ret);
        pthread_exit(NULL);
    }
    
    const char *model = "/mnt/sd/app.cvimodel"; // model path
    const char *img = "/mnt/sd/a.rgb";   // image path;
    printf("%s,%d %s %s\r\n", __func__, __LINE__, model , img);
    ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_YOLOV5, model);
    if (ret != CVI_SUCCESS)
    {
        printf("open model failed with %#x!\n", ret);
        pthread_exit(NULL);
    }
    // set thershold
    CVI_TDL_SetModelThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_YOLOV5, 0.5);
    CVI_TDL_SetModelNmsThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_YOLOV5, 0.5);

    cvtdl_object_t stObjMeta = {0};
    memset(&stObjMeta, 0, sizeof(cvtdl_face_t));
    process_image_file(tdl_handle, img, &stObjMeta);
    printf("%s,%d %d\r\n", __func__, __LINE__, stObjMeta.size);
    for (uint32_t i = 0; i < stObjMeta.size; i++)
    {
        printf("bbox: [%f, %f, %f, %f]; score: [%f]; class: [%d]\n", stObjMeta.info[i].bbox.x1, stObjMeta.info[i].bbox.y1,
               stObjMeta.info[i].bbox.x2, stObjMeta.info[i].bbox.y2, stObjMeta.info[i].bbox.score,
               stObjMeta.info[i].classes);
    }

    CVI_TDL_Free(&stObjMeta);
    CVI_TDL_DestroyHandle(tdl_handle);
    cvi_tpu_deinit();
    pthread_exit(NULL);
    return NULL;
}
void app(int argc, char *argv[])
{
    pthread_attr_t attr;
    pthread_t thread;
    int ret;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 64 * 1024);
    ret = pthread_create(&thread, &attr, yolov5_main, (void *)argv);
    if (ret != CVI_SUCCESS)
    {
        printf("Error create od thread! \n");
    }
    pthread_join(thread, NULL);
    printf("od thread finished! \n");
}

ALIOS_CLI_CMD_REGISTER(app, app, cvi_tdl sample yolov10);

static inline CVI_S32 SAMPLE_COMM_FRAME_SaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	int fd;
	CVI_U32 u32len, u32DataLen;

	fd = aos_open(filename, O_WRONLY | O_CREAT | O_SYNC);
	if (fd < 0) {
		printf("open file fail\n");
		return CVI_FAILURE;
	}

	for (int i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVideoFrame->stVFrame.pu8VirAddr[i] = (CVI_U8 *)pstVideoFrame->stVFrame.u64PhyAddr[i];
			//= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
        printf("enPixelFormat: %d u32Height: %d u32Width:%d\r\n",pstVideoFrame->stVFrame.enPixelFormat,
            pstVideoFrame->stVFrame.u32Height,
            pstVideoFrame->stVFrame.u32Width);
		printf("plane(%d): paddr(%#llx) vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		printf(" data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = aos_write(fd, (void *)pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen);
		if (u32len != u32DataLen) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "fwrite data(%d) error\n", i);
			return CVI_FAILURE;
		}
		//CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}
	aos_sync(fd);
	aos_close(fd);
	return CVI_SUCCESS;
}

void app_frame(int argc, char *argv[])
{
    VIDEO_FRAME_INFO_S stVideoFrame;

    int s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, -1);
    if (s32Ret != CVI_SUCCESS) {
        printf("vpss get chn frame failed, exit!\n");
        return;
    } else {
        #define VPSS_FRAME_FILE "/mnt/sd/frame.rgb888planar"
        SAMPLE_COMM_FRAME_SaveToFile(VPSS_FRAME_FILE, &stVideoFrame);
        printf("Video frame save success!\n");
        CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
    }
}

ALIOS_CLI_CMD_REGISTER(app_frame, app_frame, cvi_tdl app_frame yolov10);
#endif
//  app /mnt/sd/app.cvimodel /mnt/sd/a.rgb

#include "wifi_if.h"
static void app_wifi(int32_t argc,char **argv)
{
    //Note 链接WIFI Note
    CVI_S32 s32Ret;

    // s32Ret = WifiConnect((CVI_U8 *)"Mi", strlen("Mi"), (CVI_U8 *)"12345678", strlen("12345678"));
    s32Ret = WifiConnect((CVI_U8 *)"Xiaomi_8B2B", strlen("Xiaomi_8B2B"), (CVI_U8 *)"12345678", strlen("12345678"));
    // s32Ret = WifiConnect((CVI_U8 *)"TP-LINK_C3EA", strlen("TP-LINK_C3EA"), (CVI_U8 *)"1234567890", strlen("1234567890"));
    // s32Ret = WifiConnect((CVI_U8 *)"Tenda_441A40", strlen("Tenda_441A40"), (CVI_U8 *)"12345678", strlen("12345678"));
    // s32Ret = WifiConnect((CVI_U8 *)"MERCURY_AD33", strlen("MERCURY_AD33"), (CVI_U8 *)"lj001022", strlen("lj001022"));
    if(s32Ret != CVI_SUCCESS) {
        printf("Wifi Connect [%s] fail \r\n","SSID");
        return ;
    }
    printf("Wifi Connect [%s] success \r\n","MERCURY_AD33");
}
ALIOS_CLI_CMD_REGISTER(app_wifi, app_wifi, TESTWIFI_connect);