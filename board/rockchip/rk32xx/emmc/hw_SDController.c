/****************************************************************
//    CopyRight(C) 2008 by Rock-Chip Fuzhou
//      All Rights Reserved
//�ļ���:hw_SDController.c
//����:RK28 SD/MMC Host Controller driver implement
//����:hcy
//��������:2008-11-08
//���ļ�¼:
//��ǰ�汾:1.00
$Log: hw_SDController.c,v $
Revision 1.4  2011/03/30 02:33:29  Administrator
1��֧������8GB EMMC
2��֧��emmc boot 1��boot 2��д��IDBLOCK����

Revision 1.3  2011/01/27 03:43:27  Administrator
*** empty log message ***

Revision 1.2  2011/01/21 10:12:56  Administrator
֧��EMMC
�Ż�bufferЧ��

Revision 1.1  2011/01/14 10:03:10  Administrator
*** empty log message ***

Revision 1.1  2011/01/07 03:28:02  Administrator
*** empty log message ***

Revision 1.1.1.1  2010/05/17 03:44:52  hds
20100517 �Ƶ�ʤ�ύ��ʼ�汾

Revision 1.1.1.1  2010/03/06 05:22:59  zjd
2010.3.6�ɻƵ�ʤ�ύ��ʼ�汾

Revision 1.1.1.1  2009/12/15 01:46:31  zjd
20091215 �������ύ��ʼ�汾

Revision 1.2  2009/10/13 06:30:20  hcy
hcy 09-10-13 SD���������£�����3�ֿ���ⷽʽ���Ż��˴����������

Revision 1.2  2009/08/18 09:41:48  YYZ
no message

Revision 1.1.1.1  2009/08/18 06:43:27  Administrator
no message

Revision 1.1.1.1  2009/08/14 08:02:01  Administrator
no message

Revision 1.5  2009/07/28 13:00:41  hcy
hcy 09-07-28  CMMB����֧��

Revision 1.4  2009/05/07 12:10:12  hcy
hcy ʹ��С���������Ȱβ壬�����׳��ֶ������ļ������⣬�����ĳ��жϷ�ʽ

Revision 1.3  2009/04/23 13:59:31  hcy
hcy ֧��SDIO����жϴ������ݵķ�ʽ�����ˣ�flush cacheΪDMA��ȡ��Ϻ���flush

Revision 1.2  2009/04/02 03:16:56  hcy
���ӿ�������timeout����

Revision 1.1.1.1  2009/03/16 01:34:07  zjd
20090316 ��ѵ���ṩ��ʼSDK�汾

Revision 1.6  2009/03/13 01:44:44  hcy
�����Ϳ���Դ�����ĳ�GPIO��ʽ

Revision 1.5  2009/03/11 03:33:38  hcy
��������

Revision 1.4  2009/03/09 06:25:35  hcy
(hcy)�ļ�ϵͳ�޸��˿���mountʵ�֣���Ӧ�Ŀ����������޸ģ��ļ�ϵͳ��mount����ǰ��������IO_CTL_GET_MEDIUM_STATUS�����Ϊ�ɿ�����FS_MountDevice��ʵ��mount������ڲ忨����ʱ��Ҫ�����޸ġ�
(yk)����clock����

Revision 1.3  2009/03/07 07:30:18  yk
(yk)����SCUģ���Ƶ�����ã�������к��������룬���³�ʼ�����ã�
����ң�������룬ɾ��FPGA_BOARD�ꡣ
(hcy)SDRAM�����ĳ�28��

Revision 1.2  2009/03/05 12:37:15  hxy
����CVS�汾�Զ�ע��

****************************************************************/
#define  SDC_DRIVER
#include "sdmmc_config.h"

#define RK29_eMMC_Debug 0

extern int emmc_clk_power_save;

#if RK29_eMMC_Debug
static int eMMC_debug = 5;
#define eMMC_printk(n, format, arg...) \
	if (n <= eMMC_debug) {	 \
		printf(format,##arg); \
	}
#else
#define eMMC_printk(n, arg...)
static const int eMMC_debug = 0;
#endif



#ifdef DRIVERS_SDMMC

//#define pSDCReg(n)         ((n == 0) ? ((pSDC_REG_T)SDC0_ADDR) : ((pSDC_REG_T)SDC1_ADDR))
//#define pSDCFIFOADDR(n)    ((n == 0) ? ((volatile uint32 *)SDC0_FIFO_ADDR) : ((volatile uint32 *)SDC1_FIFO_ADDR))

#if eMMC_PROJECT_LINUX
extern struct rk29_eMMC *eMMC_host;

#define pSDCReg(n)         ((n == 0) ? ((pSDC_REG_T)SDC0_ADDR) : ( (n==1) ? ((pSDC_REG_T)SDC1_ADDR):(((pSDC_REG_T)eMMC_host->regs)) ) )
#define pSDCFIFOADDR(n)    ((n == 0) ? ((volatile uint32 *)SDC0_FIFO_ADDR) : ( (n==1)? ((volatile uint32 *)SDC1_FIFO_ADDR): ((volatile uint32 *)(eMMC_host->regs+SD_FIFO_OFFSET)) ) )
#else
#define pSDCReg(n)         ((n == 0) ? ((pSDC_REG_T)SDC0_ADDR) : ( (n==1) ? ((pSDC_REG_T)SDC1_ADDR):(((pSDC_REG_T)SDC2_ADDR)) ) )
#define pSDCFIFOADDR(n)    ((n == 0) ? ((volatile uint32 *)SDC0_FIFO_ADDR) : ( (n==1)? ((volatile uint32 *)SDC1_FIFO_ADDR): ((volatile uint32 *)SDC2_FIFO_ADDR) ) )
#endif

#define SDC_Start(reg, cmd) reg->SDMMC_CMD = cmd;

/* for debug */
#define CD_EVENT    (0x1 << 0)
#define DTO_EVENT   (0x1 << 1)
#define DMA_EVENT   (0x1 << 2)

/****************************************************************/
//������:SDC_ResetFIFO
//����: ���FIFO
//����˵��: 
//           
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
static int32 SDC_ResetFIFO(pSDC_REG_T        pReg)
{
    volatile uint32 value = 0;
    int32 timeOut = 0;

    value = pReg->SDMMC_STATUS;
    if (!(value & FIFO_EMPTY))
    {
        pReg->SDMMC_CTRL |= FIFO_RESET;
        timeOut = 10000;
        while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET)) && (timeOut > 0))
        {
            //SDOAM_Delay(1);
            timeOut--;
        }
        if(timeOut == 0)
        {
            return SDC_SDC_ERROR;
        }
    }

    return SDC_SUCCESS;
}


void eMMC_SetDataHigh(void)
{
    SDC_ResetFIFO(pSDCReg(2));
}

/****************************************************************/
//������:_ControlClock
//����:���������رտ���ʱ�����������ʵ��
//����˵��:nSDCPort   �������   �˿ں�
//         enable     �������  1:����ʱ�ӣ�0:�ر�ʱ��
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
static int32 _ControlClock(SDMMC_PORT_E nSDCPort, bool enable)
{
    volatile uint32 value = 0;
    pSDC_REG_T      pReg = pSDCReg(nSDCPort);
    int32           timeOut = 0;

    //wait previous start to clear
    timeOut = 1000;
    while (((value = pReg->SDMMC_CMD) & START_CMD) && (timeOut > 0))
    {
        SDOAM_Delay(1);
        timeOut--;
    }
    if(timeOut == 0)
    {
        return SDC_SDC_ERROR;
    }

    if (enable)
    {
        value = (/*CCLK_LOW_POWER |*/ CCLK_ENABLE);
    }
    else
    {
        value = (/*CCLK_LOW_POWER |*/ CCLK_DISABLE);
    }
#if (eMMC_PROJECT_LINUX)
    if(emmc_clk_power_save)
    {
        value |= CCLK_LOW_POWER;
    }
#endif    
    pReg->SDMMC_CLKENA = value;
    SDC_Start(pReg, (START_CMD | UPDATE_CLOCK | WAIT_PREV));

    //wait until current start clear
    timeOut = 1000;
    while (((value = pReg->SDMMC_CMD) & START_CMD) && (timeOut > 0))
    {
        SDOAM_Delay(1);
        timeOut--;
    }
    if(timeOut == 0)
    {
        return SDC_SDC_ERROR;
    }

    return SDC_SUCCESS;
}

/****************************************************************/
//������:_ChangeFreq
//����:�����ı俨�Ĺ���Ƶ�����������ʵ��
//����˵��:nSDCPort   �������   �˿ں�
//         freqKHz    �������  ��Ҫ���õ�Ƶ�ʣ���λKHz
//����ֵ:
//���ȫ�ֱ���:
//ע��:freqKHz����Ϊ0�������ر�ʱ�ӣ��͵���SDC_ControlClock
/****************************************************************/
#if 1
//����RK29�޸�
static int32 _ChangeFreq(SDMMC_PORT_E nSDCPort, uint32 freqKHz)
{
    volatile uint32 value = 0;
    pSDC_REG_T      pReg = pSDCReg(nSDCPort);
    uint32          suitMmcClkDiv = 0;
    uint32          suitCclkInDiv = 0;
    uint32          ahbFreq = SDPAM_GetAHBFreq();
    int32           timeOut = 0;
    int32           ret = SDC_SUCCESS;
    uint32          secondFreq;
    if (freqKHz == 0)//Ƶ�ʲ���Ϊ0������������ֳ���Ϊ0
    {
        return SDC_PARAM_ERROR;
    }

    ret = _ControlClock(nSDCPort, FALSE);
    if(ret != SDC_SUCCESS)
    {
        return ret;
    }


    //�ȱ�֤SDMMC����������cclk_in������52MHz������������üĴ������ܷɵ�
    suitMmcClkDiv = ahbFreq/MMCHS_52_FPP_FREQ + ( ((ahbFreq%MMCHS_52_FPP_FREQ)>0) ? 1: 0 );
    if(freqKHz < 12000) //��Ƶ��, ���湩����clk�Ͳ���̫��,��Ȼcmd�����ݵ�hold time����
    {
        suitMmcClkDiv = ahbFreq/freqKHz;
        suitMmcClkDiv &= 0xFFE;//ż����Ƶ
    }
    if(suitMmcClkDiv > 0x3e)
    {
        suitMmcClkDiv = 0x3e;
    }
    
    secondFreq = ahbFreq/suitMmcClkDiv;
    suitCclkInDiv = (secondFreq/freqKHz) + ( ((secondFreq%freqKHz)>0)?1:0 );
    if (((suitCclkInDiv & 0x1) == 1) && (suitCclkInDiv != 1))
    {
        suitCclkInDiv++;  //����1��Ƶ����֤��ż����
    }
    Assert((suitCclkInDiv <= 510), "_ChangeFreq:no find suitable value\n", ahbFreq);
    if(suitCclkInDiv > 510)
    {
        return SDC_SDC_ERROR;
    }

    
    //wait previous start to clear
    timeOut = 1000;
    while (((value = pReg->SDMMC_CMD) & START_CMD) && (timeOut > 0))
    {
        SDOAM_Delay(1);
        timeOut--;
    }
    if(timeOut == 0)
    {
        return SDC_SDC_ERROR;
    }
    
    if(suitCclkInDiv == 1)
    {
        value = 0;
    }
    else
    {
        value = (suitCclkInDiv >> 1);
    }
    /*if(freqKHz>400)
    {
        pReg->SDMMC_CLKDIV = 0;
    }
    else*/
    {
        pReg->SDMMC_CLKDIV = value;
    }
    SDC_Start(pReg, (START_CMD | UPDATE_CLOCK | WAIT_PREV));

    //wait until current start clear
    timeOut = 1000;
    while (((value = pReg->SDMMC_CMD) & START_CMD) && (timeOut > 0))
    {
        SDOAM_Delay(1);
        timeOut--;
    }
    if(timeOut == 0)
    {
        return SDC_SDC_ERROR;
    }
    SDPAM_SetMmcClkDiv(nSDCPort, suitMmcClkDiv);
    return _ControlClock(nSDCPort, TRUE);
}

