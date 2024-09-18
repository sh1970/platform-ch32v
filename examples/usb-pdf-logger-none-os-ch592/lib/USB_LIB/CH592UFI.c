/* 2014.09.09
*****************************************
**   Copyright  (C)  W.ch  1999-2019   **
**   Web:      http://wch.cn           **
*****************************************
**  USB-flash File Interface for CH592 **
**  KEIL423, gcc 8.20          **
*****************************************
*/
/* CHRV3 U�������ļ�ϵͳ�ӿ�, ֧��: FAT12/FAT16/FAT32 */

//#define DISK_BASE_BUF_LEN		512	/* Ĭ�ϵĴ������ݻ�������СΪ512�ֽ�(����ѡ��Ϊ2048����4096��֧��ĳЩ��������U��),Ϊ0���ֹ�ڱ��ļ��ж��建��������Ӧ�ó�����pDISK_BASE_BUF��ָ�� */
/* �����Ҫ���ô������ݻ������Խ�ԼRAM,��ô�ɽ�DISK_BASE_BUF_LEN����Ϊ0�Խ�ֹ�ڱ��ļ��ж��建����,����Ӧ�ó����ڵ���CHRV3LibInit֮ǰ��������������õĻ�������ʼ��ַ����pDISK_BASE_BUF���� */

#define NO_DEFAULT_ACCESS_SECTOR	1		/* ��ֹĬ�ϵĴ���������д�ӳ���,���������б�д�ĳ�������� */
//#define NO_DEFAULT_DISK_CONNECT		1		/* ��ֹĬ�ϵļ����������ӳ���,���������б�д�ĳ�������� */
//#define NO_DEFAULT_FILE_ENUMER		1		/* ��ֹĬ�ϵ��ļ���ö�ٻص�����,���������б�д�ĳ�������� */

#include "CH59x_common.h"
#include "CHRV3UFI.h"
#include "internal_flash.h"
#include "sw_udisk.h"
#include "pdfFile.h"
#include "spi_flash.h"
#include "peripheral.h"

UINT8 CtrlGetConfigDescrTB(void) // ��ȡ����������,������TxBuffer��
{
    return (CtrlGetConfigDescr());
}

CMD_PARAM_I mCmdParam; /* ������� */
#if DISK_BASE_BUF_LEN > 0
//UINT8	DISK_BASE_BUF[ DISK_BASE_BUF_LEN ] __attribute__((at(BA_RAM+SZ_RAM/2)));	/* �ⲿRAM�Ĵ������ݻ�����,����������Ϊһ�������ĳ��� */
//UINT8 DISK_BASE_BUF[DISK_BASE_BUF_LEN] __attribute__((aligned(4))); /* �ⲿRAM�Ĵ������ݻ�����,����������Ϊһ�������ĳ��� */
//UINT8	DISK_FAT_BUF[ DISK_BASE_BUF_LEN ] __attribute__((aligned (4)));	/* �ⲿRAM�Ĵ���FAT���ݻ�����,����������Ϊһ�������ĳ��� */
#endif

/* ���³�����Ը�����Ҫ�޸� */

#ifndef NO_DEFAULT_ACCESS_SECTOR /* ��Ӧ�ó����ж���NO_DEFAULT_ACCESS_SECTOR���Խ�ֹĬ�ϵĴ���������д�ӳ���,Ȼ�������б�д�ĳ�������� */
//if ( use_external_interface ) {  // �滻U�������ײ��д�ӳ���
//    CHRV3vSectorSize=512;  // ����ʵ�ʵ�������С,������512�ı���,��ֵ�Ǵ��̵�������С
//    CHRV3vSectorSizeB=9;   // ����ʵ�ʵ�������С��λ����,512���Ӧ9,1024��Ӧ10,2048��Ӧ11
//    CHRV3DiskStatus=DISK_MOUNTED;  // ǿ�ƿ��豸���ӳɹ�(ֻ������ļ�ϵͳ)
//}