#else
static int32 _ChangeFreq(SDMMC_PORT_E nSDCPort, uint32 freqKHz)
{
    volatile uint32 value = 0;
    pSDC_REG_T      pReg = pSDCReg(nSDCPort);
    uint32          suitMmcClkDiv = 0;
    uint32          suitCclkInDiv = 0;
    uint32          ahbFreq = SDPAM_GetAHBFreq();
    int32           timeOut = 0;
    int32           ret = SDC_SUCCESS;

    if (freqKHz == 0)//Ƶ�ʲ���Ϊ0������������ֳ���Ϊ0
    {
        return SDC_PARAM_ERROR;
    }

    ret = _ControlClock(nSDCPort, FALSE);
    if(ret != SDC_SUCCESS)
    {
        return ret;
    }

#if 0
    minMmcClkDiv = ahbFreq/50000 + (((ahbFreq%50000) > 0) ? 1:0);
    Assert((minMmcClkDiv <= 8),"_ChangeFreq:max mmc_clk_div error\n", ahbFreq);
    
    minFreqOffset    = 0xFFFFFFFF;
    suitMmcClkDiv    = 0;
    suitCclkInDiv    = 0;
    
    for(mmcClkDiv = minMmcClkDiv; mmcClkDiv <= 8; mmcClkDiv++)
    {
        cclkInDiv = ahbFreq/(mmcClkDiv*freqKHz)
                    + (((ahbFreq%(mmcClkDiv*freqKHz)) > 0) ? 1:0);
        if (((cclkInDiv & 0x1) == 1) && (cclkInDiv != 1))
        {
            cclkInDiv++;  //����1��Ƶ����֤��ż����
        }
        if(cclkInDiv > 510)
        {
            continue;
        }

        newCardFreq = (ahbFreq/(mmcClkDiv*cclkInDiv))\
                       + (((ahbFreq%(mmcClkDiv*cclkInDiv)) > 0) ? 1:0);
        if ((newCardFreq <= freqKHz) && ((freqKHz - newCardFreq) < minFreqOffset))
        {
            suitMmcClkDiv    = mmcClkDiv;
            suitCclkInDiv    = cclkInDiv;
            minFreqOffset    = (freqKHz - newCardFreq);
            if(minFreqOffset == 0)
            {
                break;
            }
        }
    }
    
    Assert(((suitMmcClkDiv != 0) \
            && (suitCclkInDiv != 0)\
            && (suitMmcClkDiv <= 8)\
            && (suitCclkInDiv <= 510)),\
            "_ChangeFreq:no find suitable value\n", ahbFreq);
#else
    if(freqKHz <= 400)
    {
        suitMmcClkDiv = 8;
        suitCclkInDiv = ((SDPAM_MAX_AHB_FREQ*1000)/(freqKHz << 3)) + ((((SDPAM_MAX_AHB_FREQ*1000)%(freqKHz << 3)) > 0) ? 1:0);
        if (((suitCclkInDiv & 0x1) == 1) && (suitCclkInDiv != 1))
        {
            suitCclkInDiv++;  //����1��Ƶ����֤��ż����
        }
        Assert((suitCclkInDiv <= 510), "_ChangeFreq:no find suitable value\n", ahbFreq);
        if(suitCclkInDiv > 510)
        {
            return SDC_SDC_ERROR;
        }
    }
    else
    {
        suitMmcClkDiv = ahbFreq/freqKHz + (((ahbFreq%freqKHz) > 0) ? 1:0);
        if(suitMmcClkDiv > 8)
        {
            suitMmcClkDiv = 8;
        }
        suitCclkInDiv = 1;
    }
#endif
    
    //wait previous start to clear
    timeOut = 1000;
    while (((value = pReg->SDMMC_CMD) & START_CMD) && (timeOut > 0))
    {
        SDOAM_Delay(1);
        timeOut--;
    }
    if(timeOut == 0)
    {
        return SDC_SDC_ERROR;
    }
    
    if(suitCclkInDiv == 1)
    {
        value = 0;
    }
    else
    {
        value = (suitCclkInDiv >> 1);
    }
    pReg->SDMMC_CLKDIV = value;
    SDC_Start(pReg, (START_CMD | UPDATE_CLOCK | WAIT_PREV));

    //wait until current start clear
    timeOut = 1000;
    while (((value = pReg->SDMMC_CMD) & START_CMD) && (timeOut > 0))
    {
        SDOAM_Delay(1);
        timeOut--;
    }
    if(timeOut == 0)
    {
        return SDC_SDC_ERROR;
    }
    SDPAM_SetMmcClkDiv(nSDCPort, suitMmcClkDiv);

    return _ControlClock(nSDCPort, TRUE);
}

#endif

extern void FlashCsTest(uint8 data);
extern void rkNand_cond_resched(void);

/****************************************************************/
//������:_WaitCardBusy
//����:�ȴ�ָ���˿��ϵĿ�busy���
//����˵��:nSDCPort   �������   �˿ں�
//����ֵ:SDC_BUSY_TIMEOUT      �ȴ�ʱ��̫����ʱ�����
//       SDC_SUCCESS           �ɹ�
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
static int32 _WaitCardBusy(SDMMC_PORT_E       nSDCPort)
{
    volatile uint32 value = 0;
    pSDC_REG_T      pReg = pSDCReg(nSDCPort);
    uint32          timeout = 0;

    //wait busy
    timeout = 0;
    //FlashCsTest(1);
    while ((value = pReg->SDMMC_STATUS) & DATA_BUSY)
    {
#if (eMMC_PROJECT_LINUX)
        if(FlashWaitBusyScheduleEn)
            rkNand_cond_resched();
#endif
        SDOAM_Delay(1);
        timeout++;
        if (timeout > 2500000) //д�ʱ��2500ms
        {
            //FlashCsTest(2);
            //printk("_WaitCardBusy timeout!!!!!!\n");
            //while(1);
            return SDC_BUSY_TIMEOUT;
        }
    }
    //FlashCsTest(0);
    return SDC_SUCCESS;
}

/****************************************************************/
//������:_SDC0WriteCallback
//����:SDMMC0��������DMAд����ʱ��callback������
//����˵��:
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
static void _SDC0DMACallback(void)
{
    gSDCInfo[SDC0].intInfo.transLen = gSDCInfo[SDC0].intInfo.desLen;
}

/****************************************************************/
//������:_SDC1ReadCallback
//����:SDMMC1��������DMA������ʱ��callback������
//����˵��:
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
static void _SDC1DMACallback(void)
{
    gSDCInfo[SDC1].intInfo.transLen = gSDCInfo[SDC1].intInfo.desLen;
}

/****************************************************************/
//������:_SDC1ReadCallback
//����:SDMMC2��������DMA������ʱ��callback������
//����˵��:
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
static void _SDC2DMACallback(void)
{
    gSDCInfo[SDC2].intInfo.transLen = gSDCInfo[SDC2].intInfo.desLen;
}

void eMMCcallback(void)
{
    gSDCInfo[SDC2].intInfo.transLen = gSDCInfo[SDC2].intInfo.desLen;
}


/****************************************************************/
//������:_PrepareForWriteData
//����:������Ҫд�����ݳ��ȣ����ú�DMA��ʼ����
//����˵��:nSDCPort   �������   �˿ں�
//         pDataBuf   �������   Ҫд���ݵĵ�ַ
//         dataLen    �������   Ҫд�����ݳ��ȣ���λ�ֽ�
//         cb         �������   DMA��callback����
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
static int32 _PrepareForWriteData(SDMMC_PORT_E        nSDCPort, void *pDataBuf, uint32 dataLen, pFunc cb)
{
#if EN_SD_DMA
    volatile uint32  value = 0;
    pSDC_REG_T       pReg = pSDCReg(nSDCPort);
#endif
    volatile uint32 *pFIFO = pSDCFIFOADDR(nSDCPort);
    uint32           i = 0;
    //ָ����32 bit��uint32ָ�룬�Ϳ�������SDMMC FIFOҪ���32 bits�����Ҫ���ˣ����������������û��4�ֽڶ��룬
    //Ҳ����Ϊ����uint32ָ�룬ÿ����FIFO������4�ֽڶ���ġ�
    uint32          *pBuf = (uint32 *)pDataBuf;
    //�������dataLen�Ĳ��������ǵ�SDMMC������Ҫ��32bit�����Ҫ�����Կ�������dataLen�Ĳ����Ƚ��鷳
    //SDMMC������Ҫ��32bit���룬��:Ҫ����13 byte�����ݣ�ʵ�ʴ���FIFO(дʱ)���FIFO����(��ʱ)�����ݱ���Ҫ16��,
    uint32           count = (dataLen >> 2) + ((dataLen & 0x3) ? 1:0);   //��32bitָ��������������ݳ���Ҫ��4
    
#if EN_SD_DMA
    if (count <= FIFO_DEPTH)
    {
        for (i=0; i<count; i++)
        {
            *pFIFO = pBuf[i];
        }
    }
    else
    {
        gSDCInfo[nSDCPort].intInfo.desLen = count;
        gSDCInfo[nSDCPort].intInfo.transLen = 0;
        gSDCInfo[nSDCPort].intInfo.pBuf = (uint32 *)pDataBuf;
        
        if(!SDPAM_DMAStart(nSDCPort, (uint32)pFIFO, (uint32)pBuf, count, 1, cb))
        {
            Assert(0, "_PrepareForWriteData:DMA busy\n", 0);
            return SDC_DMA_BUSY;
        }
        value = pReg->SDMMC_CTRL;
        value |= ENABLE_DMA;
        pReg->SDMMC_CTRL = value;
        #if eMMC_PROJECT_LINUX
        rk29_dma_ctrl(eMMC_host->dma_chn, RK29_DMAOP_START);
        #endif
    }
#else
    gSDCInfo[nSDCPort].intInfo.desLen = count;
    gSDCInfo[nSDCPort].intInfo.pBuf = pBuf;
    //if write, fill FIFO to full before start
    if (count > FIFO_DEPTH)
    {
        for (i=0; i<FIFO_DEPTH; i++)
        {
            *pFIFO = pBuf[i];
        }
        gSDCInfo[nSDCPort].intInfo.transLen = FIFO_DEPTH;
    }
    else
    {
        for (i=0; i<count; i++)
        {
            *pFIFO = pBuf[i];
        }
        gSDCInfo[nSDCPort].intInfo.transLen = count;
    }
#endif
    return SDC_SUCCESS;
}

/****************************************************************/
//������:_PrepareForReadData
//����:������Ҫ�������ݳ��ȣ����ú�DMA��ʼ����
//����˵��:nSDCPort   �������   �˿ں�
//         pDataBuf   �������   Ҫ�����ݵĵ�ַ
//         dataLen    �������   Ҫ�������ݳ��ȣ���λ�ֽ�
//         cb         �������   DMA��callback����
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
static int32 _PrepareForReadData(SDMMC_PORT_E         nSDCPort, void *pDataBuf, uint32 dataLen, pFunc cb)
{
#if EN_SD_DMA
    volatile uint32 value = 0;
    pSDC_REG_T      pReg = pSDCReg(nSDCPort);
    uint32          count = 0;
#endif

#if EN_SD_DMA
/*    uint8 *testbuf= (uint8 *)pDataBuf;
    eMMC_printk(5, "%s...%d...flush the read buf.....\n", __FILE__,__LINE__);
    for(count=0; count<(dataLen);count++)
    {
        testbuf[count]=0;
    }
   */ 

   //eMMC_printk(5, "%s..%d..   eMMC_host->dma_addr=%x \n", __FUNCTION__, __LINE__, eMMC_host->dma_addr);
   
    //�������dataLen�Ĳ��������ǵ�SDMMC������Ҫ��32bit�����Ҫ�����Կ�������dataLen�Ĳ����Ƚ��鷳
    //SDMMC������Ҫ��32bit���룬��:Ҫ����13 byte�����ݣ�ʵ�ʴ���FIFO(дʱ)���FIFO����(��ʱ)�����ݱ���Ҫ16��
    count = (dataLen >> 2) + ((dataLen & 0x3) ? 1:0);
    if (count > (RX_WMARK+1))  //���ܵ����ݳ���С�ڵ���RX_WMARK+1ʱ��������DMA��������(�����datasheet˵�Ĳ���һ��)
    {
        gSDCInfo[nSDCPort].intInfo.desLen = (dataLen >> 2);
        gSDCInfo[nSDCPort].intInfo.transLen = 0;
        gSDCInfo[nSDCPort].intInfo.pBuf = (uint32 *)pDataBuf;
        if(!SDPAM_DMAStart(nSDCPort, (uint32)pDataBuf, (uint32)pSDCFIFOADDR(nSDCPort), (dataLen >> 2), 0, cb))
        {
            Assert(0, "_PrepareForReadData:DMA busy\n", 0);
            return SDC_DMA_BUSY;
        }
        value = pReg->SDMMC_CTRL;
        value |= ENABLE_DMA;
        pReg->SDMMC_CTRL = value;

        #if eMMC_PROJECT_LINUX
        rk29_dma_ctrl(eMMC_host->dma_chn, RK29_DMAOP_START);	
        #endif
    }
#else
    gSDCInfo[nSDCPort].intInfo.desLen = (dataLen >> 2);
    gSDCInfo[nSDCPort].intInfo.pBuf = (uint32 *)pDataBuf;
    gSDCInfo[nSDCPort].intInfo.transLen = 0;
#endif
    return SDC_SUCCESS;
}

/****************************************************************/
//������:_ReadRemainData
//����:��ȡFIFO�ڵ�ʣ�����ݣ�����С��waterlevel�����ݲ���4�ֽڵ�
//     ������ʱ��ʣ���С��4�ֽ�����Ҳ�������ȡ
//����˵��:nSDCPort       �������   �˿ں�
//         pDataBuf       �������   Ҫ�����ݵĵ�ַ
//         originalLen    �������   ԭʼ���ݵĳ��ȣ�Ҫͨ���������
//                                   ������ʣ������ݣ���λ�ֽ�
//����ֵ:
//���ȫ�ֱ���:
//ע��:originalLen��ԭʼ���ݵĳ��ȣ������Ǽ����Ѿ����յ������ݺ�ĳ���
/****************************************************************/
static int32 _ReadRemainData(SDMMC_PORT_E         nSDCPort, uint32 originalLen, void *pDataBuf)
{
    volatile uint32  value = 0;
    volatile uint32 *pFIFO = pSDCFIFOADDR(nSDCPort);
    pSDC_REG_T       pReg = pSDCReg(nSDCPort);
    uint32           i = 0;
    //ָ����32 bit��uint32ָ�룬�Ϳ�������SDMMC FIFOҪ���32 bits�����Ҫ���ˣ����������������û��4�ֽڶ��룬
    //Ҳ����Ϊ����uint32ָ�룬ÿ����FIFO������4�ֽڶ���ġ�
    uint32          *pBuf = (uint32 *)pDataBuf;
    uint8           *pByteBuf = (uint8 *)pDataBuf;
    uint32           lastData = 0;
    //�������dataLen�Ĳ��������ǵ�SDMMC������Ҫ��32bit�����Ҫ�����Կ�������dataLen�Ĳ����Ƚ��鷳
    //SDMMC������Ҫ��32bit���룬��:Ҫ����13 byte�����ݣ�ʵ�ʴ���FIFO(дʱ)���FIFO����(��ʱ)�����ݱ���Ҫ16��
    uint32           count = (originalLen >> 2) + ((originalLen & 0x3) ? 1:0);   //��32bitָ��������������ݳ���Ҫ��4
    
#if EN_SD_DMA
    //DMA����ʱ��ֻ��dataLen/4С�ڵ���RX_WMARK+1,����dataLenû4�ֽڶ���ģ��Ż���ʣ������
    value = pReg->SDMMC_STATUS;
    if (!(value & FIFO_EMPTY))
    {
        if (count <= (RX_WMARK+1))
        {
            i = 0;
            while ((i<(originalLen >> 2))&&(!(value & FIFO_EMPTY)))
            {
                pBuf[i++] = *pFIFO;
                value = pReg->SDMMC_STATUS;
            }
        }

        if (count > (originalLen >> 2))
        {
            Assert((!(value & FIFO_EMPTY)), "_ReadRemainData:need data, but FIFO empty\n", value);
            if(value & FIFO_EMPTY)
            {
                return SDC_SDC_ERROR;
            }
            lastData = *pFIFO;
            //���ʣ���1-3���ֽ�
            for (i=0; i<(originalLen & 0x3); i++)
            {
                pByteBuf[(originalLen & 0xFFFFFFFC) + i] = (uint8)((lastData >> (i << 3)) & 0xFF); //ֻ����CPUΪС�ˣ�little-endian
            }
        }
    }
#else
    //�жϴ���ʱ����dataLen/4С�ڵ���RX_WMARK+1,����dataLenû4�ֽڶ���ģ��������ʣ������ݴﲻ��RX_WMARK+1������ʣ������
    value = pReg->SDMMC_STATUS;
    if (!(value & FIFO_EMPTY))
    {
        while ((gSDCInfo[nSDCPort].intInfo.transLen < gSDCInfo[nSDCPort].intInfo.desLen) && (!(value & FIFO_EMPTY)))
        {
            pBuf[gSDCInfo[nSDCPort].intInfo.transLen++] = *pFIFO;
            value = pReg->SDMMC_STATUS;
        }
        //printf("_ReadRemainData transLen = %d originalLen = %d\n",gSDCInfo[nSDCPort].intInfo.transLen,originalLen);
        if (count > (originalLen >> 2))
        {
            Assert((!(value & FIFO_EMPTY)), "_ReadRemainData:need data, but FIFO empty\n", value);
            if(value & FIFO_EMPTY)
            {
                return SDC_SDC_ERROR;
            }
            lastData = *pFIFO;
            //���ʣ���1-3���ֽ�
            for (i=0; i<(originalLen & 0x3); i++)
            {
                pByteBuf[(originalLen & 0xFFFFFFFC) + i] = (uint8)((lastData >> (i << 3)) & 0xFF);  //ֻ����CPUΪС�ˣ�little-endian
            }
        }
    }
#endif
    return SDC_SUCCESS;
}