UINT8 CHRV3ReadSector(UINT8 SectCount, PUINT8 DataBuf) /* �Ӵ��̶�ȡ������������ݵ��������� */
{
    UINT8 retry;
    //	if ( use_external_interface ) return( extReadSector( CHRV3vLbaCurrent, SectCount, DataBuf ) );  /* �ⲿ�ӿ� */
    for(retry = 0; retry < 3; retry++)
    {                                                                /* �������� */
        pCBW->mCBW_DataLen = (UINT32)SectCount << CHRV3vSectorSizeB; /* ���ݴ��䳤�� */
        pCBW->mCBW_Flag = 0x80;
        pCBW->mCBW_LUN = CHRV3vCurrentLun;
        pCBW->mCBW_CB_Len = 10;
        pCBW->mCBW_CB_Buf[0] = SPC_CMD_READ10;
        pCBW->mCBW_CB_Buf[1] = 0x00;
        pCBW->mCBW_CB_Buf[2] = (UINT8)(CHRV3vLbaCurrent >> 24);
        pCBW->mCBW_CB_Buf[3] = (UINT8)(CHRV3vLbaCurrent >> 16);
        pCBW->mCBW_CB_Buf[4] = (UINT8)(CHRV3vLbaCurrent >> 8);
        pCBW->mCBW_CB_Buf[5] = (UINT8)(CHRV3vLbaCurrent);
        pCBW->mCBW_CB_Buf[6] = 0x00;
        pCBW->mCBW_CB_Buf[7] = 0x00;
        pCBW->mCBW_CB_Buf[8] = SectCount;
        pCBW->mCBW_CB_Buf[9] = 0x00;
        CHRV3BulkOnlyCmd(DataBuf); /* ִ�л���BulkOnlyЭ������� */
        if(CHRV3IntStatus == ERR_SUCCESS)
        {
            return (ERR_SUCCESS);
        }
        CHRV3IntStatus = CHRV3AnalyzeError(retry);
        if(CHRV3IntStatus != ERR_SUCCESS)
        {
            return (CHRV3IntStatus);
        }
    }
    return (CHRV3IntStatus = ERR_USB_DISK_ERR); /* ���̲������� */
}

  #ifdef EN_DISK_WRITE
UINT8 CHRV3WriteSector(UINT8 SectCount, PUINT8 DataBuf) /* ���������еĶ�����������ݿ�д����� */
{
    UINT8 retry;
    //	if ( use_external_interface ) return( extWriteSector( CHRV3vLbaCurrent, SectCount, DataBuf ) );  /* �ⲿ�ӿ� */
    for(retry = 0; retry < 3; retry++)
    {                                                                /* �������� */
        pCBW->mCBW_DataLen = (UINT32)SectCount << CHRV3vSectorSizeB; /* ���ݴ��䳤�� */
        pCBW->mCBW_Flag = 0x00;
        pCBW->mCBW_LUN = CHRV3vCurrentLun;
        pCBW->mCBW_CB_Len = 10;
        pCBW->mCBW_CB_Buf[0] = SPC_CMD_WRITE10;
        pCBW->mCBW_CB_Buf[1] = 0x00;
        pCBW->mCBW_CB_Buf[2] = (UINT8)(CHRV3vLbaCurrent >> 24);
        pCBW->mCBW_CB_Buf[3] = (UINT8)(CHRV3vLbaCurrent >> 16);
        pCBW->mCBW_CB_Buf[4] = (UINT8)(CHRV3vLbaCurrent >> 8);
        pCBW->mCBW_CB_Buf[5] = (UINT8)(CHRV3vLbaCurrent);
        pCBW->mCBW_CB_Buf[6] = 0x00;
        pCBW->mCBW_CB_Buf[7] = 0x00;
        pCBW->mCBW_CB_Buf[8] = SectCount;
        pCBW->mCBW_CB_Buf[9] = 0x00;
        CHRV3BulkOnlyCmd(DataBuf); /* ִ�л���BulkOnlyЭ������� */
        if(CHRV3IntStatus == ERR_SUCCESS)
        {
            mDelayuS(200); /* д��������ʱ */
            return (ERR_SUCCESS);
        }
        CHRV3IntStatus = CHRV3AnalyzeError(retry);
        if(CHRV3IntStatus != ERR_SUCCESS)
        {
            return (CHRV3IntStatus);
        }
    }
    return (CHRV3IntStatus = ERR_USB_DISK_ERR); /* ���̲������� */
}
  #endif