/****************************************************************/
//������:_SDCISTHandle
//����:SDMMC controller�жϷ������
//����˵��:
//����ֵ:
//���ȫ�ֱ���:gSDCInfo[]
//ע��:
/****************************************************************/
static void _SDCISTHandle(SDMMC_PORT_E nSDCPort)
{
    volatile uint32  value = 0;
    pSDC_REG_T       pReg = pSDCReg(nSDCPort);
    uint32           data = 0;
    uint32          * pbuf32 ;
    uint32          count;
    SDC_INT_INFO_T   * p_intInfo = &gSDCInfo[nSDCPort].intInfo;   
    //eMMC_printk(5,"%s, %s  %d, ====irq callback=========\n",__FUNCTION__, __FILE__,__LINE__);
#if !(EN_SD_DMA)
    volatile uint32 *pFIFO = pSDCFIFOADDR(nSDCPort);
    uint32 i = 0;
#endif
    //eMMC_printk(5,"%s, %s  %d, ====irq callback===========\n",__FUNCTION__, __FILE__,__LINE__);

    value = pReg->SDMMC_MINTSTS;
#if EN_SD_DMA
    if (value & (RXDR_INT | TXDR_INT))
    {
        Assert(0, "_SDCISTHandle:dma mode, but rx tx int\n", value);
    }
#else
    
    //eMMC_printk(5,"%s, %s  %d, ========xbw===========\n",__FUNCTION__, __FILE__,__LINE__);
    if (value & RXDR_INT)
    {   
        //eMMC_printk(5, "%s, %s  %d, ========xbw===========\n",__FUNCTION__, __FILE__,__LINE__);
        //Assert((gSDCInfo[nSDCPort].intInfo.desLen > 0), "_SDCISTHandle:rx len <= 0\n", gSDCInfo[nSDCPort].intInfo.desLen);
        //Assert(!(gSDCInfo[nSDCPort].intInfo.rw), "_SDCISTHandle:read, but have TXDR int\n", gSDCInfo[nSDCPort].intInfo.rw);
        //read data
        //printf("read data ini\t\n");
//Re_read:        
        pbuf32 = &p_intInfo->pBuf[p_intInfo->transLen];
        for (i=0; i<(RX_WMARK+1); i++)
        {
            *pbuf32++ = *pFIFO;
        }
        p_intInfo->transLen += (RX_WMARK+1);
        pReg->SDMMC_RINISTS = RXDR_INT;     // û��ʹ��dma�������ݱȽ����������жϱ���ٿ�ʼ������
        //if(pReg->SDMMC_MINTSTS & RXDR_INT) //�������ݣ��ж��ֲ�����
        //    goto Re_read;        
    }
    if (value & TXDR_INT)
    {
    
        //eMMC_printk("%s, %s  %d, ========xbw===========\n",__FUNCTION__, __FILE__,__LINE__);
        //Assert((gSDCInfo[nSDCPort].intInfo.desLen > 0), "_SDCISTHandle:tx len <= 0\n", gSDCInfo[nSDCPort].intInfo.desLen);
        //Assert((gSDCInfo[nSDCPort].intInfo.rw), "_SDCISTHandle:write, but have RXDR int\n", gSDCInfo[nSDCPort].intInfo.rw);
        //fill data
        pbuf32 = &p_intInfo->pBuf[p_intInfo->transLen];
        if ((p_intInfo->desLen - p_intInfo->transLen) > (FIFO_DEPTH - TX_WMARK))
        {
            count = (FIFO_DEPTH - TX_WMARK);
        }
        else
        {
            count = (p_intInfo->desLen - p_intInfo->transLen);
        }
        
        for (i=0; i<count; i++)
        {
             *pFIFO = *pbuf32++;
        }
        p_intInfo->transLen += count;
        pReg->SDMMC_RINISTS = TXDR_INT;
    }
#endif

    //eMMC_printk(5, "%s, %s  %d, ========xbw===========\n",__FUNCTION__, __FILE__,__LINE__);

    if (value & SDIO_INT)
    {
        data = pReg->SDMMC_INTMASK;
        data &= ~(SDIO_INT);
        pReg->SDMMC_INTMASK = data;
        pReg->SDMMC_RINISTS = SDIO_INT;
        
        if(gSDCInfo[nSDCPort].pSdioCb)
        {
            gSDCInfo[nSDCPort].pSdioCb();
        }
        if(gSDCInfo[nSDCPort].bSdioEn)
        {
            data |= SDIO_INT;
            pReg->SDMMC_INTMASK = data;
        }
    }
    if (value & CD_INT)
    {
        SDOAM_SetEvent(gSDCInfo[nSDCPort].event, CD_EVENT);
        pReg->SDMMC_RINISTS = CD_INT;
    }
    if (value & DTO_INT)
    {   
        //eMMC_printk(5, "%s, %s  %d, ==DTO_INT ======xbw===========\n",__FUNCTION__, __FILE__,__LINE__);
        SDOAM_SetEvent(gSDCInfo[nSDCPort].event, DTO_EVENT);
        pReg->SDMMC_RINISTS = DTO_INT;
    }
    if (value & SBE_INT)
    {
        SDOAM_SetEvent(gSDCInfo[nSDCPort].event, DTO_EVENT);
        data = 0;
        #if !(EN_SD_DMA)
        data |= (RXDR_INT | TXDR_INT);
        #endif
        #if EN_SD_INT
        data |= (FRUN_INT | DTO_INT | CD_INT);
        #else
        data |= (FRUN_INT);
        #endif
        if(SDC0 == nSDCPort)
        {
            #if (SDMMC0_DET_MODE == SD_CONTROLLER_DET)
            data |= CDT_INT;
            #endif
        }
        else if(SDC1 == nSDCPort)
        {
            #if (SDMMC1_DET_MODE == SD_CONTROLLER_DET)
            data |= CDT_INT;
            #endif
        }
        else
        {
            //eMMC
            ;
        }
        
        pReg->SDMMC_INTMASK = data;
    }
    if (value & CDT_INT)
    {
        /*data = pReg->SDMMC_CDETECT;
        if(0) //////(data & NO_CARD_DETECT)  ///���迨��Զ����
        {
            //���İβ壬������÷���Ϣ������һ��ȫ�ֱ����Ļ�(����gCardState)����������������⣬
            //�Ѿ�ʶ��ÿ��õĿ�����ʹ�ù����У��ҿ��ٰγ���Ȼ������ٲ��룬��ʱ��ѯgCardState
            //�ĳ��������������Ӧ���ͻ���Ϊ��û�б��������������κζ��������Կ�Ҳ�Ͳ�����ʹ����
            //���������Ϣ�Ļ����ͻ��յ������γ���Ȼ���ֱ����룬���Ի������ٳ�ʼ��һ�£����ֿ�����
            SDOAM_SendMsg(MSG_CARD_REMOVE, nSDCPort);
        }
        else
        {
            SDOAM_SendMsg(MSG_CARD_INSERT, nSDCPort);
        }*/
        pReg->SDMMC_RINISTS = CDT_INT;
    }
    if (value & FRUN_INT)
    {
        pReg->SDMMC_RINISTS = FRUN_INT;
        Assert(0, "_SDCISTHandle:overrun or underrun\n", value);
    }
    if (value & HLE_INT)
    {
        Assert(0, "_SDCISTHandle:hardware locked write error\n", value);
    }

    return;
}

/****************************************************************/
//������:_SDC0IST
//����:SDMMC0�жϷ������
//����˵��:
//����ֵ:
//���ȫ�ֱ���:gSDCInfo[SDC0].intInfo
//ע��:
/****************************************************************/
void _SDC0IST(void)
{
    _SDCISTHandle(SDC0);
}

/****************************************************************/
//������:_SDC1IST
//����:SDMMC1�жϷ������
//����˵��:
//����ֵ:
//���ȫ�ֱ���:gSDCInfo[SDC1].intInfo
//ע��:
/****************************************************************/
void _SDC1IST(void)
{
    _SDCISTHandle(SDC1);
}

/****************************************************************/
//������:_SDC2IST
//����:SDMMC1�жϷ������
//����˵��:
//����ֵ:
//���ȫ�ֱ���:gSDCInfo[SDC2].intInfo
//ע��:
/****************************************************************/
//static
void _SDC2IST(void)
{
    _SDCISTHandle(SDC2);
}

/*void EMMC_ISR(void)
{
    eMMC_printk(5, "%s, %s  %d, ====IRQ=========\n",__FUNCTION__, __FILE__,__LINE__);
    _SDCISTHandle(SDC2);
}*/
/****************************************************************/
//������:SDC_Init
//����:SDMMC controller��ʼ������
//����˵��:
//����ֵ:TRUE  ��ʼ���ɹ�
//       FALSE ��ʼ��ʧ��
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
void SDC_Init(uint32 CardId)
{
    volatile uint32  value = 0;
    pSDC_REG_T       pReg = NULL;
    SDMMC_PORT_E     nSDCPort = SDC_MAX;
    int32            bPortEnable[SDC_MAX];
    int32            bPowerCtlEnable[SDC_MAX];
    int32            bUseGpioDet[SDC_MAX];
    int32            timeOut = 0;
    uint32          clkvalue, clkdiv;
    
    if(CardId == 0)
    {
        /* init global variable */
        gSDCInfo[SDC0].pReg             = (pSDC_REG_T)SDC0_ADDR;
        gSDCInfo[SDC0].pReg->SDMMC_PWREN = 0; //emmc 1 en, sdcard 0 en
        gSDCInfo[SDC0].intInfo.transLen = 0;
        gSDCInfo[SDC0].intInfo.desLen   = 0;
        gSDCInfo[SDC0].intInfo.pBuf     = NULL;
        gSDCInfo[SDC0].cardFreq         = 0;
        gSDCInfo[SDC0].updateCardFreq   = FALSE;
        gSDCInfo[SDC0].event            = SDOAM_CreateEvent((uint8*)"SDC0");
        Assert((gSDCInfo[SDC0].event != NULL), "SDC_Init:Create SDC0 event failed\n", (uint32)gSDCInfo[SDC1].event);
        if(gSDCInfo[SDC0].event == NULL)
        {
            return;
        }
#if (SDMMC0_BUS_WIDTH == 1)
        gSDCInfo[SDC0].busWidth = BUS_WIDTH_1_BIT;
#elif (SDMMC0_BUS_WIDTH == 4)
        gSDCInfo[SDC0].busWidth = BUS_WIDTH_4_BIT;
#elif (SDMMC0_BUS_WIDTH == 8)
        gSDCInfo[SDC0].busWidth = BUS_WIDTH_8_BIT;
#else
        #error SDMMC0 Bus Width Not Support
#endif

#if (SDMMC0_USED)
        bPortEnable[SDC0] = 1;
#else
        bPortEnable[SDC0] = 0;
#endif

#if (SDMMC0_EN_POWER_CTL)
        bPowerCtlEnable[SDC0] = 1;
#else
        bPowerCtlEnable[SDC0] = 0;
#endif

#if (SDMMC0_DET_MODE == SD_GPIO_DET)
        bUseGpioDet[SDC0] = 1;
#else
        bUseGpioDet[SDC0] = 0;
#endif
    }
    else if(CardId == 1)
    {
        gSDCInfo[SDC1].pReg             = (pSDC_REG_T)SDC1_ADDR;
        gSDCInfo[SDC1].pReg->SDMMC_PWREN = 0;          //emmc 1 en, sdcard 0 en
        gSDCInfo[SDC1].intInfo.transLen = 0;
        gSDCInfo[SDC1].intInfo.desLen   = 0;
        gSDCInfo[SDC1].intInfo.pBuf     = NULL;
        gSDCInfo[SDC1].cardFreq         = 0;
        gSDCInfo[SDC1].updateCardFreq   = FALSE;
        gSDCInfo[SDC1].event            = SDOAM_CreateEvent((uint8*)"SDC1");
        Assert((gSDCInfo[SDC1].event != NULL), "SDC_Init:Create SDC1 event failed\n", (uint32)gSDCInfo[SDC1].event);
        if(gSDCInfo[SDC1].event == NULL)
        {
            return;
        }
#if (SDMMC1_BUS_WIDTH == 1)
        gSDCInfo[SDC1].busWidth = BUS_WIDTH_1_BIT;
#elif (SDMMC1_BUS_WIDTH == 4)
        gSDCInfo[SDC1].busWidth = BUS_WIDTH_4_BIT;
#else
        #error SDMMC1 Bus Width Not Support
#endif

#if (SDMMC1_USED)
        bPortEnable[SDC1] = 1;
#else
        bPortEnable[SDC1] = 0;
#endif

#if (SDMMC1_EN_POWER_CTL)
        bPowerCtlEnable[SDC1] = 1;
#else
        bPowerCtlEnable[SDC1] = 0;
#endif

#if (SDMMC1_DET_MODE == SD_GPIO_DET)
        bUseGpioDet[SDC1] = 1;
#else
        bUseGpioDet[SDC1] = 0;
#endif
    }
    else
    {
//eMMC
        gSDCInfo[SDC2].pReg             = (pSDC_REG_T)SDC2_ADDR;
        gSDCInfo[SDC2].intInfo.transLen = 0;
        gSDCInfo[SDC2].intInfo.desLen   = 0;
        gSDCInfo[SDC2].intInfo.pBuf     = NULL;
        gSDCInfo[SDC2].cardFreq         = 0;
        gSDCInfo[SDC2].updateCardFreq   = FALSE;
        gSDCInfo[SDC2].event            = SDOAM_CreateEvent((uint8*)"SDC2");
        Assert((gSDCInfo[SDC2].event != NULL), "SDC_Init:Create SDC1 event failed\n", (uint32)gSDCInfo[SDC2].event);
        if(gSDCInfo[SDC2].event == NULL)
        {
            return;
        }
#if (SDMMC2_BUS_WIDTH == 1)
        gSDCInfo[SDC2].busWidth = BUS_WIDTH_1_BIT;
#elif (SDMMC2_BUS_WIDTH == 4)
        gSDCInfo[SDC2].busWidth = BUS_WIDTH_4_BIT;
#else
        gSDCInfo[SDC2].busWidth = BUS_WIDTH_8_BIT;
#endif
    
#if (SDMMC2_USED)
        bPortEnable[SDC2] = 1;
#else
        bPortEnable[SDC2] = 0;
#endif
    
#if (SDMMC2_EN_POWER_CTL)
        bPowerCtlEnable[SDC2] = 1;
#else
        bPowerCtlEnable[SDC2] = 0;
#endif
    
#if (SDMMC2_DET_MODE == SD_GPIO_DET)
        bUseGpioDet[SDC2] = 1;
#else
        bUseGpioDet[SDC2] = 0;
#endif
    }

    //for(nSDCPort = SDC0; nSDCPort < SDC_MAX; nSDCPort++)
    nSDCPort = CardId;
    {
        //�ȱ�֤SDMMC����������cclk_in������52MHz������������üĴ������ܷɵ�
        clkvalue = SDPAM_GetAHBFreq();        
        clkdiv =( clkvalue + (uint32)MMCHS_52_FPP_FREQ - 1)/(uint32)MMCHS_52_FPP_FREQ;
        if(clkdiv > 0x3f)
        {
            //PLLƵ��̫��
            return;
        }
        SDPAM_SetMmcClkDiv(nSDCPort, clkdiv);
        
        if(bPortEnable[nSDCPort])
        {
            /* enable SDMMC port */
            SDPAM_IOMUX_SetSDPort(nSDCPort, BUS_WIDTH_1_BIT);
            if(bPowerCtlEnable[nSDCPort])
            {
                SDPAM_IOMUX_PwrEnGPIO(nSDCPort);
                SDPAM_ControlPower(nSDCPort, FALSE);
            }
            if(bUseGpioDet[nSDCPort])
            {
                SDPAM_IOMUX_DetGPIO(nSDCPort);
            }
            SDC_ResetController(nSDCPort);
            if(SDC_IsCardPresence(nSDCPort))
            {
                SDOAM_SendMsg(MSG_CARD_INSERT, nSDCPort);
            }
            else
            {
                _ControlClock(nSDCPort, FALSE);
                SDPAM_SDCClkEnable(nSDCPort, FALSE);
            }
        }
        else
        {
            gSDCInfo[nSDCPort].busWidth = BUS_WIDTH_INVALID;
            /* reset */
            SDPAM_SDCReset(nSDCPort);
            pReg = pSDCReg(nSDCPort);
            pReg->SDMMC_CTRL = (FIFO_RESET | SDC_RESET);
            timeOut = 1000;
            while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET | SDC_RESET)) && (timeOut > 0))
            {
                SDOAM_Delay(1);
                timeOut--;
            }
            if(timeOut == 0)
            {
                SDPAM_SDCClkEnable(nSDCPort, FALSE);
                //continue;
            }
            /* config interrupt */
            pReg->SDMMC_RINISTS = 0xFFFFFFFF;
            pReg->SDMMC_CTRL = 0;  //interrupt disable
            _ControlClock(nSDCPort, FALSE);
            SDPAM_SDCClkEnable(nSDCPort, FALSE);
        }
    }
}

/****************************************************************/
//������:SDC_IsCardIdValid
//����:���cardId�Ƿ�Ϊ��Ч��id
//����˵��:cardId        �������  ��Ҫ����cardId
//����ֵ: TRUE       ��ȷ��cardId
//        FALSE      �����cardId
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
bool SDC_IsCardIdValid(int32 cardId)
{
    SDMMC_PORT_E    nSDCPort = (SDMMC_PORT_E)cardId;

    #if Only_Controller2_USED
    //Ϊ�˲�Ӱ�������1��2(��sdmmc��sdio)
    if(SDC2 == nSDCPort)
    {
        return TRUE;
    }
    #endif
    
#if ((SDMMC0_USED == 1) && (SDMMC1_USED == 1))
    if (nSDCPort >= SDC_MAX)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
#elif ((SDMMC0_USED == 1) && (SDMMC1_USED == 0))
    if (nSDCPort != SDC0)
    {
        return FALSE;
    }
    else
    {
        return TRUE; //eMMC�ߴ��ж�
    }
#elif ((SDMMC0_USED == 0) && (SDMMC1_USED == 1))
    if (nSDCPort != SDC1)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
#else
    return FALSE;
#endif
}

/****************************************************************/
//������:SDC_IsCardPresence
//����:��鿨�Ƿ��ڿ�����
//����˵��:cardId        �������  ��Ҫ���Ŀ�
//����ֵ: TRUE       ���ڿ�����
//        FALSE      �����ڿ�����
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
bool SDC_IsCardPresence(int32 cardId)
{
    volatile uint32 value = 0;
    SDMMC_PORT_E    nSDCPort = (SDMMC_PORT_E)cardId;

    return TRUE; //��Ĭ�ϣ���Զ����
    
    if(SDC0 == nSDCPort)
    {
        #if (SDMMC0_DET_MODE == SD_CONTROLLER_DET)
        value = pSDCReg(nSDCPort)->SDMMC_CDETECT;
        if (value & NO_CARD_DETECT)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
        #elif (SDMMC0_DET_MODE == SD_GPIO_DET)
        return SDPAM_IsCardPresence(SDC0);
        #else  //SD_ALWAYS_PRESENT
        return TRUE;
        #endif
    }
    else if(SDC1 == nSDCPort)
    {
        #if (SDMMC1_DET_MODE == SD_CONTROLLER_DET)
        value = pSDCReg(nSDCPort)->SDMMC_CDETECT;
        if (value & NO_CARD_DETECT)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
        #elif (SDMMC1_DET_MODE == SD_GPIO_DET)
        return SDPAM_IsCardPresence(SDC1);
        #else //SD_ALWAYS_PRESENT
        return TRUE;
        #endif
    }
    else
    {

        //eMMC always present
        return TRUE;
    }
}
#if(SD_CARD_Support)
/****************************************************************/
//������:SDC_IsCardWriteProtected
//����:���ĳ�ſ��Ļ�ед���������Ƿ�д������Ч
//����˵��:cardId     �������  ��Ҫ���Ŀ�
//����ֵ: TRUE       ����д����
//        FALSE      ��û��д����
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
bool SDC_IsCardWriteProtected(int32 cardId)
{
#if 0
    volatile uint32 value = 0;
    SDMMC_PORT_E    nSDCPort = (SDMMC_PORT_E)cardId;

    value = pSDCReg(nSDCPort)->SDMMC_WRTPRT;
    if (value & WRITE_PROTECT)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#endif
    return FALSE;
}
#endif
/****************************************************************/
//������:SDC_GetHostBusWidth
//����:�õ�cardId���ڽӿ�Host��֧�ֵ��߿�
//����˵��:cardId   �������  ��Ҫ�����Ŀ�
//         pWidth   �������  �õ����߿�
//����ֵ:
//���ȫ�ֱ���:
//ע��:��ΪIOMUX�Ͳ�ͬ������֧�ֵ�����߿��Ĳ�ͬ������ϲ�����
//     �ڸı��߿�ʱ������ͨ����������жϽӿ�֧�ֵ�����߿�
/****************************************************************/
int32  SDC_GetHostBusWidth(int32 cardId, HOST_BUS_WIDTH_E *pWidth)
{
    SDMMC_PORT_E nSDCPort = (SDMMC_PORT_E)cardId;

    *pWidth = gSDCInfo[nSDCPort].busWidth;
    return SDC_SUCCESS;
}