#else
UINT8 CHRV3ReadSector(UINT8 SectCount, PUINT8 DataBuf) /* Read multiple sectors of data from disk into a buffer */
{
    int len;
    uint32_t addr;

    len =(uint32_t)SectCount << CHRV3vSectorSizeB;
    addr = CHRV3vLbaCurrent * DEF_UDISK_SECTOR_SIZE;

    FLASH_RD_Block_Start( addr );
    FLASH_RD_Block( DataBuf, len );
    FLASH_RD_Block_End();
//    pdf_memcpy(DataBuf, (uint8_t *)addr, len);
    CHRV3IntStatus = ERR_SUCCESS;

    return ERR_SUCCESS;
}

UINT8 CHRV3WriteSector(UINT8 SectCount, PUINT8 DataBuf) /* Write data blocks of multiple sectors in the buffer to disk */
{
    int len;
    uint32_t addr;

    len =(uint32_t)SectCount << CHRV3vSectorSizeB;
    addr = CHRV3vLbaCurrent * DEF_UDISK_SECTOR_SIZE;

    pdf_memcpy(RAM_BUFFER.PDF_BUFFER.UDisk_Down_Buffer, DataBuf, len);
    W25XXX_WR_512B(RAM_BUFFER.PDF_BUFFER.UDisk_Down_Buffer, addr, 512);
//    IFlash_Prog_512( addr,(uint32_t*)UDisk_Down_Buffer);
    CHRV3IntStatus = ERR_SUCCESS;

    return ERR_SUCCESS;
}
#endif // NO_DEFAULT_ACCESS_SECTOR

#ifndef NO_DEFAULT_DISK_CONNECT /* ��Ӧ�ó����ж���NO_DEFAULT_DISK_CONNECT���Խ�ֹĬ�ϵļ����������ӳ���,Ȼ�������б�д�ĳ�������� */

/*
Լ��: USB�豸��ַ�������(�ο�USB_DEVICE_ADDR)
��ֵַ  �豸λ��
0x02    ����Root-HUB0�µ�USB�豸���ⲿHUB
0x03    ����Root-HUB1�µ�USB�豸���ⲿHUB
0x1x    ����Root-HUB0�µ��ⲿHUB�Ķ˿�x�µ�USB�豸,xΪ1~n
0x2x    ����Root-HUB1�µ��ⲿHUB�Ķ˿�x�µ�USB�豸,xΪ1~n
*/

//#define		UHUB_DEV_ADDR	( CHRV3vRootPort ? R8_USB1_DEV_AD : R8_USB0_DEV_AD )
//#define		UHUB_MIS_STAT	( CHRV3vRootPort ? R8_USB1_MIS_ST : R8_USB0_MIS_ST )
//#define		UHUB_HOST_CTRL	( CHRV3vRootPort ? R8_UHOST1_CTRL : R8_UHOST0_CTRL )
//#define		UHUB_INT_FLAG	( CHRV3vRootPort ? R8_USB1_INT_FG : R8_USB0_INT_FG )
#define UHUB_DEV_ADDR     R8_USB_DEV_AD
#define UHUB_MIS_STAT     R8_USB_MIS_ST
#define UHUB_HOST_CTRL    R8_UHOST_CTRL
#define UHUB_INT_FLAG     R8_USB_INT_FG
#define bUMS_ATTACH       RB_UMS_DEV_ATTACH
#define bUMS_SUSPEND      RB_UMS_SUSPEND

/* �������Ƿ����� */
UINT8 CHRV3DiskConnect(void)
{
    UINT8 ums, devaddr;
    UHUB_DEV_ADDR = UHUB_DEV_ADDR & 0x7F;
    ums = UHUB_MIS_STAT;
    devaddr = UHUB_DEV_ADDR;
    if(devaddr == USB_DEVICE_ADDR)
    {   /* ����Root-HUB�µ�USB�豸 */
        //		if ( UHUB_HOST_CTRL & RB_UH_PORT_EN ) {  /* ����Root-HUB�µ�USB�豸������δ��� */
        if(ums & bUMS_ATTACH)
        {   /* ����Root-HUB�µ�USB�豸���� */
            //			if ( ( UHUB_INT_FLAG & UIF_DETECT ) == 0 ) {  /* ����Root-HUB�µ�USB�豸������δ��� */
            if((ums & bUMS_SUSPEND) == 0)
            {                         /* ����Root-HUB�µ�USB�豸������δ��� */
                return (ERR_SUCCESS); /* USB�豸�Ѿ�������δ��� */
            }
            else
            { /* ����Root-HUB�µ�USB�豸���� */
            mDiskConnect:
                CHRV3DiskStatus = DISK_CONNECT; /* �����Ͽ��� */
                return (ERR_SUCCESS);           /* �ⲿHUB��USB�豸�Ѿ����ӻ��߶Ͽ����������� */
            }
        }
        else
        { /* USB�豸�Ͽ� */
        mDiskDisconn:
            CHRV3DiskStatus = DISK_DISCONNECT;
            return (ERR_USB_DISCON);
        }
    }
  #ifndef FOR_ROOT_UDISK_ONLY
    else if(devaddr > 0x10 && devaddr <= 0x14)
    {   /* �ⲿHUB�Ķ˿��µ�USB�豸 */
        //		if ( UHUB_HOST_CTRL & RB_UH_PORT_EN ) {  /* ����Root-HUB�µ��ⲿHUB������δ��� */
        if(ums & bUMS_ATTACH)
        {   /* ����Root-HUB�µ�USB�豸���� */
            //			if ( ( UHUB_INT_FLAG & UIF_DETECT ) == 0 ) {  /* ����Root-HUB�µ�USB�豸������δ��� */
            if((ums & bUMS_SUSPEND) == 0)
            {                                                                            /* ����Root-HUB�µ�USB�豸������δ��� */
                TxBuffer[MAX_PACKET_SIZE - 1] = devaddr;                                 /* ���� */
                UHUB_DEV_ADDR = USB_DEVICE_ADDR - 1 + (UHUB_DEV_ADDR >> 4);              /* ����USB�����˵�USB��ַָ��HUB */
                CHRV3IntStatus = HubGetPortStatus(TxBuffer[MAX_PACKET_SIZE - 1] & 0x0F); /* ��ѯHUB�˿�״̬,������TxBuffer�� */
                if(CHRV3IntStatus == ERR_SUCCESS)
                {
                    if(TxBuffer[2] & (1 << (HUB_C_PORT_CONNECTION - 0x10)))
                    {                                                                                     /* ��⵽HUB�˿��ϵĲ���¼� */
                        CHRV3DiskStatus = DISK_DISCONNECT;                                                /* �ٶ�ΪHUB�˿��ϵ�USB�豸�Ͽ� */
                        HubClearPortFeature(TxBuffer[MAX_PACKET_SIZE - 1] & 0x0F, HUB_C_PORT_CONNECTION); /* ���HUB�˿������¼�״̬ */
                    }
                    UHUB_DEV_ADDR = TxBuffer[MAX_PACKET_SIZE - 1]; /* ����USB�����˵�USB��ַָ��USB�豸 */
                    if(TxBuffer[0] & (1 << HUB_PORT_CONNECTION))
                    { /* ����״̬ */
                        if(CHRV3DiskStatus < DISK_CONNECT)
                        {
                            CHRV3DiskStatus = DISK_CONNECT; /* �����Ͽ��� */
                        }
                        return (ERR_SUCCESS); /* USB�豸�Ѿ����ӻ��߶Ͽ����������� */
                    }
                    else
                    {
                        //						CHRV3DiskStatus = DISK_DISCONNECT;
                        //						return( ERR_USB_DISCON );
                        CHRV3DiskStatus = DISK_CONNECT;
                        return (ERR_HUB_PORT_FREE); /* HUB�Ѿ����ӵ���HUB�˿���δ���Ӵ��� */
                    }
                }
                else
                {
                    UHUB_DEV_ADDR = TxBuffer[MAX_PACKET_SIZE - 1]; /* ����USB�����˵�USB��ַָ��USB�豸 */
                    if(CHRV3IntStatus == ERR_USB_DISCON)
                    {
                        //						CHRV3DiskStatus = DISK_DISCONNECT;
                        //						return( ERR_USB_DISCON );
                        goto mDiskDisconn;
                    }
                    else
                    {
                        CHRV3DiskStatus = DISK_CONNECT; /* HUB����ʧ�� */
                        return (CHRV3IntStatus);
                    }
                }
            }
            else
            {   /* ����Root-HUB�µ�USB�豸����,�ⲿHUB��USB�豸�Ѿ����ӻ��߶Ͽ����������� */
                //				CHRV3DiskStatus = DISK_CONNECT;  /* �����Ͽ��� */
                //				return( ERR_SUCCESS );  /* �ⲿHUB��USB�豸�Ѿ����ӻ��߶Ͽ����������� */
                goto mDiskConnect;
            }
        }
        else
        { /* �ⲿHUB�Ͽ� */
            CHRV3DiskStatus = DISK_DISCONNECT;
        }
    }
  #endif
    else
    {
        //		CHRV3DiskStatus = DISK_DISCONNECT;
        //		return( ERR_USB_DISCON );
        goto mDiskDisconn;
    }
}
#endif // NO_DEFAULT_DISK_CONNECT