/****************************************************************/
//������:SDC_SetHostBusWidth
//����:�ı�SDMMC �������������߿���
//     Ӧ���ȷ�����ACMD6�󣬽������øú����ı�������������߿���
//����˵��:cardId   �������  ��Ҫ�����Ŀ�
//         value      �������  ��Ҫд�������߼Ĵ�����ֵ
//����ֵ:
//���ȫ�ֱ���:
//ע��:�������ֻ�ı�SDMMC�������ڲ��������߿������ã����ڿ�����
//     ��Ҫ����ACMD6���������õġ�
//     Ӧ���ȷ�����ACMD6�󣬽������øú����ı�������������߿���
//     ���ݴ�������в�����ı�bus width
/****************************************************************/
int32 SDC_SetHostBusWidth(int32 cardId, HOST_BUS_WIDTH_E width)
{
    SDMMC_PORT_E nSDCPort = (SDMMC_PORT_E)cardId;
    uint32       value = 0;

    switch (width)
    {
        case BUS_WIDTH_1_BIT:
            value = BUS_1_BIT;
            break;
        case BUS_WIDTH_4_BIT:
            value = BUS_4_BIT;
            break;
        case BUS_WIDTH_8_BIT:    //����IC��֧��8bit
            value = BUS_8_BIT;
            break;
        default:
            return SDC_PARAM_ERROR;
    }
    SDPAM_IOMUX_SetSDPort(nSDCPort, width);
    pSDCReg(nSDCPort)->SDMMC_CTYPE = value;

    return SDC_SUCCESS;
}

int  eMMC_Switch_ToMaskRom(void)
{
    uint32       value = 0;
    _Identify_SendCmd(2, (SD_GO_IDLE_STATE | SD_NODATA_OP | SD_RSP_NONE | NO_WAIT_PREV | SEND_INIT), 0xF0F0F0F0, NULL, 0, 0, NULL);
    value = BUS_1_BIT;
    pSDCReg(2)->SDMMC_CTYPE = value;
    pSDCReg(2)->SDMMC_BLKSIZ = 0x200;
    pSDCReg(2)->SDMMC_BYTCNT = 0x200;
    pSDCReg(2)->SDMMC_INTMASK = 0;
    return SDC_SUCCESS;
}

/****************************************************************/
//������:SDC_ResetController
//����:��λcardId���ڵĶ˿�
//����˵��:cardId   �������  ��Ҫ�����Ŀ�
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
int32 SDC_ResetController(int32 cardId)
{
    volatile uint32  value = 0;
    pSDC_REG_T       pReg = NULL;
    SDMMC_PORT_E     nSDCPort = (SDMMC_PORT_E)cardId;
    uint32           ahbFreq = SDPAM_GetAHBFreq();
    int32            timeOut = 0;
    /* reset SDMMC IP */
    SDPAM_SDCClkEnable(nSDCPort, TRUE);
    SDOAM_Delay(10);
    SDPAM_SDCReset(nSDCPort);
    /* reset */
    pReg = pSDCReg(nSDCPort);
    pReg->SDMMC_CTRL = (FIFO_RESET | SDC_RESET);
    pReg->SDMMC_PWREN = (cardId == 2)?1:0; //emmc 1 en, sdcard 0 en
    timeOut = 10000;
    while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET | SDC_RESET)) && (timeOut > 0))
    {
        SDOAM_Delay(1);
        timeOut--;
    }
    if(timeOut == 0)
    {
        SDPAM_SDCClkEnable(nSDCPort, FALSE);
        return SDC_SDC_ERROR;
    }
    /* config FIFO */
    pReg->SDMMC_FIFOTH = (SD_MSIZE_16 | (RX_WMARK << RX_WMARK_SHIFT) | (TX_WMARK << TX_WMARK_SHIFT));
    pReg->SDMMC_CTYPE = BUS_1_BIT;
    pReg->SDMMC_CLKSRC = CLK_DIV_SRC_0;
    /* config debounce */
    pReg->SDMMC_DEBNCE = (DEBOUNCE_TIME*ahbFreq)&0xFFFFFF;
    pReg->SDMMC_TMOUT = 0xFFFFFF40;
    /* config interrupt */
    pReg->SDMMC_RINISTS = 0xFFFFFFFF;
    value = 0;
#if !(EN_SD_DMA)
    value |= (RXDR_INT | TXDR_INT);
#endif
#if EN_SD_INT
    value |= (SBE_INT | FRUN_INT | DTO_INT | CD_INT);
#else
    value |= (FRUN_INT);
#endif
    if(SDC0 == nSDCPort)
    {
        #if (SDMMC0_DET_MODE == SD_CONTROLLER_DET)
        value |= CDT_INT;
        #endif
    }
    else if (SDC1 == nSDCPort)
    {
        #if (SDMMC1_DET_MODE == SD_CONTROLLER_DET)
        value |= CDT_INT;
        #endif
    }

    pReg->SDMMC_INTMASK = value;
    if(nSDCPort == SDC0)
    {
        SDPAM_INTCRegISR(nSDCPort, _SDC0IST);
    }
	else if(nSDCPort == SDC1)
    {
        SDPAM_INTCRegISR(nSDCPort, _SDC1IST);
    }
	else if(nSDCPort == SDC2)
	{
	    //return SDC_FALSE;
	    SDPAM_INTCRegISR(nSDCPort, _SDC2IST);	//eMMC controller	
	}
    SDPAM_INTCEnableIRQ(nSDCPort);
    pReg->SDMMC_CTRL = ENABLE_INT;
    
    return SDC_SUCCESS;
}
#if(0)//SD_CARD_Support)
/****************************************************************/
//������:SDC_EnableSdioInt
//����:ʹ��SDIO�жϣ�����ע��callback����
//����˵��:cardId   �������  ��Ҫ�����Ŀ�
//         cb       �������  �ص�����
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
int32 SDC_EnableSdioInt(int32 cardId, pFunc cb)
{
    volatile uint32  value = 0;
    pSDC_REG_T       pReg = NULL;
    SDMMC_PORT_E     nSDCPort = (SDMMC_PORT_E)cardId;

    gSDCInfo[nSDCPort].pSdioCb = cb;
    gSDCInfo[nSDCPort].bSdioEn = TRUE;
    pReg = pSDCReg(nSDCPort);
    value = pReg->SDMMC_INTMASK;
    value |= SDIO_INT;
    pReg->SDMMC_INTMASK = value;

    return SDC_SUCCESS;
}

/****************************************************************/
//������:SDC_DisableSdioInt
//����:��ֹSDIO�ж�
//����˵��:cardId   �������  ��Ҫ�����Ŀ�
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
int32 SDC_DisableSdioInt(int32 cardId)
{
    volatile uint32  value = 0;
    pSDC_REG_T       pReg = NULL;
    SDMMC_PORT_E     nSDCPort = (SDMMC_PORT_E)cardId;

    gSDCInfo[nSDCPort].pSdioCb = NULL;
    gSDCInfo[nSDCPort].bSdioEn = FALSE;
    pReg = pSDCReg(nSDCPort);
    value = pReg->SDMMC_INTMASK;
    value &= ~(SDIO_INT);
    pReg->SDMMC_INTMASK = value;

    return SDC_SUCCESS;
}
#endif
/****************************************************************/
//������:SDC_UpdateCardFreq
//����:�ı俨�Ĺ���Ƶ�ʣ��������Ժ�ʱ�����Ͽ�ʼ����
//����˵��:freqKHz  �������  ��Ҫ���õ�Ƶ�ʣ���λKHz
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
int32 SDC_UpdateCardFreq(int32 cardId, uint32 freqKHz)
{
    SDMMC_PORT_E nSDCPort = (SDMMC_PORT_E)cardId;
    int32        ret = SDC_SUCCESS;

    //ensure that there are no data or command transfers in progress
    ret = _ChangeFreq(nSDCPort, freqKHz);
    if(ret == SDC_SUCCESS)
    {
        gSDCInfo[nSDCPort].cardFreq = freqKHz;
    }

    return ret;
}

#if 0
/****************************************************************/
//������:SDC_UpdateAhbFreq
//����:AHB��Ƶʱ�����øú�������Ӧ�Ķ�SDMMC���������е�����ʹ��
//     ���Ĺ���Ƶ�ʲ���
//����˵��:newAhbFreq  �������  AHB���߸��º��Ƶ�ʣ���λKHz
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
void SDC_UpdateAhbFreq(uint32 newAhbKHz)
{
    uint32        currentAhbFreq;
    uint32        mmc_clk_div = 0;
    SDMMC_PORT_E  nSDCPort = SDC_MAX;
    SDMMC_PORT_E  nSDCPortMax = SDC_MAX;
    
#if ((SDMMC0_USED == 1) && (SDMMC1_USED == 1))
    nSDCPort = SDC0;
    nSDCPortMax = SDC_MAX; //eMMC �Ӵ˷�֧ 
#elif ((SDMMC0_USED == 1) && (SDMMC1_USED == 0))
    nSDCPort = SDC0;
    nSDCPortMax = SDC1;
#elif ((SDMMC0_USED == 0) && (SDMMC1_USED == 1))
    nSDCPort = SDC1;
    nSDCPortMax = SDC_MAX;
#else
    return;
#endif

    currentAhbFreq = SDPAM_GetAHBFreq();
    if(newAhbKHz == currentAhbFreq)
    {
        return;
    }

    for(; nSDCPort < nSDCPortMax; nSDCPort++)
    {
        if(gSDCInfo[nSDCPort].cardFreq == 0)
        {
            continue;
        }
        if(newAhbKHz < currentAhbFreq)
        {
            gSDCInfo[nSDCPort].updateCardFreq = TRUE;
            continue;
        }
        if(gSDCInfo[nSDCPort].cardFreq <= 400)
        {
            continue;  // 400K����Ƶ��ʱ���Ѿ���֤��Ƶ���ǳ�������˲����ٵ���
        }

        //ʣ�µĴ���400K����ʱSDMMC_CLKDIVΪ0
        mmc_clk_div = newAhbKHz/(gSDCInfo[nSDCPort].cardFreq)
                      + (((newAhbKHz%(gSDCInfo[nSDCPort].cardFreq)) > 0) ? 1:0);
        if(mmc_clk_div > 8)
        {
            mmc_clk_div = 8;
        }
        SDPAM_SetMmcClkDiv(nSDCPort, mmc_clk_div);
    }
}
#endif


/****************************************************************/
//������:SDC_ControlClock
//����:����cardIdָ����cardʱ�ӿ�����ر�
//����˵��:cardId   �������   ��Ҫ�����Ŀ�
//         enable   �������   1:����ʱ�ӣ�0:�ر�ʱ��
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
int32 SDC_ControlClock(int32 cardId, bool enable)
{
    SDMMC_PORT_E nSDCPort = (SDMMC_PORT_E)cardId;
    uint32 clkvalue, clkdiv;

    //ensure that there are no data or command transfers in progress
    if(!enable)
    {
        gSDCInfo[nSDCPort].cardFreq = 0;
        //�ر�ʱ�Ӻ󣬽�SCU��Ƶ����Ϊ��󣬱�֤�´����¶�SDMMC����������ʱcclk_in������52M
        //�ȱ�֤SDMMC����������cclk_in������52MHz������������üĴ������ܷɵ�
        clkvalue = SDPAM_GetAHBFreq();
        clkdiv = clkvalue/MMCHS_52_FPP_FREQ + ( ((clkvalue%MMCHS_52_FPP_FREQ)>0) ? 1: 0 );
        if(clkdiv > 0x3f)
        {
            //PLLƵ��̫��
            return SDC_SDC_ERROR;
        }  
        SDPAM_SetMmcClkDiv(nSDCPort, clkdiv);
    }
    return _ControlClock(nSDCPort, enable);
}

/****************************************************************/
//������:SDC_ControlPower
//����:����cardIdָ����card��Դ������ر�
//����˵��:cardId   �������   ��Ҫ�����Ŀ�
//         enable   �������   1:������Դ��0:�رյ�Դ
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
int32 SDC_ControlPower(int32 cardId, bool enable)
{
    SDMMC_PORT_E nSDCPort = (SDMMC_PORT_E)cardId;
    SDPAM_ControlPower(nSDCPort, enable);
    SDPAM_SDCClkEnable(nSDCPort, enable);
    return SDC_SUCCESS;
}

/****************************************************************/
//������:SDC_WaitCardBusy
//����:�ȴ�ָ���Ŀ�busy���
//����˵��:cardId   �������   ��Ҫ�����Ŀ�
//����ֵ:SDC_PARAM_ERROR      ��������
//       SDC_BUSY_TIMEOUT     �ȴ�ʱ��̫����ʱ�����
//       SDC_SUCCESS          �ɹ�
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
int32 SDC_WaitCardBusy(int32 cardId)
{
    SDMMC_PORT_E nSDCPort = (SDMMC_PORT_E)cardId;

    return _WaitCardBusy(nSDCPort);
}

/****************************************************************/
//������:SDC_BusRequest
//����:��ָ����cardId���������߲���
//����˵��:cardId        �������  ����ָ������Ҫ�����Ŀ�
//         cmd           �������  ��Ҫ���͵�����
//         cmdArg        �������  ����������������Ҫ�������ʱ���������ֵ
//         isAppcmd      �������  ָʾ��ǰ�����Ƿ���application����
//         respType      �������  �ظ�����
//         responseBuf   �������  ��Żظ���buf
//         blockSize     �������  block��С,��λ�ֽ�
//         datalen       �������  ��Ҫ���ջ��͵��ֽڳ��ȣ���λ�ֽ�
//         dataBuf       ������������  ��Ž��յ������ݻ���Ҫ���ͳ�ȥ������
//����ֵ:SDC_PARAM_ERROR
//       SDC_BUSY_TIMEOUT
//       SDC_RESP_TIMEOUT
//       SDC_DATA_CRC_ERROR
//       SDC_END_BIT_ERROR
//       SDC_START_BIT_ERROR
//       SDC_DATA_READ_TIMEOUT
//       SDC_RESP_CRC_ERROR
//       SDC_RESP_ERROR
//       SDC_SUCCESS
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
int32 SDC_BusRequest(int32 cardId,
                             uint32 cmd,
                             uint32 cmdArg,
                             uint32 *responseBuf,
                             uint32  blockSize,
                             uint32  dataLen,
                             void   *pDataBuf)
{
    volatile uint32 value = 0;
    SDMMC_PORT_E    nSDCPort = (SDMMC_PORT_E)cardId;
    pSDC_REG_T      pReg = pSDCReg(nSDCPort);
    int32           ret = SDC_SUCCESS;
    int32           timeOut = 0;
    pFunc           cb;

//    uint8   *tempbuf;
    //eMMC_printk(4, "SDC_BusRequest 111%s  %d\n", __FILE__,__LINE__);
    
    //eMMC_printk(3, "SDC_BusRequest:  CMD=%d \n",cmd&0x3f);
    //ensure the card is not busy due to any previous data transfer command
    if (!(cmd & STOP_CMD))
    {
        //���FIFO
        value = pReg->SDMMC_STATUS;
        if (!(value & FIFO_EMPTY))
        {
            value = pReg->SDMMC_CTRL;
            value |= FIFO_RESET;
            pReg->SDMMC_CTRL = value;
            timeOut = 100000;
            while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET)) && (timeOut > 0))
            {
                SDOAM_Delay(1);
                timeOut--;
            }
            if(timeOut == 0)
            {
                eMMC_printk(3, "SDC_BusRequest:  CMD=%d SDC_SDC_ERROR  %d\n",cmd&0x3f,__LINE__);
                return SDC_SDC_ERROR;
            }
        }
    }
    if (cmd & WAIT_PREV)
    {
        value = pReg->SDMMC_STATUS;
        if (value & DATA_BUSY)
        {
            eMMC_printk(3, "SDC_BusRequest:  CMD=%d DATA BUSY  %d\n",cmd&0x3f,__LINE__);
            return SDC_BUSY_TIMEOUT;
        }
    }
    Assert(!(((value = pReg->SDMMC_STATUS) & DATA_BUSY) && (cmd & WAIT_PREV)), "SDC_BusRequest:busy error\n", value);
    Assert(!(((value = pReg->SDMMC_STATUS) & 0x3FFE0000) && (!(cmd & STOP_CMD))), "SDC_BusRequest:+FIFO not empty\n", value);
    Assert(!((value = pReg->SDMMC_CMD) & START_CMD), "SDC_BusRequest:start bit != 0\n", value);
    value = pReg->SDMMC_CMD;
    if(value & START_CMD)
    {
        eMMC_printk(3, "SDC_BusRequest:  CMD=%d START CMD ERROR %d\n",cmd&0x3f,__LINE__);
        return SDC_SDC_ERROR;
    }

    if(gSDCInfo[nSDCPort].updateCardFreq)
    {
        ret = _ChangeFreq(nSDCPort, gSDCInfo[nSDCPort].cardFreq);
        if (ret != SDC_SUCCESS)
        {
            eMMC_printk(3, "SDC_BusRequest:  CMD=%d ChangeFreq ERROR %d\n",cmd&0x3f,__LINE__);
            return ret;
        }
        gSDCInfo[nSDCPort].updateCardFreq = FALSE;
    }
    //eMMC_printk(2, "SDC_BusRequest 333%s  %d\n", __FILE__,__LINE__);
    if (cmd & DATA_EXPECT)
    {       
        Assert((dataLen > 0),"SDC_BusRequest:dataLen = 0\n", dataLen);
        if (dataLen == 0)
        {
            return SDC_PARAM_ERROR;
        }
        if((cmd & SD_OP_MASK) == SD_WRITE_OP)
        {
            if(nSDCPort == SDC0)
            {
                cb   = _SDC0DMACallback;
            }
            else if(nSDCPort == SDC1)
            {
                cb   = _SDC1DMACallback;
            }
            else
            {
                 cb   = _SDC2DMACallback;
            }
            
        	//д֮ǰclean cache
        	SDPAM_CleanCache(pDataBuf, dataLen);
            _PrepareForWriteData(nSDCPort, pDataBuf, dataLen, cb);
        }
        else
        {
            if(nSDCPort == SDC0)
            {
                cb   = _SDC0DMACallback;
            }
            else if(nSDCPort == SDC1)
            {
                cb   = _SDC1DMACallback;
            }
            else
            {
                 cb   = _SDC2DMACallback;
            }
        
            _PrepareForReadData(nSDCPort, pDataBuf, dataLen, cb);
        }
        pReg->SDMMC_BLKSIZ = blockSize;
        pReg->SDMMC_BYTCNT = dataLen;  //����Ĵ����ĳ���һ��Ҫ����Ϊ��Ҫ�ĳ��ȣ����ÿ���SDMMC��������32bit����
    }
    //eMMC_printk(3, "SDC_BusRequest 444  %s  %d\n", __FILE__,__LINE__);
    pReg->SDMMC_CMDARG = cmdArg;
    //set start bit,CMD/CMDARG/BYTCNT/BLKSIZ/CLKDIV/CLKENA/CLKSRC/TMOUT/CTYPE were locked
    SDC_Start(pReg, ((cmd & (~(RSP_BUSY))) | START_CMD | USE_HOLD_REG));
    //wait until current start clear
    timeOut = 10000;
    while (((value = pReg->SDMMC_CMD) & START_CMD) && (timeOut > 0))
    {
        SDOAM_Delay(1);
        timeOut--;
    }
    if(timeOut == 0)
    {        
        eMMC_printk(3, "SDC_BusRequest:  CMD=%d  START_CMD timeout %d\n",cmd&0x3f,__LINE__);
        return SDC_SDC_ERROR;
    }
    #if EN_SD_INT
    SDOAM_GetEvent(gSDCInfo[nSDCPort].event, CD_EVENT);
    #else
    
    //eMMC_printk(5, "SDC_BusRequest 444-2  ------  %s  %d\n", __FILE__,__LINE__);
    timeOut = 250000; // 250ms
    while (!((value = pReg->SDMMC_RINISTS) & CD_INT) && (timeOut > 0))
    {
        SDOAM_Delay(1);
        timeOut--;
    }
    if(timeOut == 0)
    {        
        eMMC_printk(5, "SDC_BusRequest 444-1  ---START_CMD timeout---  %s  %d\n", __FILE__,__LINE__);
        return SDC_SDC_ERROR;
    }
    pReg->SDMMC_RINISTS = CD_INT;
    #endif
    if (cmd & STOP_CMD)
    {
        //����STOP������ܻ��������ݽ���(��mutiple readʱ)��Ҫ�����FIFO
        value = pReg->SDMMC_STATUS;
        if (!(value & FIFO_EMPTY))
        {
            value = pReg->SDMMC_CTRL;
            value |= FIFO_RESET;
            pReg->SDMMC_CTRL = value;
            timeOut = 1000;
            while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET)) && (timeOut > 0))
            {
                SDOAM_Delay(1);
                timeOut--;
            }
            if(timeOut == 0)
            {
                return SDC_SDC_ERROR;
            }
        }
    }
    //eMMC_printk(3, "SDC_BusRequest 555  %s  %d\n", __FILE__,__LINE__);
    //check response error, or response crc error, or response timeout
    value = pReg->SDMMC_RINISTS;
    if (value & RTO_INT)
    {       
        //eMMC_printk(3, "SDC_BusRequest 555--1  %s  %d\n", __FILE__,__LINE__);
        pReg->SDMMC_RINISTS = 0xFFFFFFFF;  //���response timeout�����ݴ��� ����ֹ����������ж�
#if EN_SD_DMA
        value = pReg->SDMMC_CTRL;
        if (value & ENABLE_DMA)
        {
            if((cmd & SD_OP_MASK) == SD_WRITE_OP)
            {
                SDPAM_DMAStop(nSDCPort, 1);
            }
            else
            {
                SDPAM_DMAStop(nSDCPort, 0);
            }
            value &= ~(ENABLE_DMA);
            value |= FIFO_RESET;
            pReg->SDMMC_CTRL = value;
            timeOut = 1000;
            while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET)) && (timeOut > 0))
            {
                SDOAM_Delay(1);
                timeOut--;
            }
            if(timeOut == 0)
            {
                return SDC_SDC_ERROR;
            }
        }