#ifndef NO_DEFAULT_FILE_ENUMER /* ��Ӧ�ó����ж���NO_DEFAULT_FILE_ENUMER���Խ�ֹĬ�ϵ��ļ���ö�ٻص�����,Ȼ�������б�д�ĳ�������� */
void xFileNameEnumer(void)     /* �ļ���ö�ٻص��ӳ��� */
{
    /* ���ָ��ö�����CHRV3vFileSizeΪ0xFFFFFFFF�����FileOpen����ôÿ������һ���ļ�FileOpen������ñ��ص�����
       �ص�����xFileNameEnumer���غ�FileOpen�ݼ�CHRV3vFileSize������ö��ֱ�����������ļ�����Ŀ¼�����������ǣ�
       �ڵ���FileOpen֮ǰ����һ��ȫ�ֱ���Ϊ0����FileOpen�ص�������󣬱�������CHRV3vFdtOffset�õ��ṹFAT_DIR_INFO��
       �����ṹ�е�DIR_Attr�Լ�DIR_Name�ж��Ƿ�Ϊ�����ļ�������Ŀ¼������¼�����Ϣ������ȫ�ֱ�������������
       ��FileOpen���غ��жϷ���ֵ�����ERR_MISS_FILE��ERR_FOUND_NAME����Ϊ�����ɹ���ȫ�ֱ���Ϊ����������Ч�ļ�����
       ����ڱ��ص�����xFileNameEnumer�н�CHRV3vFileSize��Ϊ1����ô����֪ͨFileOpen��ǰ���������������ǻص��������� */
  #if 0
    UINT8           i;
    UINT16          FileCount;
    PX_FAT_DIR_INFO pFileDir;
    PUINT8          NameBuf;
    pFileDir = (PX_FAT_DIR_INFO)(pDISK_BASE_BUF + CHRV3vFdtOffset); /* ��ǰFDT����ʼ��ַ */
    FileCount = (UINT16)(0xFFFFFFFF - CHRV3vFileSize);              /* ��ǰ�ļ�����ö�����,CHRV3vFileSize��ֵ��0xFFFFFFFF,�ҵ��ļ�����ݼ� */
    if(FileCount < sizeof(FILE_DATA_BUF) / 12)
    {                                             /* ��黺�����Ƿ��㹻���,�ٶ�ÿ���ļ�����ռ��12���ֽڴ�� */
        NameBuf = &FILE_DATA_BUF[FileCount * 12]; /* ���㱣�浱ǰ�ļ����Ļ�������ַ */
        for(i = 0; i < 11; i++)
            NameBuf[i] = pFileDir->DIR_Name[i]; /* �����ļ���,����Ϊ11���ַ�,δ�����ո� */
                                                //		if ( pFileDir -> DIR_Attr & ATTR_DIRECTORY ) NameBuf[ i ] = 1;  /* �ж���Ŀ¼�� */
        NameBuf[i] = 0;                         /* �ļ��������� */
    }
  #endif
}
#endif // NO_DEFAULT_FILE_ENUMER

UINT8 CHRV3LibInit(void) /* ��ʼ��CHRV3�����,�����ɹ�����0 */
{
    if(CHRV3GetVer() < CHRV3_LIB_VER)
        return (0xFF); /* ��ȡ��ǰ�ӳ����İ汾��,�汾̫���򷵻ش��� */
#if DISK_BASE_BUF_LEN > 0
    pDISK_BASE_BUF = &RAM_BUFFER.PDF_BUFFER.DISK_BASE_BUF[0]; /* ָ���ⲿRAM�Ĵ������ݻ����� */
//    pDISK_FAT_BUF = &DISK_BASE_BUF[0];  /* ָ���ⲿRAM�Ĵ���FAT���ݻ�����,������pDISK_BASE_BUF�����Խ�ԼRAM */
	pDISK_FAT_BUF = &RAM_BUFFER.PDF_BUFFER.DISK_FAT_BUF[0];  /* ָ���ⲿRAM�Ĵ���FAT���ݻ�����,������pDISK_BASE_BUF������ٶ� */
/* ���ϣ������ļ���ȡ�ٶ�,��ô�������������е���CHRV3LibInit֮��,��pDISK_FAT_BUF����ָ����һ�������������pDISK_BASE_BUFͬ����С�Ļ����� */
#endif
    CHRV3DiskStatus = DISK_UNKNOWN;          /* δ֪״̬ */
    CHRV3vSectorSizeB = 9;                  /* Ĭ�ϵ��������̵�������512B */
    CHRV3vSectorSize = 512;                 // Ĭ�ϵ��������̵�������512B,��ֵ�Ǵ��̵�������С
    CHRV3vStartLba = 0;                      /* Ĭ��Ϊ�Զ�����FDD��HDD */
    CHRV3vPacketSize = 64;                   /* USB�洢���豸����������:64@FS,512@HS/SS,��Ӧ�ó����ʼ��,ö��U�̺�����Ǹ��ٻ��߳�����ô��ʱ����Ϊ512 */
    pTX_DMA_A_REG = (PUINT32)&R16_UH_TX_DMA; /* ָ����DMA��ַ�Ĵ���,��Ӧ�ó����ʼ�� */
    pRX_DMA_A_REG = (PUINT32)&R16_UH_RX_DMA; /* ָ�����DMA��ַ�Ĵ���,��Ӧ�ó����ʼ�� */
    pTX_LEN_REG = (PUINT16)&R8_UH_TX_LEN;    /* ָ���ͳ��ȼĴ���,��Ӧ�ó����ʼ�� */
    pRX_LEN_REG = (PUINT16)&R8_USB_RX_LEN;   /* ָ����ճ��ȼĴ���,��Ӧ�ó����ʼ�� */

    //CHRV3vRootPort = 0;  /* USB����ѡ��(����Root-hub��������ѡ�˿�) */
    return (ERR_SUCCESS);
}