#endif
        eMMC_printk(3, "SDC_BusRequest:  CMD=%d  SDC_RESP_TIMEOUT %d\n",cmd&0x3f,__LINE__);
        return SDC_RESP_TIMEOUT;
    }

    if (cmd & DATA_EXPECT)
    {        
        #if EN_SD_INT
        SDOAM_GetEvent(gSDCInfo[nSDCPort].event, DTO_EVENT);
        #else
       // uint8 *tempbuf = (uint8 *)pDataBuf;
        timeOut = 100 * dataLen + 250000; // 1ms read 20 sector, 1 sector timeout set 50ms + 250ms
        while (!((value = pReg->SDMMC_RINISTS) & (DTO_INT | SBE_INT)) &&  timeOut)
        {
            SDOAM_Delay(1);
            timeOut--;
        }
        if(timeOut == 0)
        {
            eMMC_printk(5, "SDC_BusRequest 444-1  ---wait for data timeout---  %s  %d\n", __FILE__,__LINE__);
            return SDC_SDC_ERROR;
        }
        pReg->SDMMC_RINISTS = DTO_INT;  
        #endif
        if((cmd & SD_OP_MASK) == SD_WRITE_OP)
        {
#if EN_SD_DMA
            value = pReg->SDMMC_CTRL;
            if (value & ENABLE_DMA)
            {
                SDPAM_DMAStop(nSDCPort, 1);
                value &= ~(ENABLE_DMA);
                pReg->SDMMC_CTRL = value;
            }
#endif
            value = pReg->SDMMC_RINISTS;
            if (value & DCRC_INT)
            {
                ret = SDC_DATA_CRC_ERROR;
            }
            else if (value & EBE_INT)
            {
                ret = SDC_END_BIT_ERROR;
            }
            else
            {
                ret = _WaitCardBusy(nSDCPort);
                if (ret != SDC_SUCCESS)
                {
                    return ret;
                }
                ret = SDC_SUCCESS;
            }
        }
        else
        {
#if EN_SD_DMA
            value = pReg->SDMMC_CTRL;
            if (value & ENABLE_DMA)
            {
                SDPAM_DMAStop(nSDCPort, 0);
                value &= ~(ENABLE_DMA);
                pReg->SDMMC_CTRL = value;
            }
#endif
        	//����ɺ�Ҫflush cache
        	SDPAM_FlushCache(pDataBuf, dataLen);
    
            value = pReg->SDMMC_RINISTS;
            //Assert(!((value & SBE_INT) && (!(value & DRTO_INT))), "SDC_BusRequest:start bit error but not timeout\n", value);
            if(value & (SBE_INT | EBE_INT | DRTO_INT | DCRC_INT))
            {
                if (value & SBE_INT)
                {
                    uint32 stopCmd = 0;
                	stopCmd = (SD_STOP_TRANSMISSION | SD_NODATA_OP | SD_RSP_R1 | STOP_CMD | NO_WAIT_PREV);
                    SDC_Start(pReg, ((stopCmd & (~(RSP_BUSY))) | START_CMD));
                    timeOut = 10000;
                    while (((value = pReg->SDMMC_CMD) & START_CMD) && (timeOut > 0))
                    {
                        SDOAM_Delay(1);
                        timeOut--;
                    }
                    if(timeOut == 0)
                    {
                        eMMC_printk(3, "SDC_BusRequest:  CMD=%d START_CMD ERROR %d\n",cmd&0x3f,__LINE__);
                        return SDC_SDC_ERROR;
                    }
                    #if EN_SD_INT
                    SDOAM_GetEvent(gSDCInfo[nSDCPort].event, CD_EVENT);
                    SDOAM_GetEvent(gSDCInfo[nSDCPort].event, DTO_EVENT);
                    #else
                    while (((value = pReg->SDMMC_RINISTS) & (CD_INT | DTO_INT)) != (CD_INT | DTO_INT))
                    {
                        SDOAM_Delay(1);
                    }
                    pReg->SDMMC_RINISTS = (CD_INT | DTO_INT);
                    #endif
                    pReg->SDMMC_RINISTS = SBE_INT;
                    value = 0;
                    #if !(EN_SD_DMA)
                    value |= (RXDR_INT | TXDR_INT);
                    #endif
                    #if EN_SD_INT
                    value |= (SBE_INT | FRUN_INT | DTO_INT | CD_INT);
                    #else
                    value |= (FRUN_INT);
                    #endif
                    if(SDC0 == nSDCPort)
                    {
                        #if (SDMMC0_DET_MODE == SD_CONTROLLER_DET)
                        value |= CDT_INT;
                        #endif
                    }
                    else if(SDC1 == nSDCPort)
                    {
                        #if (SDMMC1_DET_MODE == SD_CONTROLLER_DET)
                        value |= CDT_INT;
                        #endif
                    }
                    else
                    {
                        //eMMC no detect
                    }
                    
                    pReg->SDMMC_INTMASK = value;
                    ret = SDC_START_BIT_ERROR;
                }
                else if (value & EBE_INT)
                {
                	if((cmd & SD_CMD_MASK) == SD_CMD14)
                    {
                        ret = _ReadRemainData(nSDCPort, dataLen, pDataBuf);
                    }
                    else
                    {
                        ret = SDC_END_BIT_ERROR;
                    }
                }
                else if (value & DRTO_INT)
                {
                    ret = SDC_DATA_READ_TIMEOUT;
                }
                else if (value & DCRC_INT) 
                {
                    ret = SDC_DATA_CRC_ERROR;
                }
            }
            else
            {
                ret = _ReadRemainData(nSDCPort, dataLen, pDataBuf);
            }

            
            //eMMC_printk(3, "SDC_BusRequest 777 %s  %d\n", __FILE__,__LINE__);

#if !(EN_SD_DMA)
            Assert(!((gSDCInfo[nSDCPort].intInfo.transLen != gSDCInfo[nSDCPort].intInfo.desLen) && (ret == SDC_SUCCESS)), "SDC_BusRequest:translen != deslen\n", gSDCInfo[nSDCPort].intInfo.transLen);
            gSDCInfo[nSDCPort].intInfo.transLen = 0;
            gSDCInfo[nSDCPort].intInfo.desLen   = 0;
            gSDCInfo[nSDCPort].intInfo.pBuf     = NULL;
#endif
            //eMMC_printk(3, "%s...%d....SDMMC_STATUS=%x, ret=%d \n", __FILE__,__LINE__, value = pReg->SDMMC_STATUS, ret);
            Assert(!(((value = pReg->SDMMC_STATUS) & 0x3FFE0000) && (ret == SDC_SUCCESS)), "SDC_BusRequest:-FIFO not empty\n", value);
            //eMMC_printk(3, "SDC_BusRequest 777--1  %s  %d\n", __FILE__,__LINE__);
        }
    }
    value = pReg->SDMMC_RINISTS;
    pReg->SDMMC_RINISTS = 0xFFFFFFFF;

    //eMMC_printk(3, "SDC_BusRequest 888 %s  %d\n", __FILE__,__LINE__);

    if (ret == SDC_SUCCESS)
    {
        if (cmd & RSP_BUSY) //R1b
        {
            ret = _WaitCardBusy(nSDCPort);
            if (ret != SDC_SUCCESS)
            {
                eMMC_printk(3, "SDC_BusRequest:  CMD=%d WaitCard Ready Timeout%d\n",cmd&0x3f,__LINE__);
                return ret;
            }
        }

        //if need, get response
        if ((cmd & R_EXPECT) && (responseBuf != NULL))
        {
            if (cmd & LONG_R)
            {
                responseBuf[0] = pReg->SDMMC_RESP0;
                responseBuf[1] = pReg->SDMMC_RESP1;
                responseBuf[2] = pReg->SDMMC_RESP2;
                responseBuf[3] = pReg->SDMMC_RESP3;
            }
            else
            {
                responseBuf[0] = pReg->SDMMC_RESP0;
            }
        }

        if(gSDCInfo[nSDCPort].cardFreq > 400)
        {
            if (value & RCRC_INT)
            {
                return SDC_RESP_CRC_ERROR;
            }
            if (value & RE_INT)
            {
                return SDC_RESP_ERROR;
            }
        }
    }
    //eMMC_printk(3, "SDC_BusRequest Success.  %s  %d\n", __FILE__,__LINE__);

    return ret;
}

#endif  //end of #ifdef DRIVERS_SDMMC
