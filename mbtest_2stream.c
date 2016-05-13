#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "wdc_defs.h"
#include "wdc_lib.h"
#include "utils.h"
#include "status_strings.h"
#include "samples/shared/diag_lib.h"
#include "samples/shared/wdc_diag_lib.h"
#include "samples/shared/pci_regs.h"
#include "pcie_lib.h"

/*************************************************************
  General definitions
*************************************************************/
/* Error messages display */
#define PCIE_ERR printf

/*************************************************************
  Global variables
*************************************************************/
/* User's input command */
static CHAR gsInput[256];

/* --------------------------------------------------
   PCIE configuration registers information
   -------------------------------------------------- */
/* Configuration registers information array */
const WDC_REG gPCIE_CfgRegs[] = {
  { WDC_AD_CFG_SPACE, PCI_VID, WDC_SIZE_16, WDC_READ_WRITE, "VID", "Vendor ID" },
  { WDC_AD_CFG_SPACE, PCI_DID, WDC_SIZE_16, WDC_READ_WRITE, "DID", "Device ID" },
  { WDC_AD_CFG_SPACE, PCI_CR, WDC_SIZE_16, WDC_READ_WRITE, "CMD", "Command" },
  { WDC_AD_CFG_SPACE, PCI_SR, WDC_SIZE_16, WDC_READ_WRITE, "STS", "Status" },
  { WDC_AD_CFG_SPACE, PCI_REV, WDC_SIZE_32, WDC_READ_WRITE, "RID_CLCD", "Revision ID & Class Code" },
  { WDC_AD_CFG_SPACE, PCI_CCSC, WDC_SIZE_8, WDC_READ_WRITE, "SCC", "Sub Class Code" },
  { WDC_AD_CFG_SPACE, PCI_CCBC, WDC_SIZE_8, WDC_READ_WRITE, "BCC", "Base Class Code" },
  { WDC_AD_CFG_SPACE, PCI_CLSR, WDC_SIZE_8, WDC_READ_WRITE, "CALN", "Cache Line Size" },
  { WDC_AD_CFG_SPACE, PCI_LTR, WDC_SIZE_8, WDC_READ_WRITE, "LAT", "Latency Timer" },
  { WDC_AD_CFG_SPACE, PCI_HDR, WDC_SIZE_8, WDC_READ_WRITE, "HDR", "Header Type" },
  { WDC_AD_CFG_SPACE, PCI_BISTR, WDC_SIZE_8, WDC_READ_WRITE, "BIST", "Built-in Self Test" },
  { WDC_AD_CFG_SPACE, PCI_BAR0, WDC_SIZE_32, WDC_READ_WRITE, "BADDR0", "Base Address 0" },
  { WDC_AD_CFG_SPACE, PCI_BAR1, WDC_SIZE_32, WDC_READ_WRITE, "BADDR1", "Base Address 1" },
  { WDC_AD_CFG_SPACE, PCI_BAR2, WDC_SIZE_32, WDC_READ_WRITE, "BADDR2", "Base Address 2" },
  { WDC_AD_CFG_SPACE, PCI_BAR3, WDC_SIZE_32, WDC_READ_WRITE, "BADDR3", "Base Address 3" },
  { WDC_AD_CFG_SPACE, PCI_BAR4, WDC_SIZE_32, WDC_READ_WRITE, "BADDR4", "Base Address 4" },
  { WDC_AD_CFG_SPACE, PCI_BAR5, WDC_SIZE_32, WDC_READ_WRITE, "BADDR5", "Base Address 5" },
  { WDC_AD_CFG_SPACE, PCI_CIS, WDC_SIZE_32, WDC_READ_WRITE, "CIS", "CardBus CIS Pointer" },
  { WDC_AD_CFG_SPACE, PCI_SVID, WDC_SIZE_16, WDC_READ_WRITE, "SVID", "Sub-system Vendor ID" },
  { WDC_AD_CFG_SPACE, PCI_SDID, WDC_SIZE_16, WDC_READ_WRITE, "SDID", "Sub-system Device ID" },
  { WDC_AD_CFG_SPACE, PCI_EROM, WDC_SIZE_32, WDC_READ_WRITE, "EROM", "Expansion ROM Base Address" },
  { WDC_AD_CFG_SPACE, PCI_CAP, WDC_SIZE_8, WDC_READ_WRITE, "NEW_CAP", "New Capabilities Pointer" },
  { WDC_AD_CFG_SPACE, PCI_ILR, WDC_SIZE_32, WDC_READ_WRITE, "INTLN", "Interrupt Line" },
  { WDC_AD_CFG_SPACE, PCI_IPR, WDC_SIZE_32, WDC_READ_WRITE, "INTPIN", "Interrupt Pin" },
  { WDC_AD_CFG_SPACE, PCI_MGR, WDC_SIZE_32, WDC_READ_WRITE, "MINGNT", "Minimum Required Burst Period" },
  { WDC_AD_CFG_SPACE, PCI_MLR, WDC_SIZE_32, WDC_READ_WRITE, "MAXLAT", "Maximum Latency" },
};
#define PCIE_CFG_REGS_NUM sizeof(gPCIE_CfgRegs) / sizeof(WDC_REG)
/* TODO: For read-only or write-only registers, change the direction field of
   the relevant registers in gPCIE_CfgRegs to WDC_READ or WDC_WRITE. */
/* NOTE: You can define additional configuration registers in gPCIE_CfgRegs. */
const WDC_REG *gpPCIE_CfgRegs = gPCIE_CfgRegs;

/* -----------------------------------------------
   PCIE run-time registers information
   ----------------------------------------------- */
/* Run-time registers information array */
/* const WDC_REG gPCIE_Regs[]; */
const WDC_REG *gpPCIE_Regs = NULL;
/* TODO: You can remove the comment from the gPCIE_Regs array declaration and
   fill the array with run-time registers information for your device,
   in which case be sure to set gpPCIE_Regs to point to gPCIE_Regs. */
#define PCIE_REGS_NUM 0

/*************************************************************
  Static functions prototypes
*************************************************************/
/* -----------------------------------------------
   Main diagnostics menu
   ----------------------------------------------- */
static void MenuMain(WDC_DEVICE_HANDLE *phDev, WDC_DEVICE_HANDLE *phDev1, WDC_DEVICE_HANDLE *phDev2, 
		     WDC_DEVICE_HANDLE *phDev3, WDC_DEVICE_HANDLE *phDev4, WDC_DEVICE_HANDLE *phDev5,int min__, int max__);

/* static void MenuMain(WDC_DEVICE_HANDLE *phDev, WDC_DEVICE_HANDLE *phDev1, WDC_DEVICE_HANDLE *phDev2,  */
/* WDC_DEVICE_HANDLE *phDev3, WDC_DEVICE_HANDLE *phDev4, WDC_DEVICE_HANDLE *phDev5); */

/* -----------------------------------------------
   Device find, open and close
   ----------------------------------------------- */
static WDC_DEVICE_HANDLE DeviceFindAndOpen(DWORD dwVendorId, DWORD dwDeviceId);
static BOOL DeviceFind(DWORD dwVendorId, DWORD dwDeviceId, WD_PCI_SLOT *pSlot);
static WDC_DEVICE_HANDLE DeviceOpen(const WD_PCI_SLOT *pSlot);
static void DeviceClose(WDC_DEVICE_HANDLE hDev);
static void MenuMBtest(WDC_DEVICE_HANDLE hDev, WDC_DEVICE_HANDLE hDev1 ,WDC_DEVICE_HANDLE hDev2, 
		       WDC_DEVICE_HANDLE hDev3, WDC_DEVICE_HANDLE hDev4, WDC_DEVICE_HANDLE hDev5,int min__, int max__);

static int pcie_send(WDC_DEVICE_HANDLE hDev, int mode, int nword, UINT32 *buff_send);
static int pcie_rec(WDC_DEVICE_HANDLE hDev, int mode, int istart, int nword, int ipr_status, UINT32 *buff_rec);

static int tpc_adc_setup(WDC_DEVICE_HANDLE hDev, int imod_fem, int iframe, int itpc_adc, int ihuff, int icom_factor, int timesize);

static void *pt_sn_copythread(void* arg);
static void *pt_trig_copythread(void* arg);

static void *pt_sn_filewrite(void *nword_write);
static void *pt_trig_filewrite(void *nword_write);

void *pt_trig_dma(void *threadarg);
void *pt_sn_dma(void *threadarg);


static int check_status(WDC_DEVICE_HANDLE hDevPMT,
			WDC_DEVICE_HANDLE hDevTPC,
			int imod_trig,
			int imod_fem_pmt_start,
			int imod_fem_pmt_end,
			int imod_fem_tpc_start,
			int imod_fem_tpc_end,
			int imod_xmit_pmt,
			int imod_xmit_tpc);


//
//     data storage
//
static int buff_snova[4000000];
static int buff_trig [4000000];
static int snova_pointer;
static int trig_pointer;
static int snova_wcount;
static int trig_wcount;
static int fd_trig_pt;
static int fd_sn_pt;
static int fd_trig_pt_m;
static int fd_monitor_pt;
//
static int fd_trig_pt_tpc;
static int fd_sn_pt_tpc;
static int fd_trig_pt_m_tpc;
static int fd_monitor_pt_tpc;
//
static DWORD dwDMABufSize;
static int itrig_m_d, itrig_m, ith_fr;
static int VDEBUG;
//vic
static int sn_buf_filled[4];
static int neu_buf_filled[2];
static int total_used_s,total_used_n;
static long mytime1, seconds1, useconds1;
struct timeval starttest1, endtest1;
//vic

#define  jbuf_ev_size 1000000

pthread_mutex_t mutexlock;
static int write_point_n, read_point_n, write_point_s, read_point_s;
static int write_point_n_tpc, read_point_n_tpc, write_point_s_tpc, read_point_s_tpc;
//static int buffer_wc_n[jbuf_ev_size], buffer_wc_s[jbuf_ev_size];
static int buffer_ev_n[jbuf_ev_size], buffer_ev_s[jbuf_ev_size];
static int buffer_ev_n_tpc[jbuf_ev_size], buffer_ev_s_tpc[jbuf_ev_size];

//
//  for PMT
//
PVOID pbuf_rec_n1;
WD_DMA *pDma_rec_n1;

PVOID pbuf_rec_n2;
WD_DMA *pDma_rec_n2;


PVOID pbuf_rec_s;
WD_DMA *pDma_rec_s;
PVOID pbuf_rec_s1;
WD_DMA *pDma_rec_s1;
PVOID pbuf_rec_s2;
WD_DMA *pDma_rec_s2;
PVOID pbuf_rec_s3;
WD_DMA *pDma_rec_s3;
PVOID pbuf_rec_s4;
WD_DMA *pDma_rec_s4;
/*************************************************************
  Functions implementation
*************************************************************/
/* int main(void) */
int main(int argc, char **argv)
{

  int min__ = atoi(argv[1]);
  int max__ = atoi(argv[2]);

  struct timeval start;
  gettimeofday(&start,NULL);

  long seconds, useconds;
  seconds = start.tv_sec;
  useconds = start.tv_usec;

  printf("\nStart time of program: %ld sec %ld usec\n",seconds,useconds);


  WDC_DEVICE_HANDLE hDev = NULL;
  WDC_DEVICE_HANDLE hDev1 = NULL;
  WDC_DEVICE_HANDLE hDev2 = NULL;
  WDC_DEVICE_HANDLE hDev3 = NULL;
  WDC_DEVICE_HANDLE hDev4 = NULL;
  WDC_DEVICE_HANDLE hDev5 = NULL;

  DWORD dwStatus;

  printf("\n");
  printf("PCIE diagnostic utility.\n");
  printf("Application accesses hardware using " WD_PROD_NAME ".\n");

  /* Initialize the PCIE library */
  dwStatus = PCIE_LibInit();
  if (WD_STATUS_SUCCESS != dwStatus)
    {
      PCIE_ERR("pcie_diag: Failed to initialize the PCIE library: %s",
	       PCIE_GetLastErr());
      return dwStatus;
    }

  /* Find and open a PCIE device (by default ID) */
  if (PCIE_DEFAULT_VENDOR_ID)
    hDev = DeviceFindAndOpen(PCIE_DEFAULT_VENDOR_ID, PCIE_DEFAULT_DEVICE_ID);
  if (PCIE_DEFAULT_VENDOR_ID)
    hDev1 = DeviceFindAndOpen(PCIE_DEFAULT_VENDOR_ID, PCIE_DEFAULT_DEVICE_ID+1);
  if (PCIE_DEFAULT_VENDOR_ID)
    hDev2 = DeviceFindAndOpen(PCIE_DEFAULT_VENDOR_ID, PCIE_DEFAULT_DEVICE_ID+2);
  if (PCIE_DEFAULT_VENDOR_ID)
    hDev3 = DeviceFindAndOpen(PCIE_DEFAULT_VENDOR_ID, PCIE_DEFAULT_DEVICE_ID+3);
  if (PCIE_DEFAULT_VENDOR_ID)
    hDev4 = DeviceFindAndOpen(PCIE_DEFAULT_VENDOR_ID, PCIE_DEFAULT_DEVICE_ID+4);
  if (PCIE_DEFAULT_VENDOR_ID)
    hDev5 = DeviceFindAndOpen(PCIE_DEFAULT_VENDOR_ID, PCIE_DEFAULT_DEVICE_ID+5);
    
    
  hDev3  = hDev;  // controller
  hDev4  = hDev2; // Nu Card
  hDev5  = hDev1; // SN card


  /* Display main diagnostics menu for communicating with the device */
  /* MenuMain(&hDev, &hDev1, &hDev2, &hDev3, &hDev4, &hDev5); */
  MenuMain(&hDev, &hDev1, &hDev2, &hDev3, &hDev4, &hDev5,min__,max__);


  /* Perform necessary cleanup before exiting the program */
  if (hDev)
    DeviceClose(hDev);
  DeviceClose(hDev1);
  DeviceClose(hDev2);
  DeviceClose(hDev3);
  DeviceClose(hDev4);
  DeviceClose(hDev5);

  dwStatus = PCIE_LibUninit();
  if (WD_STATUS_SUCCESS != dwStatus)
    PCIE_ERR("pcie_diag: Failed to uninit the PCIE library: %s", PCIE_GetLastErr());
    
  return dwStatus;
}

/* -----------------------------------------------
   Main diagnostics menu
   ----------------------------------------------- */
/* Main menu options */
enum {
  MENU_MAIN_SCAN_PCI_BUS = 1,
  MENU_MAIN_FIND_AND_OPEN,
  MENU_MAIN_RW_ADDR,
  MENU_MAIN_RW_CFG_SPACE,
  MENU_MAIN_RW_REGS,
  MENU_MAIN_ENABLE_DISABLE_INT,
  MENU_MAIN_EVENTS,
  MENU_MAIN_MB_TEST, /* add new route for testing */
  MENU_MAIN_JSEBII_TEST, /* add new route for testing */
  MENU_MAIN_EXIT = DIAG_EXIT_MENU,
};

/* Main diagnostics menu */
static void MenuMain(WDC_DEVICE_HANDLE *phDev, WDC_DEVICE_HANDLE *phDev1, WDC_DEVICE_HANDLE *phDev2, 
		     WDC_DEVICE_HANDLE *phDev3, WDC_DEVICE_HANDLE *phDev4, WDC_DEVICE_HANDLE *phDev5,int min__,int max__)
  
/* static void MenuMain(WDC_DEVICE_HANDLE *phDev, WDC_DEVICE_HANDLE *phDev1, WDC_DEVICE_HANDLE *phDev2,  */
/* 		     WDC_DEVICE_HANDLE *phDev3, WDC_DEVICE_HANDLE *phDev4, WDC_DEVICE_HANDLE *phDev5) */
{

  MenuMBtest(*phDev, *phDev1, *phDev2, *phDev3, *phDev4, *phDev5,min__,max__);
  return;
}

/* -----------------------------------------------
   Device find, open and close
   ----------------------------------------------- */
/* Find and open a PCIE device */
static WDC_DEVICE_HANDLE DeviceFindAndOpen(DWORD dwVendorId, DWORD dwDeviceId)
{
  WD_PCI_SLOT slot;
    
  if (!DeviceFind(dwVendorId, dwDeviceId, &slot))
    return NULL;

  return DeviceOpen(&slot);
}

/* Find a PCIE device */
static BOOL DeviceFind(DWORD dwVendorId, DWORD dwDeviceId, WD_PCI_SLOT *pSlot)
{
  DWORD dwStatus;
  DWORD i, dwNumDevices;
  WDC_PCI_SCAN_RESULT scanResult;

  if (dwVendorId == 0)
    {
      if (DIAG_INPUT_SUCCESS != DIAG_InputDWORD((PVOID)&dwVendorId,
						"Enter vendor ID", TRUE, 0, 0))
        {
	  return FALSE;
        }

      if (DIAG_INPUT_SUCCESS != DIAG_InputDWORD((PVOID)&dwDeviceId,
						"Enter device ID", TRUE, 0, 0))
        {
	  return FALSE;
        }
    }

  BZERO(scanResult);
  dwStatus = WDC_PciScanDevices(dwVendorId, dwDeviceId, &scanResult);
  if (WD_STATUS_SUCCESS != dwStatus)
    {
      PCIE_ERR("DeviceFind: Failed scanning the PCI bus.\n"
	       "Error: 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
      return FALSE;
    }

  dwNumDevices = scanResult.dwNumDevices;
  if (!dwNumDevices)
    {
      PCIE_ERR("No matching device was found for search criteria "
	       "(Vendor ID 0x%lX, Device ID 0x%lX)\n",
	       dwVendorId, dwDeviceId);

      return FALSE;
    }

  printf("\n");
  printf("Found %ld matching device%s [ Vendor ID 0x%lX%s, Device ID 0x%lX%s ]:\n",
	 dwNumDevices, dwNumDevices > 1 ? "s" : "",
	 dwVendorId, dwVendorId ? "" : " (ALL)",
	 dwDeviceId, dwDeviceId ? "" : " (ALL)");

  for (i = 0; i < dwNumDevices; i++)
    {
      printf("\n");
      printf("%2ld. Vendor ID: 0x%lX, Device ID: 0x%lX\n",
	     i + 1,
	     scanResult.deviceId[i].dwVendorId,
	     scanResult.deviceId[i].dwDeviceId);

      WDC_DIAG_PciDeviceInfoPrint(&scanResult.deviceSlot[i], FALSE);
    }
  printf("\n");

  if (dwNumDevices > 1)
    {
      sprintf(gsInput, "Select a device (1 - %ld): ", dwNumDevices);
      i = 0;
      if (DIAG_INPUT_SUCCESS != DIAG_InputDWORD((PVOID)&i,
						gsInput, FALSE, 1, dwNumDevices))
        {
	  return FALSE;
        }
    }

  *pSlot = scanResult.deviceSlot[i - 1];

  return TRUE;
}

/* Open a handle to a PCIE device */
static WDC_DEVICE_HANDLE DeviceOpen(const WD_PCI_SLOT *pSlot)
{
  WDC_DEVICE_HANDLE hDev;
  DWORD dwStatus;
  WD_PCI_CARD_INFO deviceInfo;

  /* Retrieve the device's resources information */
  BZERO(deviceInfo);
  deviceInfo.pciSlot = *pSlot;
  dwStatus = WDC_PciGetDeviceInfo(&deviceInfo);
  if (WD_STATUS_SUCCESS != dwStatus)
    {
      PCIE_ERR("DeviceOpen: Failed retrieving the device's resources information.\n"
	       "Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
      return NULL;
    }

  /* NOTE: You can modify the device's resources information here, if
     necessary (mainly the deviceInfo.Card.Items array or the items number -
     deviceInfo.Card.dwItems) in order to register only some of the resources
     or register only a portion of a specific address space, for example. */

  /* Open a handle to the device */
  hDev = PCIE_DeviceOpen(&deviceInfo);
  if (!hDev)
    {
      PCIE_ERR("DeviceOpen: Failed opening a handle to the device: %s",
	       PCIE_GetLastErr());
      return NULL;
    }

  return hDev;
}

/* Close handle to a PCIE device */
static void DeviceClose(WDC_DEVICE_HANDLE hDev)
{
  if (!hDev)
    return;

  if (!PCIE_DeviceClose(hDev))
    {
      PCIE_ERR("DeviceClose: Failed closing PCIE device: %s",
	       PCIE_GetLastErr());
    }
}



/* Read/write memory or I/O space address menu */
//static void MenuMBtest(WDC_DEVICE_HANDLE hDev, WDC_DEVICE_HANDLE hDev1 ,WDC_DEVICE_HANDLE hDev2, 
//		       WDC_DEVICE_HANDLE hDev3, WDC_DEVICE_HANDLE hDev4, WDC_DEVICE_HANDLE hDev5)
static void MenuMBtest(WDC_DEVICE_HANDLE hDev, WDC_DEVICE_HANDLE hDev1 ,WDC_DEVICE_HANDLE hDev2, 
		       WDC_DEVICE_HANDLE hDev3, WDC_DEVICE_HANDLE hDev4, WDC_DEVICE_HANDLE hDev5,int min__, int max__)

{

#include "wdc_defs.h"
#define poweroff      0x0
#define poweron       0x1
#define configure_s30 0x2
#define configure_s60 0x3
#define configure_cont 0x20
#define rdstatus      0x80
#define loopback        0x04

#define dcm2_run_off  254
#define dcm2_run_on   255

#define dcm2_online   2
#define dcm2_setmask  3
#define dcm2_offline_busy 4
#define dcm2_load_packet_a 10
#define dcm2_load_packet_b 11
#define dcm2_offline_load 9
#define dcm2_status_read 20
#define dcm2_led_sel     29
#define dcm2_buffer_status_read 30
#define dcm2_status_read_inbuf 21
#define dcm2_status_read_evbuf 22
#define dcm2_status_read_noevnt 23
#define dcm2_zero 12
#define dcm2_compressor_hold 31

#define dcm2_5_readdata 4
#define dcm2_5_firstdcm 8
#define dcm2_5_lastdcm  9
#define dcm2_5_status_read 5
#define dcm2_5_source_id 25
#define dcm2_5_lastchnl 24

#define dcm2_packet_id_a 25
#define dcm2_packet_id_b 26
#define dcm2_hitformat_a 27
#define dcm2_hitformat_b 28

#define part_run_off  254
#define part_run_on   255
#define part_online   2
#define part_offline_busy 3
#define part_offline_hold 4
#define part_status_read 20
#define part_source_id 25


#define  t1_tr_bar 0
#define  t2_tr_bar 4
#define  cs_bar 2

  /**  command register location **/

#define  tx_mode_reg 0x28
#define  t1_cs_reg 0x18
#define  r1_cs_reg 0x1c
#define  t2_cs_reg 0x20
#define  r2_cs_reg 0x24

#define  tx_md_reg 0x28

#define  cs_dma_add_low_reg 0x0
#define  cs_dma_add_high_reg  0x4
#define  cs_dma_by_cnt 0x8
#define  cs_dma_cntrl 0xc
#define  cs_dma_msi_abort 0x10

  /** define status bits **/

#define  cs_init  0x20000000
#define  cs_mode_p 0x8000000
#define  cs_mode_n 0x0
#define  cs_start 0x40000000
#define  cs_done  0x80000000

#define  dma_tr1  0x100000
#define  dma_tr2  0x200000
#define  dma_tr12 0x300000
#define  dma_3dw_trans 0x0
#define  dma_4dw_trans 0x0
#define  dma_3dw_rec   0x40
#define  dma_4dw_rec   0x60
#define  dma_in_progress 0x80000000

#define  dma_abort 0x2

#define  mb_cntrl_add     0x1
#define  mb_cntrl_test_on 0x1
#define  mb_cntrl_test_off 0x0
#define  mb_cntrl_set_run_on 0x2
#define  mb_cntrl_set_run_off 0x3
#define  mb_cntrl_set_trig1 0x4
#define  mb_cntrl_set_trig2 0x5
#define  mb_cntrl_load_frame 0x6
#define  mb_cntrl_load_trig_pos 0x7

#define  mb_feb_power_add 0x1
#define  mb_feb_conf_add 0x2
#define  mb_feb_pass_add 0x3

#define  mb_feb_lst_on          1
#define  mb_feb_lst_off         0
#define  mb_feb_rxreset         2
#define  mb_feb_align           3
#define  mb_feb_pll_reset       5


#define  mb_feb_adc_align       1
#define  mb_feb_a_nocomp        2
#define  mb_feb_b_nocomp        3
#define  mb_feb_blocksize       4
#define  mb_feb_timesize        5
#define  mb_feb_mod_number      6
#define  mb_feb_a_id            7
#define  mb_feb_b_id            8
#define  mb_feb_max             9

#define  mb_feb_test_source    10
#define  mb_feb_test_sample    11
#define  mb_feb_test_frame     12
#define  mb_feb_test_channel   13
#define  mb_feb_test_ph        14
#define  mb_feb_test_base      15
#define  mb_feb_test_ram_data  16

#define  mb_feb_a_test         17
#define  mb_feb_b_test         18

#define  mb_feb_rd_status      20

#define  mb_feb_a_rdhed        21
#define  mb_feb_a_rdbuf        22
#define  mb_feb_b_rdhed        23
#define  mb_feb_b_rdbuf        24

#define  mb_feb_read_probe     30
#define  mb_feb_dram_reset     31
#define  mb_feb_adc_reset      33

#define  mb_a_buf_status       34
#define  mb_b_buf_status       35
#define  mb_a_ham_status       36
#define  mb_b_ham_status       37

#define  mb_feb_a_maxwords     40
#define  mb_feb_b_maxwords     41

#define  mb_feb_hold_enable    42

#define  mb_feb_tpc_load_threshold   100
#define  mb_feb_tpc_load_baseline    164
  //#define  mb_feb_tpc_load_presample   230
  //#define  mb_feb_tpc_load_postsample  231
#define  mb_feb_tpc_sel_combase     232
  //#define  mb_feb_tpc_sel_comthres    233
#define  mb_feb_tpc_load_combase   234
#define  mb_feb_tpc_load_comthres   235
  //#define  mb_feb_tpc_sel_bipolar   236

#define  mb_feb_tpc_load_thr_mean   164
#define  mb_feb_tpc_load_thr_vari   165
#define  mb_feb_tpc_load_common_thr 166
#define  mb_feb_tpc_sel_bipolar     167
#define  mb_feb_tpc_load_presample  168
#define  mb_feb_tpc_load_postsample 169
#define  mb_feb_tpc_sel_comthres    170

#define  mb_pmt_adc_reset       1
#define  mb_pmt_spi_add         2
#define  mb_pmt_adc_data_load   3

#define  mb_xmit_conf_add 0x2
#define  mb_xmit_pass_add 0x3

#define  mb_xmit_modcount 0x1
#define  mb_xmit_enable_1 0x2
#define  mb_xmit_enable_2 0x3
#define  mb_xmit_test1 0x4
#define  mb_xmit_test2 0x5

#define   mb_xmit_testdata  10

#define  mb_xmit_rdstatus 20
#define  mb_xmit_rdcounters 21
#define  mb_xmit_link_reset    22
#define  mb_opt_dig_reset   23
#define  mb_xmit_dpa_fifo_reset    24
#define  mb_xmit_dpa_word_align    25
#define  mb_xmit_link_pll_reset    26

#define  mb_trig_run                1
#define  mb_trig_frame_size         2
#define  mb_trig_deadtime_size      3
#define  mb_trig_active_size        4
#define  mb_trig_delay1_size        5
#define  mb_trig_delay2_size        6
#define  mb_trig_enable             7

#define  mb_trig_calib_delay        8

#define  mb_trig_prescale0         10
#define  mb_trig_prescale1         11
#define  mb_trig_prescale2         12
#define  mb_trig_prescale3         13
#define  mb_trig_prescale4         14
#define  mb_trig_prescale5         15
#define  mb_trig_prescale6         16
#define  mb_trig_prescale7         17
#define  mb_trig_prescale8         18

#define  mb_trig_mask0             20
#define  mb_trig_mask1             21
#define  mb_trig_mask2             22
#define  mb_trig_mask3             23
#define  mb_trig_mask4             24
#define  mb_trig_mask5             25
#define  mb_trig_mask6             26
#define  mb_trig_mask7             27
#define  mb_trig_mask8             28

#define  mb_trig_rd_param          30
#define  mb_trig_pctrig            31
#define  mb_trig_rd_status         32
#define  mb_trig_reset             33
#define  mb_trig_calib             34
#define  mb_trig_rd_gps            35

#define  mb_trig_sel1              40
#define  mb_trig_sel2              41
#define  mb_trig_sel3              42
#define  mb_trig_sel4              43

#define  mb_trig_p1_delay          50
#define  mb_trig_p1_width          51
#define  mb_trig_p2_delay          52
#define  mb_trig_p2_width          53
#define  mb_trig_p3_delay          54
#define  mb_trig_p3_width          55
#define  mb_trig_pulse_delay       58
#define  mb_trig_output_select     59

#define  mb_trig_pulse1            60
#define  mb_trig_pulse2            61
#define  mb_trig_pulse3            62

#define  mb_trig_frame_trig        63

#define  mb_trig_frame_trig_frm    83
#define  mb_trig_frame_trig_div    84

#define  mb_shaper_pulsetime        1
#define  mb_shaper_dac              2
#define  mb_shaper_pattern          3
#define  mb_shaper_write            4
#define  mb_shaper_pulse            5
#define  mb_shaper_entrig           6

#define  mb_feb_pmt_gate_size      47
#define  mb_feb_pmt_beam_delay     48
#define  mb_feb_pmt_beam_size      49

#define  mb_feb_pmt_ch_set         50
#define  mb_feb_pmt_delay0         51
#define  mb_feb_pmt_delay1         52
#define  mb_feb_pmt_precount       53
#define  mb_feb_pmt_thresh0        54
#define  mb_feb_pmt_thresh1        55
#define  mb_feb_pmt_thresh2        56
#define  mb_feb_pmt_thresh3        57
#define  mb_feb_pmt_width          58
#define  mb_feb_pmt_deadtime       59
#define  mb_feb_pmt_window         60
#define  mb_feb_pmt_words          61
#define  mb_feb_pmt_cos_mul        62
#define  mb_feb_pmt_cos_thres      63
#define  mb_feb_pmt_mich_mul       64
#define  mb_feb_pmt_mich_thres     65
#define  mb_feb_pmt_beam_mul       66
#define  mb_feb_pmt_beam_thres     67
#define  mb_feb_pmt_en_top         68
#define  mb_feb_pmt_en_upper       69
#define  mb_feb_pmt_en_lower       70
#define  mb_feb_pmt_blocksize      71

#define  mb_feb_pmt_test           80
#define  mb_feb_pmt_clear          81
#define  mb_feb_pmt_test_data      82
#define  mb_feb_pmt_pulse          83

#define  mb_feb_pmt_rxreset        84
#define  mb_feb_pmt_align_pulse    85
#define  mb_feb_pmt_rd_counters    86

#define  dma_buffer_size        40000000

  static DWORD dwAddrSpace;

  static UINT32 u32Data;
  static unsigned short u16Data;
  static unsigned long long u64Data, u64Data1;
  static DWORD dwOffset;
  static long imod,ichip;
  unsigned short *buffp;


  //vic
  char title1[100];
  char title[100];
  //vic
    
  /*
    WDC_ADDR_MODE mode;
    WDC_ADDR_RW_OPTIONS options;
  */
  static UINT32 i,j,k,ifr,nread,iprint,iwrite,ik,il,is,checksum;
  static UINT32 istop,newcmd,irand,ioffset,kword,lastchnl,ib;
  static UINT32 send_array[40000],read_array[dma_buffer_size],read_array1[40000];
  static UINT32 read_array_c[40000];
  static UINT32 read_comp[8000];
  static UINT32 nmask,index,itmp,nword_tot,nevent,iv,ijk,islow_read;
  static UINT32 imod_p,imod_trig,imod_shaper;
  unsigned short idcm_read_array[40000],read_array_s[1600000];
  static UINT32 idcm_read_array32[40000];
  static UINT32 idcm_send_array[400000];
  static UINT32 idcm_verify_array[400000];
  static int icomp_l,comp_s,ia,ic,ihuff,sample_b,dis;
  UINT32 *idcm_send_p,*idcm_verify_p,*pbuffp_rec;
  //    DWORD dwDMABufSize;
  PVOID pbuf;
  WD_DMA *pDma;
  DWORD dwStatus;
  DWORD dwOptions = DMA_FROM_DEVICE;
  UINT32 iread,icheck,izero;
  UINT32 buf_send[40000];
  static int   count,num,counta,nword,ireadback,nloop,ierror;
  static int   ij,nsend,iloop,inew,idma_readback,iadd,jevent;
  static int   itest,iframe,irun,ichip_c,dummy1,itrig_c;
  static int   idup,ihold,idouble,ihold_set,istatus_read;
  static int   idone,tr_bar,t_cs_reg,r_cs_reg,dma_tr;
  static int   timesize,ipulse,ibase,a_id,itrig_delay;
  static int   iset,ncount,nsend_f,nwrite,itrig_ext;
  static int   imod_xmit,idiv,isample;
  static int   iframe_length, itrig,idrift_time,ijtrig;
  static int   idelay0, idelay1, threshold0, threshold1, pmt_words;
  static int   cos_mult, cos_thres, en_top, en_upper, en_lower;
  static int   irise, ifall, istart_time, use_pmt, pmt_testpulse;
  static int   ich_head, ich_sample, ich_frm,idebug,ntot_rec,nred;
  static int   ineu,ibusy_send,ibusy_test,ihold_word,ndma_loop;
  static int   irawprint, nwrite_byte,idis_c,idis_c1;
  static int   icomp_index, nword_comp, nk, ilast_check;
  static int   ic_ev, ic_fr, event_save, frame_save,frame_ev,event_ev;
  static int   imod_fem, imod_st, imod_last, itrig_type, last_dma_loop_size;
  static int   fd, n_read, n_write, pt_trig_wdone, pt_snova_wdone;
  static int   pt_trig_dmastart;
  static int   nremain, nread_dma,nremain_tran1, nremain_tran2,ig, nremain_dma;
  static int   rc_pt, nword_n, nwrite_byte_n, is1, event_head[100];
  static int   icom_factor,ifr_c2;
  static int   itrig_pulse,p1_delay,p1_width,p2_delay,p2_width, pulse_trig_delay;
  static int   icont,ibytec,n_trig, imulti,isuper, ipr_trig, imonitor;
  static int   ipmt_read, itpc_read, itpc_adc, imod_xmit_tpc, imod_st_tpc,imod_last_tpc;
  static int   islope, iwidth, ibeg, izero_sup, istart, istart_old;
  static int   iround, idir;
  static int   icount, idiff, adc_v;
  //
  struct timeval start;
    
    
  static int fd_sn;
  //
  void *status_pt;
  size_t stacksize;
  //
  //
  pthread_t threads[6];
  //
  pthread_attr_t attr_pt_sn_cp, attr_pt_tr_cp;
  pthread_attr_t attr_pt_trig_dma, attr_pt_sn_dma;
  pthread_attr_t attr_pt_tr, attr_pt_sn;

  //
  pthread_attr_t attr_pt_trig_dma_tpc, attr_pt_sn_dma_tpc;
  pthread_attr_t attr_pt_tr_tpc, attr_pt_sn_tpc;
  //
  pthread_attr_t attr_pt_tr_m, attr_pt_trig_dma_m;
  pthread_attr_t attr_pt_fake;

  //
  unsigned char    charchannel;
  unsigned char    carray[4000];
  struct timespec tim, tim2;
  tim.tv_sec = 0;
  //    tim.tv_nsec =128000;
  tim.tv_nsec =172000; // extend the delay for 12 MHz clock

  PVOID pbuf_rec;
  WD_DMA *pDma_rec;
  PVOID pbuf_rec1;
  WD_DMA *pDma_rec1;
  PVOID pbuf_rec2;
  WD_DMA *pDma_rec2;

  //    PVOID pbuf_rec_n;
  //    WD_DMA *pDma_rec_n;

  DWORD dwOptions_send = DMA_TO_DEVICE | DMA_ALLOW_CACHE;
  //    DWORD dwOptions_rec = DMA_FROM_DEVICE | DMA_ALLOW_CACHE | DMA_ALLOW_64BIT_ADDRESS;
  DWORD dwOptions_rec = DMA_FROM_DEVICE | DMA_ALLOW_64BIT_ADDRESS;

  static UINT64 *buffp_rec64;
  static UINT32 *buffp_rec32, *buffp_rec32_n;
  static UINT32 *buffp_send;
  UINT32 *px, *py, *py1;

  FILE *outf,*inpf,*outfile,*pFilee;

  struct thread_data
  {
    int id;
    WDC_DEVICE_HANDLE hdev;
    WDC_DEVICE_HANDLE hdevc;
  };

  struct thread_data thread_data_n;
  struct thread_data thread_data_sn;
  //
  struct thread_data thread_data_n_tpc;
  struct thread_data thread_data_sn_tpc;
  //
  struct thread_data thread_data_n_m;

  struct timeval starttest, endtest;
  long mytime, seconds, useconds;

  nread = 4096*2+6; /*16384 32768, 65536+4;  number of byte to be readout */
  ifr=0;
  iwrite =0;
  iprint =0;
  icheck =0;
  istop=0;

  printf("\t==> Your only choice ==> (5) SuperNova readout test -- thread \n");

  scanf("%d",&newcmd);
  
  newcmd=5; 
  switch(newcmd) { //useless switch statement

    //dwDMABufSize = 1000000;
    
  case 5:
    
    
    VDEBUG = 0;
    
    
    printf("Case 5 thread readout\n");
    //     printf(" number of event per loop \n");
    //     scanf("%d",&nevent);
    //nloop  = 10000;
    nloop = 1;
    nevent = 10000;
    //printf(" enter 1 to turn on huffman encoding \n");
    //scanf("%d",&ihuff);
    ihuff = 1;
    //     printf("typw 1 to compare with the 1st event\n");
    //     scanf("%d",&comp_s);
    //     printf("type 1 for print out debug information in dma loop\n");
    //     scanf("%d",&idebug);
    //     printf("type 1 for raw data print, 2 for the decoded data \n");
    //     scanf("%d",&irawprint);
    //printf(" enter compression factor \n");
    //scanf("%d",&icom_factor);
    icom_factor=1;
    /* printf(" enter 1 to enable the neutrino trigger \n"); */
    /* scanf("%d",&ineu); */
    ineu = 1;
    //printf(" xmit module address \n");
    //scanf("%d",&imod_xmit);
    imod_xmit = 2;
    //printf(" slot address of the 1st FEM module \n");
    /* scanf("%d",&imod_st); */
    imod_st = 3;
    printf(" number of FEM = %d\n",(imod_st - imod_xmit));
    /* printf(" write the file through thread \n"); */
    /* scanf("%d",&ith_fr); */
    ith_fr = 1;
    //     printf(" type 1 for neutrino event, 2 for superNova \n");
    //     scanf("%d",&itrig_type);
    itrig_type =2 ;
    if(ineu == 1) itrig_type = 3;

    fd_sn_pt = creat("test123_pt_snova_thres0_Bmean1000_BVar10000.dat",0755);
    printf("fd_sn_pt = %d\n", fd_sn_pt);
    if(ineu == 1) {
      fd_trig_pt = creat("test123_pt_trig_thres0_Bmean1000_BVar10000.dat",0755);
      printf("fd_trig_pt = %d\n", fd_trig_pt);
    }
    pt_trig_wdone=1;
    pt_snova_wdone=1;
    pt_trig_dmastart=1;
    //
    //
    pthread_mutex_init(&mutexlock, NULL);
    pthread_mutex_init(&mutexlock, NULL);
    //
    /* Initialize and set thread detached attribute */
    //
    pthread_attr_init(&attr_pt_sn_dma);
    pthread_attr_setdetachstate(&attr_pt_sn_dma, PTHREAD_CREATE_JOINABLE);
    pthread_attr_getstacksize (&attr_pt_sn_dma, &stacksize);
    printf("SuperNova DMA defualt stack size = %li\n", stacksize);
    stacksize = 4*stacksize;
    //      printf("Amount of stack needed per thread = %li\n",stacksize);
    pthread_attr_setstacksize (&attr_pt_sn_dma, stacksize);
    //      printf("Creating threads with stack size = %li bytes\n",stacksize);

    //
    //      superNova file write thread
    //
    pthread_attr_init(&attr_pt_sn);
    pthread_attr_setdetachstate(&attr_pt_sn, PTHREAD_CREATE_JOINABLE);
    pthread_attr_getstacksize (&attr_pt_sn, &stacksize);
    printf("SuperNova filewrite defualt stack size = %li\n", stacksize);
    stacksize = 4*stacksize;
    //      printf("Amount of stack needed per thread = %li\n",stacksize);
    pthread_attr_setstacksize (&attr_pt_sn, stacksize);
    //      printf("Creating threads with stack size = %li bytes\n",stacksize);
    //


    //      superNova file copythread
    //
    pthread_attr_init(&attr_pt_sn_cp);
    pthread_attr_setdetachstate(&attr_pt_sn_cp, PTHREAD_CREATE_JOINABLE);
    pthread_attr_getstacksize (&attr_pt_sn_cp, &stacksize);
    printf("SuperNova copythread defualt stack size = %li\n", stacksize);
    stacksize = 4*stacksize;
    //      printf("Amount of stack needed per thread = %li\n",stacksize);
    pthread_attr_setstacksize (&attr_pt_sn_cp, stacksize);
    //      printf("Creating threads with stack size = %li bytes\n",stacksize);
      


    if(ineu == 1 ) {
      //
      //   neutrino data thread
      //
      pthread_attr_init(&attr_pt_trig_dma);
      pthread_attr_setdetachstate(&attr_pt_trig_dma, PTHREAD_CREATE_JOINABLE);
      //
      pthread_attr_getstacksize (&attr_pt_trig_dma, &stacksize);
      printf("Trigger Default stack size = %li\n", stacksize);
      stacksize = 4*stacksize;
      //      printf("Amount of stack needed per thread = %li\n",stacksize);
      pthread_attr_setstacksize (&attr_pt_trig_dma, stacksize);
      //      printf("Creating threads with stack size = %li bytes\n",stacksize);
      //
      //    neutrino filewrite thread
      //
      pthread_attr_init(&attr_pt_tr);
      pthread_attr_setdetachstate(&attr_pt_tr, PTHREAD_CREATE_JOINABLE);
      //
      pthread_attr_getstacksize (&attr_pt_tr, &stacksize);
      printf("Trigger Default stack size = %li\n", stacksize);
      stacksize = 4*stacksize;
      //      printf("Amount of stack needed per thread = %li\n",stacksize);
      pthread_attr_setstacksize (&attr_pt_tr, stacksize);
      //      printf("Creating threads with stack size = %li bytes\n",stacksize);
      //

      //      neutrion copythread
      //
      pthread_attr_init(&attr_pt_tr_cp);
      pthread_attr_setdetachstate(&attr_pt_tr_cp, PTHREAD_CREATE_JOINABLE);
      pthread_attr_getstacksize (&attr_pt_tr_cp, &stacksize);
      printf("Trigger copythread defualt stack size = %li\n", stacksize);
      stacksize = 4*stacksize;
      //      printf("Amount of stack needed per thread = %li\n",stacksize);
      pthread_attr_setstacksize (&attr_pt_tr_cp, stacksize);
      //      printf("Creating threads with stack size = %li bytes\n",stacksize);
    }
      
    

    /* iframe_length = 8191; */
    /* timesize = 10; */
    /* itrig_delay = 10; */


    /* iframe_length = 25599; */
    /* timesize = 3199; */
    /* itrig_delay = 100000;//16 */

    //dwDMABufSize = 1000000;
    
    //xxx
    dwDMABufSize = 200000; // 2e5 same as BNB run fcl
      

    //     printf(" 1 for checking the event \n");
    //     scanf("%d",&icheck);
    //     printf(" type 1 to use random number \n");
    //     scanf("%d",&irand);
    icheck =0;
    ifr=0;
    irand = 0;
    islow_read =0;

    iprint = 1;

    nsend=500;
 
    // once the fpga is booted we should let system receive fill frame before send any data.
    // set system with normal transmitter mode

    /* iframe_length = 8191; */
    /* timesize = 10; */
    /* itrig_delay = 10; */

    iframe_length = 25599;
    timesize = 3199;
    /* timesize =500; */
    itrig_delay = 16;

    printf(" frame size = %d, timesize = %d\n",iframe_length+1, timesize);
    printf(" buffer size = %d\n", dwDMABufSize);
    printf(" trigger delay = %d\n", itrig_delay);


    //dwDMABufSize = 1000000;
    
    //xxx
    dwDMABufSize = 200000; // 2e5 same as BNB run fcl
      
    printf(" frame size = %d, timesize = %d\n",iframe_length+1, timesize);
    printf(" buffer size = %d\n", dwDMABufSize);
    printf(" trigger delay = %d\n", itrig_delay);
    icheck =0;
    ifr=0;
    irand = 0;
    islow_read =0;

    iprint = 1;

    nsend=500;
    
    //initialize SN transmitters
    dwAddrSpace =2;
    u32Data = 0x20000000;    // initial transmitter, no hold
    dwOffset = 0x18;
    WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);
    dwAddrSpace =2;
    u32Data = 0x20000000;    // initial transmitter, no hold
    dwOffset = 0x20;
    WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);

    dwAddrSpace =2;
    u32Data = 0x20000000;    // initial receiver
    dwOffset = 0x1c;
    WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);
    dwAddrSpace =2;
    u32Data = 0x20000000;   // initial receiver
    dwOffset = 0x24;
    WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);

    dwAddrSpace =2;
    u32Data = 0xfff;    // set mode off with 0xfff...
    dwOffset = 0x28;
    WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);
      
    if ( ineu == 1 ) {
      //neutrino
      dwAddrSpace =2;
      u32Data = 0x20000000;    // initial transmitter, no hold
      dwOffset = 0x18;
      WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);
      dwAddrSpace =2;
      u32Data = 0x20000000;    // initial transmitter, no hold
      dwOffset = 0x20;
      WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);

      dwAddrSpace =2;
      u32Data = 0x20000000;    // initial receiver
      dwOffset = 0x1c;
      WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);
      dwAddrSpace =2;
      u32Data = 0x20000000;   // initial receiver
      dwOffset = 0x24;
      WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);

      dwAddrSpace =2;
      u32Data = 0xfff;    // set mode off with 0xfff...
      dwOffset = 0x28;
      WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);
    }
    //
    //
    //
    px = &buf_send;
    py = &read_array;
    imod =0;  /* controller module */
    /** initialize **/
    buf_send[0]=0x0;
    buf_send[1]=0x0;
    i=1;
    k=1;
    i = pcie_send(hDev3, i, k, px);
    // set offline test
    imod=0;
    ichip=1;
    buf_send[0]=(imod<<11)+(ichip<<8)+(mb_cntrl_test_on)+(0x0<<16); //enable offline run on
    i=1;
    k=1;
    i = pcie_send(hDev3, i, k, px);
    //disable the run command
    imod=0;
    ichip=1;
    buf_send[0]=(imod<<11)+(ichip<<8)+(mb_cntrl_set_run_off)+(0x0<<16); //turn off run
    i=1;
    k=1;
    i = pcie_send(hDev3, i, k, px);

    for (j=0; j<nloop; j++) {
      usleep(10000); // wait for 10ms

      //
      //    boot up xmit module 1st
      //
      printf(" boot xmit module \n");
      /* inpf = fopen("/home/ub/xmit_fpga_link","r"); */
      inpf = fopen("/home/ub/xmit_fpga_link_header","r");
      imod=imod_xmit;
      ichip=mb_xmit_conf_add;
      buf_send[0]=(imod<<11)+(ichip<<8)+0x0+(0x0<<16);  // turn conf to be on
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      
      //      for (i=0; i<100000; i++) {
      //          ik= i%2;
      //          dummy1= (ik+i)*(ik+i);
      //      }

      /* read data as characters (28941) */
      usleep(1000);   // wait fior a while
      count = 0;
      counta= 0;
      ichip_c = 7; // set ichip_c to stay away from any other command in the
      dummy1 =0;
      while (fread(&charchannel,sizeof(char),1,inpf)==1) {
	carray[count] = charchannel;
	count++;
	counta++;
	if((count%(nsend*2)) == 0) {
	  //        printf(" loop = %d\n",dummy1);
	  buf_send[0] = (imod <<11) +(ichip_c <<8)+ (carray[0]<<16);
	  send_array[0] =buf_send[0];
	  if(dummy1 <= 5 ) printf(" counta = %d, first word = %x, %x, %x %x %x \n",counta,buf_send[0], carray[0], carray[1]
				  ,carray[2], carray[3]);
	  for (ij=0; ij< nsend; ij++) {
	    if(ij== (nsend-1)) buf_send[ij+1] = carray[2*ij+1]+(0x0<<16);
	    else buf_send[ij+1] = carray[2*ij+1]+ (carray[2*ij+2]<<16);
	    //         buf_send[ij+1] = carray[2*ij+1]+ (carray[2*ij+2]<<16);
	    send_array[ij+1] = buf_send[ij+1];
	  }
	  nword =nsend+1;
	  i=1;
	  //       if(dummy1 == 0)
	  ij = pcie_send(hDev3, i, nword, px);
	  nanosleep(&tim , &tim2);
	  dummy1 = dummy1+1;
	  count =0;
	}
      }
      if(feof(inpf)) {
	printf("You have reached the end-of-file word count= %d %d\n", counta, count);
	buf_send[0] = (imod <<11) +(ichip_c <<8)+ (carray[0]<<16);
	if ( count > 1) {
	  if( ((count-1)%2) ==0) {
	    ik =(count-1)/2;
	  }
	  else {
	    ik =(count-1)/2+1;
	  }
	  ik=ik+2;   // add one more for safety
	  printf("ik= %d\n",ik);
	  for (ij=0; ij<ik; ij++){
	    if(ij == (ik-1)) buf_send[ij+1] = carray[(2*ij)+1]+(((imod<<11)+(ichip<<8)+0x0)<<16);
	    else buf_send[ij+1] = carray[(2*ij)+1]+ (carray[(2*ij)+2]<<16);
	    send_array[ij+1] = buf_send[ij+1];
	  }
	}
	else ik=1;

	for (ij=ik-10; ij< ik+1; ij++) {
	  printf("Last data = %d, %x\n",ij,buf_send[ij]);
	}

	nword =ik+1;
	i=1;
	i = pcie_send(hDev3, i, nword, px);
      }
      usleep(2000);    // wait for 2ms to cover the packet time plus fpga init time
      fclose(inpf);
      //
      printf(" xmit done, booting FEM \n");

      //vic: put it in 

      imod_last = imod_xmit+1;
      for (imod_fem = (imod_xmit+1); imod_fem< (imod_st+1); imod_fem++) {

      	ik=tpc_adc_setup(hDev3, imod_fem, iframe_length, 1, 0, 4, timesize);

      }
      //vic: stop
      
      //
      //    Boot stratix after XMIT module
      //
      
      
      #if 0
      imod_last = imod_xmit+1;
      for (imod_fem = (imod_xmit+1); imod_fem< (imod_st+1); imod_fem++) {
      	imod = imod_fem;
      	//
      	// turn on the Stratix III power supply
      	//       imod=11;
      	ichip =1;
      	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_power_add+(0x0<<16); //turn module 11 power on
      	i=1;
      	k=1;
      	i = pcie_send(hDev3, i, k, px);
      	usleep(200000);  // wait for 200 ms
      	//
      	/* inpf = fopen("/home/ub/feb_fpga_test","r"); */ // old reference code
      	//inpf = fopen("/home/ub/module1x_140820_deb_3_8_2016.rbf","r"); // Chi's new FPGA code (Jan 25, 2016)
      	inpf = fopen("/home/ub/module1x_140820_deb_3_21_2016.rbf","r"); // Chi's new-est FPGA code (Mar 21, 2016)
      	printf("\n\t==> Start booting FEM %d\n", imod);
      	ichip=mb_feb_conf_add;
      	buf_send[0]=(imod<<11)+(ichip<<8)+0x0+(0x0<<16);  // turn conf to be on
      	i=1;
      	k=1;
      	i = pcie_send(hDev3, i, k, px);
	
      	//      for (i=0; i<100000; i++) {
      	//          ik= i%2;
      	//          dummy1= (ik+i)*(ik+i);
      	//      }


      	/* read data as characters (28941) */
      	usleep(1000);   // wait fior a while
      	count = 0;
      	counta= 0;
      	ichip_c = 7; // set ichip_c to stay away from any other command in the
      	dummy1 =0;
      	while (fread(&charchannel,sizeof(char),1,inpf)==1) {
      	  carray[count] = charchannel;
      	  count++;
      	  counta++;
      	  if((count%(nsend*2)) == 0) {
      	    buf_send[0] = (imod <<11) +(ichip_c <<8)+ (carray[0]<<16);
      	    send_array[0] =buf_send[0];
      	    if(dummy1 <= 5 ) printf(" counta = %d, first word = %x, %x, %x %x %x \n",counta,buf_send[0], carray[0], carray[1]
      				    ,carray[2], carray[3]);
      	    for (ij=0; ij< nsend; ij++) {
      	      if(ij== (nsend-1)) buf_send[ij+1] = carray[2*ij+1]+(0x0<<16);
      	      else buf_send[ij+1] = carray[2*ij+1]+ (carray[2*ij+2]<<16);
      	      //         buf_send[ij+1] = carray[2*ij+1]+ (carray[2*ij+2]<<16);
      	      send_array[ij+1] = buf_send[ij+1];
      	    }
      	    nword =nsend+1;
      	    i=1;
      	    //       if(dummy1 == 0)
      	    ij = pcie_send(hDev3, i, nword, px);
      	    nanosleep(&tim , &tim2);
      	    dummy1 = dummy1+1;
      	    count =0;
      	  }
      	}
      	if(feof(inpf)) {
      	  printf("You have reached the end-of-file word count= %d %d\n", counta, count);
      	  buf_send[0] = (imod <<11) +(ichip_c <<8)+ (carray[0]<<16);
      	  if ( count > 1) {
      	    if( ((count-1)%2) ==0) {
      	      ik =(count-1)/2;
      	    }
      	    else {
      	      ik =(count-1)/2+1;
      	    }
      	    ik=ik+2;   // add one more for safety
      	    printf("ik= %d\n",ik);
      	    for (ij=0; ij<ik; ij++){
      	      if(ij == (ik-1)) buf_send[ij+1] = carray[(2*ij)+1]+(((imod<<11)+(ichip<<8)+0x0)<<16);
      	      else buf_send[ij+1] = carray[(2*ij)+1]+ (carray[(2*ij)+2]<<16);
      	      send_array[ij+1] = buf_send[ij+1];
      	    }
      	  }
      	  else ik=1;

      	  for (ij=ik-10; ij< ik+1; ij++) {
      	    printf("Last data = %d, %x\n",ij,buf_send[ij]);
      	  }

      	  nword =ik+1;
      	  i=1;
      	  i = pcie_send(hDev3, i, nword, px);
      	}
      	usleep(2000);    // wait for 2ms to cover the packet time plus fpga init time
      	fclose(inpf);
      }
      #endif

      
      
      //
      //    both FEM and XMIT bootted.
      //
      //
      //   /* set tx mode register */
      //
      u32Data = 0x00003fff;  // set up number of words hold coming back from the XMIT module
      printf(" number of words for hold be send back = %x\n",u32Data);
      dwOffset = 0x28;
      dwAddrSpace =2;
      WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);
      //
      //   set up hold
      //
      printf(" set up the hold condition \n");
      dwAddrSpace =2;
      u32Data = 0x8000000;    // set up transmitter to return the hold -- upper transciever
      dwOffset = 0x18;
      WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);
      dwAddrSpace =2;
      u32Data = 0x8000000;    // set up transmitter to return the hold -- lower transciever
      dwOffset = 0x20;
      WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);
	
      //
      ith_fr=1;
      if((ineu ==1) & (ith_fr ==1)) {
	//
	/* set tx mode register */
	//
	u32Data = 0x00003fff;  // set up number of words hold coming back from the XMIT module
	printf(" number of words for hold be send back -- trigger = %x\n",u32Data);
	dwOffset = 0x28;
	dwAddrSpace =2;
	WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);
	//
	//   set up hold
	//
	printf(" set up the hold condition \n");
	dwAddrSpace =2;
	u32Data = 0x8000000;    // set up transmitter to return the hold -- upper transciever
	dwOffset = 0x18;
	WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);
	dwAddrSpace =2;
	u32Data = 0x8000000;    // set up transmitter to return the hold -- lower transciever
	dwOffset = 0x20;
	WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);
      }
      //
      // set frame set to be 255 --- there will be 256/8 = 32 adc samples.
      //
      
      #if 0
      imod=0;
      ichip=1;
      iframe= iframe_length;   
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_cntrl_load_frame)+((iframe & 0xffff)<<16); //enable test mode
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      
      //
      // load trig 1 position relative to the frame..
      //
      
      imod=0;
      ichip=1;
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_cntrl_load_trig_pos)+((itrig_delay & 0xffff)<<16); //enable test mode
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      
      //
      //    start testing routine
      //
      /* printf("\t==> Enter 1 to continue with the testing routine\n"); */
      /* scanf("%d",&ik); */
      //      ik =1;
      for (imod_fem = (imod_xmit+1); imod_fem< (imod_st+1); imod_fem++) {
	imod=imod_fem;
	//
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_dram_reset+(0x1<<16);  // turm the DRAM reset on
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
	//        imod=11;
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_dram_reset+(0x0<<16);  // turm the DRAM reset off
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);

	usleep(5000);    // wait for 5 ms for DRAM to be initialized

	//         imod=11;
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_mod_number+(imod<<16);  // set module number
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
	//
	//
	nword =1;
	i = pcie_rec(hDev3,0,1,nword,iprint,py);     // init the receiver

	//         imod=11;
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_rd_status+(0x0<<16);  // read out status
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
	py = &read_array;
	i = pcie_rec(hDev3,0,2,nword,iprint,py);     // read out 2 32 bits words
	printf("receive data word = %x, %x \n", read_array[0], read_array[1]);


	nword =1;
	//
	// set to use test generator 2, set test =2
	//
	//       imod=11;
	ichip=mb_feb_pass_add;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_test_source+(0x2<<16);  // set test source to 2
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);

	iround =0;
	idir =1;
	//
	//    start loading the test 2 data memory
	//
	//       imod =11;
	ichip=3;
	for (is=0; is<64; is++) {
	  ik = 0x4000+is;                        // load channel address
	  buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_test_ram_data)+((ik & 0xffff)<<16); // load channe address
	  i = pcie_send(hDev3, 1, 1, px);
	  il = is%8;
	  if(il == 0) printf(" loading channel %d\n",is);
	  for (ik=0; ik< 256; ik++) {                 // loop over all possible address
	    ibase = is+1;    // set the base value of the ADC data

	    /* if(iround == 3) idir =-1; */
	    /* if(iround == 0) idir = 1; */
	    /* iround = iround + idir; */
	    /* ijk= ibase + iround; */
         
	    /* /\* if( (ik >= min__) && (ik <= max__) ) ijk=300+ibase; *\/ */
	    /* /\* if( (ik >= 254) || (ik <= 5) ) ijk=300+ibase; *\/ */
	    /* if( (ik >= 200) && (ik <= 207) ) ijk=300+ibase; */

	    /* if(iround == 3) idir =-1; */
	    if(iround == 12) idir =-4; // Huffman-incompressible baseline
	    /* if(iround == 0) idir = 1; */
	    if(iround == 0) idir = +4; // Huffman-incompressible baseline
	    iround = iround + idir;
	    ijk= ibase + iround;
	    
	    if( ik < 2 )   ijk += 20;
	    if( ik > 250 ) ijk += 300; 
	    

	    /* if(iround == 3) idir =-1; */
	    /* if(iround == 0) idir = 1; */
	    /* iround = iround + idir; */
	    /* ijk= ibase + iround; */
	    /* if( (ik >= 200) && (ik <= 208) ) ijk=300+ibase; */
	    
	    k = 0x8000+ ijk;        // make sure bit 15-12 is clear for the data
	    buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_test_ram_data)+((k & 0xffff)<<16); // load test data
	    i = pcie_send(hDev3, 1, 1, px);
	    send_array[is*256+ik]=ijk;           //load up data map
	  }
	}

	//vic
	//start copy
	
	#endif

	ichip =3;
	printf("\t==> Loading zero suppression parameter\n");
	imod=imod_fem;
      
	//vic set threshold to maximum value
	for (ik=0; ik< 64; ik++) {
	  ibase =ik+1;
	  //ijk=ik+10;     // threshold
	  //ijk = 3;
	  ijk = ik + min__; // set the threshold from command line

	  ijk = 0x0; //  threshold of 0

	  buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_tpc_load_threshold+ik)+((ijk & 0xffff)<< 16); // load threshold       
	  i = pcie_send(hDev3, 1, 1, px);
	  usleep(10);
	}

	ijk=2;
	buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_tpc_load_presample)+((ijk & 0xffff)<< 16); // load preample
	i = pcie_send(hDev3, 1, 1, px);
	usleep(10);

	ijk=3;     //was 4
	buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_tpc_load_postsample)+((ijk & 0xffff)<< 16); // load postsample
	i = pcie_send(hDev3, 1, 1, px);
	usleep(10);

	//common channel threshold
      
	buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_tpc_sel_comthres)+((1 & 0xffff)<< 16); // channel threshold
	i = pcie_send(hDev3, 1, 1, px);
	usleep(10);

	//
	buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_tpc_sel_bipolar)+((0 & 0xffff)<< 16); // no biploar
	i = pcie_send(hDev3, 1, 1, px);
	usleep(10);
	//
	//
	//	ijk=10;
	ijk=1000; // increased to accept any baseline
	buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_tpc_load_thr_mean)+((ijk & 0xffff)<< 16); // load preample
	i = pcie_send(hDev3, 1, 1, px);
	usleep(10);

	//ijk=100;
	ijk=10000; // increased to accept any baseline
	buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_tpc_load_thr_vari)+((ijk & 0xffff)<< 16); // load preample
	i = pcie_send(hDev3, 1, 1, px);
	usleep(10);
	
	
#if 0
	
	//vic
	//end copy
	//      imod=11;
	ichip=3;
	if(ihuff == 1) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_nocomp+(0x0<<16);  // turn the compression
	else buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_nocomp+(0x1<<16);  // set b channel no compression
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);

	//         timesize =4;
	//         imod=11;
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_timesize+(timesize<<16);  // set drift time size
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);

	a_id =0xf;
	//         imod=11;
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_id+(a_id<<16);  // set a_id
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);

	//       imod=11;
	ichip=4;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_id+(a_id<<16);  // set b_id
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
	//
	//     set max word in the pre-buffer memory
	//
	ik=8000;
	//         imod=11;
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_max+(ik<<16);  // set pre-buffer max word
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
	//
	//     enable hold
	//
	//         imod=11;
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_hold_enable+(0x1<<16);  // enable the hold
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);


	//       imod=11;
	//       ichip=3;
	//       buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_a_test+(0x1<<16);    // enable a test on
	//       i=1;
	//       k=1;
	//       if(islow_read == 1) i = pcie_send(hDev3, i, k, px);


	//       imod=11;
	if(imod == imod_st) {
	  printf(" set last module on, module address %d\n", imod);
	  ichip=4;
	  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_lst_on+(0x0<<16);    // set last module on
	  i=1;
	  k=1;
	  i = pcie_send(hDev3, i, k, px);
	}
	else {
	  printf(" set last module off, module address %d\n", imod);
	  ichip=4;
	  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_lst_off+(0x0<<16);    // set last module on
	  i=1;
	  k=1;
	  i = pcie_send(hDev3, i, k, px);
	}
      }

      #endif
      //
      //     now reset all the link port receiver PLL
      //
      for (imod_fem = (imod_st-1); imod_fem > imod_xmit; imod_fem--) {
	imod=imod_fem;
	printf(" reset the link PLL for module %x \n", imod);
	ichip=4;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_pll_reset+(0x0<<16);    // reset LINKIN PLL
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
	usleep(1000);   // give PLL time to reset
      }
      //
      //
      //
      for (imod_fem = (imod_xmit+1); imod_fem< (imod_st+1); imod_fem++) {
	i = pcie_rec(hDev3,0,1,nword,iprint,py);     // init the receiver
	imod=imod_fem;
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+20+(0x0<<16);  // read out status
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
	py = &read_array;
	i = pcie_rec(hDev3,0,2,nword,iprint,py);     // read out 2 32 bits words
	if(iprint == 1) printf("FEM module %d status word after PLL reset = %x, %x \n", imod, read_array[0], read_array[1]);
      }

      //
      //     now reset all the link port receiver
      //
      for (imod_fem = (imod_st-1); imod_fem > imod_xmit; imod_fem--) {
	imod=imod_fem;
	printf(" reset the link for module %d \n", imod);
	ichip=4;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_rxreset+(0x0<<16);    // reset LINKIN DPA
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
	//
	//
	ichip=4;
	buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_align+(0x0<<16);    // send alignment command
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
      }

      //
      //     set up xmit module  -- module count
      //
      imod=imod_xmit;
      ichip=3;
      //                  -- number of FEM module -1, counting start at 0
      buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_modcount+((imod_st-imod_xmit-1)<<16);
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      //
      //     rest optical
      //
      imod=imod_xmit;
      ichip=3;
      buf_send[0]=(imod<<11)+(ichip<<8)+mb_opt_dig_reset+(0x1<<16);  // set optical reset on
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      //
      //     enable Neutrino/superNova Token Passing
      //
      printf(" trigger type = %d \n", itrig_type);
      imod=imod_xmit;
      ichip=3;
      
      //
      // I WANT BOTH DMA TO BE ON WHY IS IT FAILING ONLY HERE
      //

      /* if((itrig_type & 0x1) == 1) buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_enable_1+(0x1<<16);  // enable token 1 pass */
      /* else buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_enable_1+(0x0<<16);  // disable token 1 pass */
      //
      /* if((itrig_type & 0x2) == 2) buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_enable_2+(0x1<<16);  // enable token 2 pass */
      /* else buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_enable_2+(0x0<<16);  // enable token 2 pass */

      
      //enable neutrino stream
      buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_enable_1+(0x1<<16);
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);

      //enable supernova stream
      buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_enable_2+(0x1<<16);
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);

      //
      //
      //       reset XMIT LINK PLL IN DPA
      //
      imod=imod_xmit;
      ichip=3;
      buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_link_pll_reset+(0x1<<16);  //  reset XMIT LINK PLL
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      usleep(1000);
      //
      //     reset XMIT LINK IN DPA
      //
      imod=imod_xmit;
      ichip=3;
      buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_link_reset+(0x1<<16);  //  reset XMIT LINK IN DPA
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      //
      //     wait for 10ms just in case
      //
      usleep(10000);
      printf(" XMIT FIFO reset \n");
      //
      //     reset XMIT FIFO reset
      //
      imod=imod_xmit;
      ichip=3;
      buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_dpa_fifo_reset+(0x1<<16);  //  reset XMIT LINK IN DPA
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      //
      //
      //
      is = 0;
      //
      //      test re-align circuit
      //
      imod=imod_xmit;
      ichip=3;
      buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_dpa_word_align+(0x1<<16);  //  send alignment pulse
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);


      usleep(5000); //wait for 5 ms
      printf(" XMIT re-align done \n");
      //
      nword = 1;

      i = pcie_rec(hDev3,0,1,nword,iprint,py);     // init the receiver

      //         imod=11;
      ichip=3;
      buf_send[0]=(imod_xmit<<11)+(ichip<<8)+mb_xmit_rdstatus+(0x0<<16);  // read out status

      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      py = &read_array;
      i = pcie_rec(hDev3,0,2,nword,iprint,py);     // read out 2 32 bits words
      printf("xmit status word = %x, %x \n", read_array[0], read_array[1]);


      for (imod_fem = (imod_xmit+1); imod_fem< (imod_st+1); imod_fem++) {
	i = pcie_rec(hDev3,0,1,nword,iprint,py);     // init the receiver
	imod=imod_fem;
	ichip=3;
	buf_send[0]=(imod<<11)+(ichip<<8)+20+(0x0<<16);  // read out status
	i=1;
	k=1;
	i = pcie_send(hDev3, i, k, px);
	py = &read_array;
	i = pcie_rec(hDev3,0,2,nword,iprint,py);     // read out 2 32 bits words
	if(iprint == 1) printf("FEM module %d status word = %x, %x \n", imod, read_array[0], read_array[1]);
      }


      nword = (((64*(iframe_length+1)/8)/2+5)*(imod_st-imod_xmit))+2;  //total number of 32 bits word
      //       nword = nword-20;
      printf("\t==> event length is %d \n", nword);
      last_dma_loop_size = (nword*4) % dwDMABufSize;          // last dma loop size
      ndma_loop = (nword*4)/dwDMABufSize;
      printf(" DMA will run %d loop per events \n", (ndma_loop+1));
      
//
      //     allocate SuperNova buffer 1
      //
      printf(" buffer allocation --- SuperNova 1\n");
      dwStatus = WDC_DMAContigBufLock(hDev5, &pbuf_rec_s1, dwOptions_rec, dwDMABufSize, &pDma_rec_s1);
      if (WD_STATUS_SUCCESS != dwStatus) {
	printf("Failed locking a SuperNova Receive Contiguous DMA buffer 1. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
	printf("enter 1 to continue \n");
	scanf("%d",&is);
      }
      else {
	u32Data = pDma_rec_s1->Page->pPhysicalAddr & 0xffffffff;
	printf(" buffer allocation s 1 lower address = %x\n", u32Data);
	u32Data = (pDma_rec_s1->Page->pPhysicalAddr >> 32) & 0xffffffff;
	printf(" buffer allocation s 1 higher address = %x\n", u32Data);
      }
      //
      //     allocate SuperNova buffer 2
      //
      printf(" buffer allocation --- SuperNova 2\n");
      dwStatus = WDC_DMAContigBufLock(hDev5, &pbuf_rec_s2, dwOptions_rec, dwDMABufSize, &pDma_rec_s2);
      if (WD_STATUS_SUCCESS != dwStatus) {
	printf("Failed locking a SuperNova Receive Contiguous DMA buffer 2. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
	printf("enter 1 to continue \n");
	scanf("%d",&is);
      }
      else {
	u32Data = pDma_rec_s2->Page->pPhysicalAddr & 0xffffffff;
	printf(" buffer allocation s 2 lower address = %x\n", u32Data);
	u32Data = (pDma_rec_s2->Page->pPhysicalAddr >> 32) & 0xffffffff;
	printf(" buffer allocation s 2 higher address = %x\n", u32Data);
      }
      //
      //     allocate SuperNova buffer 3
      //
      printf(" buffer allocation --- SuperNova 3\n");
      dwStatus = WDC_DMAContigBufLock(hDev5, &pbuf_rec_s3, dwOptions_rec, dwDMABufSize, &pDma_rec_s3);
      if (WD_STATUS_SUCCESS != dwStatus) {
	printf("Failed locking a SuperNova Receive Contiguous DMA buffer 3. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
	printf("enter 1 to continue \n");
	scanf("%d",&is);
      }
      else {
	u32Data = pDma_rec_s3->Page->pPhysicalAddr & 0xffffffff;
	printf(" buffer allocation s 3 lower address = %x\n", u32Data);
	u32Data = (pDma_rec_s3->Page->pPhysicalAddr >> 32) & 0xffffffff;
	printf(" buffer allocation s 3 higher address = %x\n", u32Data);
      }
      //
      //     allocate SuperNova buffer 4
      //
      printf(" buffer allocation --- SuperNova 4\n");
      dwStatus = WDC_DMAContigBufLock(hDev5, &pbuf_rec_s4, dwOptions_rec, dwDMABufSize, &pDma_rec_s4);
      if (WD_STATUS_SUCCESS != dwStatus) {
	printf("Failed locking a SuperNova Receive Contiguous DMA buffer 4. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
	printf("enter 1 to continue \n");
	scanf("%d",&is);
      }
      else {
	u32Data = pDma_rec_s4->Page->pPhysicalAddr & 0xffffffff;
	printf(" buffer allocation s 4 lower address = %x\n", u32Data);
	u32Data = (pDma_rec_s4->Page->pPhysicalAddr >> 32) & 0xffffffff;
	printf(" buffer allocation s 4 higher address = %x\n", u32Data);
      }

      //
      //     set up the SuperNova data stream PCIe mode and reset DMA
      //
      /* set tx mode register */
      u32Data = 0x00002000;//GOGOGOGOGOGO
      dwOffset = tx_md_reg;
      dwAddrSpace =cs_bar;
      WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);
      /* write this will abort previous DMA */
      dwAddrSpace =2;
      dwOffset = cs_dma_msi_abort;
      u32Data = dma_abort;
      WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);
      /* clear DMA register after the abort */
      dwAddrSpace =2;
      dwOffset = cs_dma_msi_abort;
      u32Data = 0;
      WDC_WriteAddr32(hDev5, dwAddrSpace, dwOffset, u32Data);
      printf(" initial abort finished \n");

      //sn
      rc_pt = pthread_create(&threads[0], &attr_pt_sn, pt_sn_filewrite, (void *)nword_n);
      if(rc_pt) printf("ERROR; return code from pthread_create() for sn_filewrite is %d\n", rc_pt);
      
      //sn copy thread
      rc_pt = pthread_create(&threads[0], &attr_pt_sn_cp, pt_sn_copythread,(void *)nword_n);
      if(rc_pt) printf("ERROR; return code from pthread_create() for sn_copythread is %d\n", rc_pt);
      

      thread_data_sn.id = 1;
      thread_data_sn.hdev = hDev5;
      thread_data_sn.hdevc = hDev3;
      printf("\nReceived event notification (device handle 0x%p): ", hDev);
      rc_pt = pthread_create(&threads[0], &attr_pt_sn_dma, pt_sn_dma, (void *)&thread_data_sn);
      if(rc_pt) printf("ERROR; return code from pthread_create() for pt_sn_dma is %d\n", rc_pt);

      
      if ( ineu == 1 ) {
      	//
      	//       allocate Neutrino Buffer 1
      	//
      	printf(" buffer allocation --- Neutrino 1\n");
      	dwStatus = WDC_DMAContigBufLock(hDev4, &pbuf_rec_n1, dwOptions_rec, dwDMABufSize, &pDma_rec_n1);
      	if (WD_STATUS_SUCCESS != dwStatus) {
      	  printf("Failed locking a Neutrino Rec Contiguous DMA buffer 1. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
      	  printf("enter 1 to continue \n");
      	  scanf("%d",&is);
      	}
      	else {
      	  u32Data = pDma_rec_n1->Page->pPhysicalAddr & 0xffffffff;
      	  printf(" buffer allocation 1 lower address = %x\n", u32Data);
      	  u32Data = (pDma_rec_n1->Page->pPhysicalAddr >> 32) & 0xffffffff;
      	  printf(" buffer allocation 1 higher address = %x\n", u32Data);
      	}
      	//
      	//        allocate Neutrino buffer 2
      	//
      	printf(" buffer allocation --- Neutrino 2\n");
      	dwStatus = WDC_DMAContigBufLock(hDev4, &pbuf_rec_n2, dwOptions_rec, dwDMABufSize, &pDma_rec_n2);
      	if (WD_STATUS_SUCCESS != dwStatus) {
      	  printf("Failed locking a Neutrino Rec Contiguous DMA buffer 2. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
      	  printf("enter 1 to continue \n");
      	  scanf("%d",&is);
      	}
      	else {
      	  u32Data = pDma_rec_n2->Page->pPhysicalAddr & 0xffffffff;
      	  printf(" buffer allocation lower address = %x\n", u32Data);
      	  u32Data = (pDma_rec_n2->Page->pPhysicalAddr >> 32) & 0xffffffff;
      	  printf(" buffer allocation higher address = %x\n", u32Data);
      	}
      	//
      	//      set up the Neutrino data stream PCIe mode and reset DMA
      	//

      	//set tx mode register
      	u32Data = 0x00002000;
      	dwOffset = tx_md_reg;
      	dwAddrSpace =cs_bar;
      	WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);
      	/* write this will abort previous DMA */
      	dwAddrSpace =2;
      	dwOffset = cs_dma_msi_abort;
      	u32Data = dma_abort;
      	WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);
      	/* clear DMA register after the abort */
      	dwAddrSpace =2;
      	dwOffset = cs_dma_msi_abort;
      	u32Data = 0;
      	WDC_WriteAddr32(hDev4, dwAddrSpace, dwOffset, u32Data);
      	printf(" initial abort finished \n");


      	thread_data_n.id = 0;
      	thread_data_n.hdev  = hDev4;
      	thread_data_n.hdevc = hDev3;
      	rc_pt = pthread_create(&threads[0], &attr_pt_trig_dma, pt_trig_dma, (void *)&thread_data_n);
      	if(rc_pt) printf("ERROR; return code from pthread_create() for pt_trig_DMA is %d\n", rc_pt);
      
      	//file write and copy threads
      	//neutrino
      	rc_pt = pthread_create(&threads[0], &attr_pt_tr, pt_trig_filewrite, (void *)nword_n);
      	if(rc_pt) printf("ERROR; return code from pthread_create() for trig_filewrite is %d\n", rc_pt);
      
      	//neu copy thread
      	rc_pt = pthread_create(&threads[0], &attr_pt_tr_cp, pt_trig_copythread,(void *)nword_n);
      	if(rc_pt) printf("ERROR; return code from pthread_create() for trig_copythread is %d\n", rc_pt);
	
      }
      
      usleep(1000);
      ifr = 0;
	
      printf("\t==>Start-ing the run in 2 seconds!\n");
      usleep(2000000);
      ifr =1;
      imod=0;
      ichip=1;
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_cntrl_set_run_on)+(0x0<<16); //enable offline run on
      printf("\t==>Start-ed the run...\n");
      i=1;
      k=1;
      i = pcie_send(hDev3, i, k, px);
      
      

      //start the clock
      
      /** begin timer **/

      gettimeofday(&starttest1,NULL);
      seconds1  = starttest1.tv_sec;
      useconds1 = starttest1.tv_usec;

      printf("\n\nStart time of test: %ld sec %ld usec\n",seconds1,useconds1);
      printf("\n\nWill let you know when you crash!!\n");
      //get a fresh timestamp
      gettimeofday(&starttest1,NULL);

      
 
      while(1) {
      	usleep(2000000);
      	//trigger block
      	printf("\e[1;35m\t(main) ==> Sending trigger !\n\e[0m");
      	imod=0;
      	ichip=1;
      	buf_send[0]=(imod<<11)+(ichip<<8)+mb_cntrl_set_trig1+(0x0<<16);  // send trigger
      	i=1;
      	k=1;
      	i = pcie_send(hDev, i, k, px);
      	//end trigger block
      	printf("\e[1;35m\t(main) ==> \e[1;33m NOT Waiting\e[1;35m a bit for NU!\n\e[0m");
      	//printf("\e[1;35m\t(main) ==> \e[1;33mDONE...\e[1;35m waiting a bit for NU!)\e[0m\n");
      }

      usleep(10000000000000); //30 minutes?
    }
    break;
  }
}



void *pt_sn_dma(void *threadarg)
{

#include "wdc_defs.h"
#define  t1_tr_bar 0
#define  t2_tr_bar 4
#define  cs_bar 2

  /**  command register location **/

#define  tx_mode_reg 0x28
#define  t1_cs_reg 0x18
#define  r1_cs_reg 0x1c
#define  t2_cs_reg 0x20
#define  r2_cs_reg 0x24

#define  tx_md_reg 0x28

#define  cs_dma_add_low_reg 0x0
#define  cs_dma_add_high_reg  0x4
#define  cs_dma_by_cnt 0x8
#define  cs_dma_cntrl 0xc
#define  cs_dma_msi_abort 0x10

  /** define status bits **/

#define  cs_init  0x20000000
#define  cs_mode_p 0x8000000
#define  cs_mode_n 0x0
#define  cs_start 0x40000000
#define  cs_done  0x80000000

#define  dma_tr1  0x100000
#define  dma_tr2  0x200000
#define  dma_tr12 0x300000
#define  dma_3dw_trans 0x0
#define  dma_4dw_trans 0x0
#define  dma_3dw_rec   0x40
#define  dma_4dw_rec   0x60
#define  dma_in_progress 0x80000000

#define  dma_abort 0x2

#define  mb_cntrl_set_trig1 0x4

  static DWORD dwAddrSpace;
  static UINT32 u32Data, u32Data_h;
  static DWORD dwOffset;

  static int ifr,is,iused,nwrite,nwrite_byte_s, taskid,ch;
  static int icopy, idebug;
  static int idone,tr_bar,t_cs_reg,r_cs_reg,dma_tr;
  static int dis, ik;
  static int i,k,imod,ichip;


  UINT32 *px;
  UINT32 buf_send[40];

  WDC_DEVICE_HANDLE hDev,hDev2;

  struct thread_data
  {
    int thread_id;
    WDC_DEVICE_HANDLE hDev;
    WDC_DEVICE_HANDLE hDevc;
  };

  struct thread_data *my_data;

  //xxx
  dwDMABufSize = 200000;
  px = &buf_send;

  nwrite_byte_s = dwDMABufSize;
  nwrite        = nwrite_byte_s/4;

  my_data = (struct thread_data *) threadarg;

  taskid = my_data->thread_id;
  hDev2  = my_data->hDev;
  hDev   = my_data->hDevc;

  idebug =0;
  ifr = 0;

  for (is=0; is< 4; is++)  {
    sn_buf_filled[is] = 0;
  }

  total_used_s = 0;
  iused = 0;
  if(VDEBUG) printf("\e[1;34m\n(pt_sn_dma) ==> dma thread started\e[0m\n");

  while (1) {

    while(total_used_s == 4) {
      usleep(1000);//wait a millisecond if they are all full
      continue;
    }

    /* //trigger block */
    /* printf("\t(main) ==> Sent trigger)\n"); */
    /* imod=0; */
    /* ichip=1; */
    /* buf_send[0]=(imod<<11)+(ichip<<8)+mb_cntrl_set_trig1+(0x0<<16);  // send trigger */
    /* i=1; */
    /* k=1; */
    /* i = pcie_send(hDev, i, k, px); */
    /* //end trigger block */
    /* usleep(1000000); */
    /* continue; */


    // two transceivers
    for (is=1; is<3; is++) {
      tr_bar   = t1_tr_bar;
      r_cs_reg = r1_cs_reg;
      dma_tr   = dma_tr1;
      if(is == 2) {
        tr_bar   = t2_tr_bar;
        r_cs_reg = r2_cs_reg;
        dma_tr   = dma_tr2;
      }

      /** initialize the receiver ***/
      u32Data     = cs_init;
      dwOffset    = r_cs_reg;
      dwAddrSpace = cs_bar;
      
      // receiver only get initialize for the 1st time -->gives problem to neutrino trigger...
      if(ifr == 0) {
	WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
      }
      /** start the receiver **/
      dwAddrSpace = cs_bar;
      u32Data     = cs_start+nwrite_byte_s;
      dwOffset    = r_cs_reg;
      WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
    }
    ifr = 1;

    //
    //
    /** set up DMA for both transceiver together **/ //lie
    if(VDEBUG) printf("(pt_sn_dma) ==> iused: %d\n",iused);
    if(sn_buf_filled[iused] == 0) {
      if(VDEBUG) printf("(pt_sn_dma) ==> unused!\n");
      dwAddrSpace =cs_bar;
      dwOffset = cs_dma_add_low_reg;
      if(iused ==0)       u32Data = pDma_rec_s1->Page->pPhysicalAddr & 0xffffffff;
      else if(iused ==1)  u32Data = pDma_rec_s2->Page->pPhysicalAddr & 0xffffffff;
      else if(iused ==2)  u32Data = pDma_rec_s3->Page->pPhysicalAddr & 0xffffffff;
      else if(iused ==3)  u32Data = pDma_rec_s4->Page->pPhysicalAddr & 0xffffffff;
      
      WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

      dwAddrSpace =cs_bar;
      dwOffset = cs_dma_add_high_reg;
      if(iused ==0)       u32Data_h = (pDma_rec_s1->Page->pPhysicalAddr >>32) & 0xffffffff;
      else if(iused ==1)  u32Data_h = (pDma_rec_s2->Page->pPhysicalAddr >>32) & 0xffffffff;
      else if(iused ==2)  u32Data_h = (pDma_rec_s3->Page->pPhysicalAddr >>32) & 0xffffffff;
      else if(iused ==3)  u32Data_h = (pDma_rec_s4->Page->pPhysicalAddr >>32) & 0xffffffff;
      WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data_h);

      if(iused ==0)       WDC_DMASyncCpu(pDma_rec_s1);
      else if(iused ==1)  WDC_DMASyncCpu(pDma_rec_s2);
      else if(iused ==2)  WDC_DMASyncCpu(pDma_rec_s3);
      else if(iused ==3)  WDC_DMASyncCpu(pDma_rec_s4);
      
      /* byte count */
      dwAddrSpace = cs_bar;
      dwOffset    = cs_dma_by_cnt;
      u32Data     = nwrite_byte_s; //(*2 fibers?)
      WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

      /* write this will start DMA */
      dwAddrSpace =2;
      dwOffset   =cs_dma_cntrl;
      
      if(u32Data_h == 0) {
        u32Data = dma_tr12+dma_3dw_rec;
      }
      else {
        u32Data = dma_tr12+dma_4dw_rec;
      }

      //This starts DMA
      if(VDEBUG) printf("(pt_sn_dma) ==> Start DMA \n");      
      WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
      if(VDEBUG) printf("(pt_sn_dma) ==> Started DMA \n");      

      if(VDEBUG) printf("(pt_sn_dma) ==> total_used: %d\n",total_used_s);
    }
    else {  // this one was already used, go around the horn
      if     (iused == 0) iused=1;
      else if(iused == 1) iused=2;
      else if(iused == 2) iused=3;
      else if(iused == 3) iused=0;
      continue;
    }
    //
    //
    //
    

    //god
    dwAddrSpace =2;
    u32Data =0;
    dwOffset = 0x1c;
    WDC_ReadAddr32(hDev2, dwAddrSpace, dwOffset, &u32Data);
    if(VDEBUG) printf ("(pt_sn_dma) ==> status R1 word before read = %x ~ ",u32Data);
    dwAddrSpace =2;
    u32Data =0;
    dwOffset = 0x24;
    WDC_ReadAddr32(hDev2, dwAddrSpace, dwOffset, &u32Data);
    if(VDEBUG) printf ("status R2 word before read = %x\n\n",u32Data);
    //god

    idone=0;
    ch = 0;
    while (idone == 0) {
      ch += 1;
      
      if (ch%10000000 == 0){ 
	

	// small delay here BUT should be accurate ``enough"
	gettimeofday(&endtest1,NULL);
	long mytime2, seconds2, useconds2;
	seconds2  = endtest1.tv_sec;
	useconds2 = endtest1.tv_usec;
	
	printf("\n\n\tTime elapsed: %ld sec %ld usec\n",seconds2-seconds1,useconds2-useconds1);
	//get a fresh timestamp
	
	printf("(pt_sn_dma) ==> receive DMA status word %d %X \n",ch,u32Data);
	//god
	dwAddrSpace =2;
	u32Data =0;
	dwOffset = 0x1c;
	WDC_ReadAddr32(hDev2, dwAddrSpace, dwOffset, &u32Data);
	printf ("(pt_sn_dma) ==> status R1 word DMA = %x ~ ",u32Data);
	dwAddrSpace =2;
	u32Data =0;
	dwOffset = 0x24;
	WDC_ReadAddr32(hDev2, dwAddrSpace, dwOffset, &u32Data);
	printf ("status R2 word DMA = %x\n\n",u32Data);
	//god

	exit(0);
      }

      dwAddrSpace = cs_bar;
      dwOffset    = cs_dma_cntrl;
      WDC_ReadAddr32(hDev2, dwAddrSpace, dwOffset, &u32Data);


      if((u32Data & dma_in_progress) == 0) {
      	
	idone =1;
	
	sn_buf_filled[iused] = 1;
	total_used_s++;
      	
	if     (iused == 0) { WDC_DMASyncIo(pDma_rec_s1); iused=1; }
	else if(iused == 1) { WDC_DMASyncIo(pDma_rec_s2); iused=2; }
	else if(iused == 2) { WDC_DMASyncIo(pDma_rec_s3); iused=3; }
	else if(iused == 3) { WDC_DMASyncIo(pDma_rec_s4); iused=0; }
        
      }
    }
    
    
    ch = 0;
    if     (iused == 0) ch=3;
    else if(iused == 1) ch=0;
    else if(iused == 2) ch=1;
    else if(iused == 3) ch=2;

    
    printf("\e[1;32m\n DMA done --\e[1;34mSN on buffer: %d \n\e[0m",ch);

  }

}


void *pt_trig_dma(void *threadarg)
{

#include "wdc_defs.h"
#define  t1_tr_bar 0
#define  t2_tr_bar 4
#define  cs_bar 2

  /**  command register location **/

#define  tx_mode_reg 0x28
#define  t1_cs_reg 0x18
#define  r1_cs_reg 0x1c
#define  t2_cs_reg 0x20
#define  r2_cs_reg 0x24

#define  tx_md_reg 0x28

#define  cs_dma_add_low_reg 0x0
#define  cs_dma_add_high_reg  0x4
#define  cs_dma_by_cnt 0x8
#define  cs_dma_cntrl 0xc
#define  cs_dma_msi_abort 0x10

  /** define status bits **/

#define  cs_init  0x20000000
#define  cs_mode_p 0x8000000
#define  cs_mode_n 0x0
#define  cs_start 0x40000000
#define  cs_done  0x80000000

#define  dma_tr1  0x100000
#define  dma_tr2  0x200000
#define  dma_tr12 0x300000
#define  dma_3dw_trans 0x0
#define  dma_4dw_trans 0x0
#define  dma_3dw_rec   0x40
#define  dma_4dw_rec   0x60
#define  dma_in_progress 0x80000000

#define  dma_abort 0x2

  static DWORD dwAddrSpace;
  static UINT32 u32Data, u32Data_h;
  static DWORD dwOffset;

  static int ifr,is,iused,nwrite,nwrite_byte_n, taskid;
  static int icopy, idebug, read_point_n_tmp,ch;
  static int idone,tr_bar,t_cs_reg,r_cs_reg,dma_tr;
  static int dis, ik;
  static UINT32 *buffp_rec32_n;

  WDC_DEVICE_HANDLE hDev,hDev1;

  struct thread_data
  {
    int thread_id;
    WDC_DEVICE_HANDLE hDev;
    WDC_DEVICE_HANDLE hDevc;
  };

  struct thread_data *my_data;

  //xx
  dwDMABufSize = 200000;
  //
  //
  //
  if(VDEBUG) printf("\e[1;31m\t==> trigger thread started\e[0\n");
  my_data = (struct thread_data *) threadarg;
  taskid = my_data->thread_id;
  hDev1  = my_data->hDev;
  hDev   = my_data->hDevc;
  //
  //
  //
  idebug =0;
  ifr = 0;
  for (is=0; is< 2; is++) {
    neu_buf_filled[is] = 0;
  }
  iused =0;
  nwrite_byte_n = dwDMABufSize;  //0x30d40
  nwrite = nwrite_byte_n/4;
  write_point_n = 0;
  read_point_n = 0;
  total_used_n = 0;
  
  while (1) {
    if(VDEBUG) printf("\ntotal_used_n == %d\n",total_used_n);
    while(total_used_n == 2) {
      usleep(1000);//wait a millisecond if they are all full
      continue;
    }

    for (is=1; is<3; is++) {
      tr_bar = t1_tr_bar;
      r_cs_reg = r1_cs_reg;
      dma_tr = dma_tr1;
      if(is == 2) {
        tr_bar = t2_tr_bar;
        r_cs_reg = r2_cs_reg;
        dma_tr = dma_tr2;
      }

      /** initialize the receiver ***/
      u32Data = cs_init;
      dwOffset = r_cs_reg;
      dwAddrSpace =cs_bar;
      //
      // rreceiver only get initialize for the 1st time
      //
      if(ifr ==0) {
        /* printf(" initialize the input fifo -- Tr\n"); */
        WDC_WriteAddr32(hDev1, dwAddrSpace, dwOffset, u32Data);
      }
      /** start the receiver **/
      dwAddrSpace = cs_bar;
      u32Data = cs_start+nwrite_byte_n;   /* 32 bits mode == 4 bytes per word *2 fibers **/
      /* if(idebug == 1) printf(" trigger -- DMA loop with DMA data length %d \n", nwrite_byte_n); */
      dwOffset = r_cs_reg;
      WDC_WriteAddr32(hDev1, dwAddrSpace, dwOffset, u32Data);

    }
    ifr=1;
    
    if(VDEBUG)    printf("(pt_trig_dma) ==> iused: %d\n",iused);
    if(neu_buf_filled[iused] == 0) {
      dwAddrSpace =cs_bar;
      dwOffset = cs_dma_add_low_reg;
      if(iused ==0)     u32Data = pDma_rec_n1->Page->pPhysicalAddr & 0xffffffff;
      else if(iused==1) u32Data = pDma_rec_n2->Page->pPhysicalAddr & 0xffffffff;
      
      WDC_WriteAddr32(hDev1, dwAddrSpace, dwOffset, u32Data);

      dwAddrSpace =cs_bar;
      dwOffset = cs_dma_add_high_reg;
      if(iused==0)      u32Data_h = (pDma_rec_n1->Page->pPhysicalAddr >>32) & 0xffffffff;
      else if(iused==1) u32Data_h = (pDma_rec_n2->Page->pPhysicalAddr >> 32) & 0xffffffff;
      WDC_WriteAddr32(hDev1, dwAddrSpace, dwOffset, u32Data_h);

      
      if(iused==0)      WDC_DMASyncCpu(pDma_rec_n1);
      else if(iused==1) WDC_DMASyncCpu(pDma_rec_n2);
      
      /* byte count */
      dwAddrSpace =cs_bar;
      dwOffset = cs_dma_by_cnt;
      //       u32Data = (nwrite)*4*2;      /** twice more data - from fiber 1& 2**/
      u32Data = nwrite_byte_n;
      WDC_WriteAddr32(hDev1, dwAddrSpace, dwOffset, u32Data);


      /* write this will start DMA */
      dwAddrSpace = 2;
      dwOffset    = cs_dma_cntrl;

      if(u32Data_h == 0) {
        u32Data = dma_tr12+dma_3dw_rec;
      }
      else {
        u32Data = dma_tr12+dma_4dw_rec;
      }
      WDC_WriteAddr32(hDev1, dwAddrSpace, dwOffset, u32Data);
      
    }
    else {
      if      (iused==0) iused=1;
      else if (iused==1) iused=0;
    }

    idone =0;
    ch    =0;
    
    //god
    dwAddrSpace =2;
    u32Data =0;
    dwOffset = 0x1c;
    WDC_ReadAddr32(hDev1, dwAddrSpace, dwOffset, &u32Data);
    if(VDEBUG) printf ("(pt_trig_dma) ==> status R1 word before read = %x ~ ",u32Data);
    dwAddrSpace =2;
    u32Data =0;
    dwOffset = 0x24;
    WDC_ReadAddr32(hDev1, dwAddrSpace, dwOffset, &u32Data);
    if(VDEBUG) printf ("status R2 word before read = %x\n\n",u32Data);
    //god

       
    while (idone == 0) {
      
      ch+=1;
      

      if (ch%1000000 == 0){
	printf("(pt_trig_dma) ==> receive DMA status word %d %X \n",ch,u32Data);
	//god
	dwAddrSpace =2;
	u32Data =0;
	dwOffset = 0x1c;
	WDC_ReadAddr32(hDev1, dwAddrSpace, dwOffset, &u32Data);
	printf ("(pt_trig_dma) ==> status R1 word DMA = %x ~ ",u32Data);
	dwAddrSpace =2;
	u32Data =0;
	dwOffset = 0x24;
	WDC_ReadAddr32(hDev1, dwAddrSpace, dwOffset, &u32Data);
	printf ("status R2 word DMA = %x\n\n",u32Data);
	//god
    
      }
      dwAddrSpace = cs_bar;
      dwOffset    = cs_dma_cntrl;
      WDC_ReadAddr32(hDev1, dwAddrSpace, dwOffset, &u32Data);

      if((u32Data & dma_in_progress) == 0) {
	idone =1;
	  
	neu_buf_filled[iused] = 1;

	total_used_n++;
	if     (iused == 0) { WDC_DMASyncIo(pDma_rec_n1); iused=1; }
	else if(iused == 1) { WDC_DMASyncIo(pDma_rec_n2); iused=0; }

      }
    }
    
    
    idone = 0;
    if     (iused == 0) idone=1;
    else if(iused == 1) idone=0;
    
    printf("\e[1;32m\n DMA done --\e[1;31m Neu on buffer: %d \e[0m\n",idone);
    /* usleep(2000000); */
  }

}




void *pt_trig_filewrite(void *nword_write)
{
  static int file_buf[250000];
  static int w_t1,r_t1,nwrite,dis,is, index, n_write;
  static int read_point_tmp;
  static UINT32 send_array[2];

  //xxx
  dwDMABufSize = 200000;
  if(VDEBUG) printf("\e[1;31m{pt_trig_filewrite} ==> trig file write thread started\e[0m\n");
  while (1) {
    w_t1 = write_point_n;
    r_t1 = read_point_n;
    nwrite = dwDMABufSize/4;
    dis =w_t1 - r_t1;
    while (dis< nwrite) {
      w_t1 = write_point_n;
      r_t1 = read_point_n;
      dis =w_t1 - r_t1;
      if (dis < 0) dis = jbuf_ev_size + dis;
      usleep(5); // sleep less time
    }

    if((w_t1 > r_t1) | ((jbuf_ev_size -r_t1)>nwrite)) {
      for (is=0; is<nwrite; is++) {
	file_buf[is] = buffer_ev_n[is+r_t1];
      }
      read_point_tmp = read_point_n+nwrite;
    }
    else {
      for (is=0; is<(jbuf_ev_size-r_t1); is++) {
	file_buf[is] = buffer_ev_n[is+r_t1];
      }
      index =0;
      for (is=(jbuf_ev_size-r_t1); is< (nwrite- (jbuf_ev_size-r_t1)); is++) {
	file_buf[is] = buffer_ev_n[index];
	index = index+1;
      }
      //read_point_n = index;
      read_point_tmp = index;
    }
    send_array[0] = nwrite;
    n_write = write(fd_trig_pt,send_array,4);
    n_write = write(fd_trig_pt,file_buf,(nwrite*4));
    read_point_n = read_point_tmp;
    if(VDEBUG) printf("\e[1;31m{pt_trig_filewrite} ==> Nuetrino write point = %d, read point %d\e[0m\n", write_point_n, read_point_n);
  }
}

void *pt_sn_copythread(void *arg)
{

  static int ch;
  static int dis, ik;
  static int nwrite;
  static int nwrite_byte_s;
  static UINT32 *buffp_rec32_s;
  static int read_point_s_tmp;
  
  ch = 0;

  if(VDEBUG) printf("\e[1;34m[pt_sn_copythread] ==> Started\e[0m\n");
  
  nwrite_byte_s = dwDMABufSize;
  nwrite        = nwrite_byte_s/4;
  write_point_s = 0;
  read_point_s  = 0;

  while(1) {
    
    while(1) {
      
      if( sn_buf_filled[ch] == 0 ) { // race condition on this array at all?
	  ch += 1; 
	  ch = ch%4; 
	  usleep(100); // 1 microsecond wait until recheck
	  continue; 
	}
      
      if(VDEBUG) printf("\e[1;34m[pt_sn_copythread] ==> Buffer ch: %d is filled!\n\e[0m",ch);
      // buffer is filled!
      break;
    }
    
    if      (ch == 0) { buffp_rec32_s = pbuf_rec_s1; }
    else if (ch == 1) { buffp_rec32_s = pbuf_rec_s2; }
    else if (ch == 2) { buffp_rec32_s = pbuf_rec_s3; }
    else if (ch == 3) { buffp_rec32_s = pbuf_rec_s4; }
    
      //vic doesn't completely undetstand following commented code, lets 
      //read one out at a time?
    /* printf("[pt_sn_copythread] ==> check write and read point -- get lock\n"); */
    pthread_mutex_lock (&mutexlock);
    if(write_point_s >= read_point_s) dis =  jbuf_ev_size - (write_point_s - read_point_s);
    else dis = read_point_s - write_point_s;
    pthread_mutex_unlock (&mutexlock);
    /* printf("[pt_sn_copythread] ==> release lock\n");*/
  
    while (dis < nwrite) {
      usleep(100);
      pthread_mutex_lock (&mutexlock);
      /* printf("[pt_sn_copythread] ==> have to wait for space -- get lock\n"); */
      if(write_point_s >= read_point_s) dis =  jbuf_ev_size - (write_point_s - read_point_s);
      else dis = read_point_s - write_point_s;
      pthread_mutex_unlock (&mutexlock);
      /* printf("[pt_sn_copythread] ==> release lock\n"); */
    }
    
    if(VDEBUG) printf("\e[1;34m[pt_sn_copythread] ==> enter array copy --SN : ch: %d wps: %d rps: %d\e[0m\n", ch, write_point_s, read_point_s);
    read_point_s_tmp =  read_point_s;
    if(write_point_s >= read_point_s_tmp) {
      if((jbuf_ev_size - write_point_s) >= nwrite) {
    	for (ik=0; ik< nwrite; ik++) {
    	  buffer_ev_s[write_point_s+ik] = *buffp_rec32_s++;
    	}
    	write_point_s = write_point_s + nwrite;
      }
      else {
    	for (ik=0; ik< (jbuf_ev_size-write_point_s); ik++) {
    	  buffer_ev_s[write_point_s+ik] = *buffp_rec32_s++;
    	}
    	for (ik=0; ik< (nwrite-(jbuf_ev_size-write_point_s)); ik++) {
    	  buffer_ev_s[ik] = *buffp_rec32_s++;
    	}
    	write_point_s = write_point_s + nwrite- jbuf_ev_size;
      }
    }
    else {
      for (ik=0; ik< nwrite; ik++) {
    	buffer_ev_s[write_point_s+ik] = *buffp_rec32_s++;
      }
      write_point_s = write_point_s+ nwrite;
    }

    /* printf("[pt_sn_copythread] ==> finished copy\n"); */
    
    //whats in the buffer?
    /* printf("==> What is in the buffer? \n"); */
    /* for(ik=0;ik<nwrite*4;++ik){ */
      
    /*   printf("%08x ",buffer_ev_s[ ik ] ); */
    /*   if(ik%8==0)printf("\n"); */

    /* } */
    
    
    sn_buf_filled[ch] = 0;
    total_used_s -= 1;
    if(VDEBUG) printf("\e[1;34m[pt_sn_copythread] ==> total_used: %d\e[0m\n",total_used_s);
    //vic
  }  
}


void *pt_trig_copythread(void *arg)
{

  static int ch;
  static int dis, ik;
  static int nwrite;
  static int nwrite_byte_n;
  static UINT32 *buffp_rec32_n;
  static int read_point_n_tmp;
  
  ch = 0;

  if(VDEBUG) printf("\e[1;31m[pt_trig_copythread] ==> Started\e[0m\n");
  
  nwrite_byte_n = dwDMABufSize;
  nwrite        = nwrite_byte_n/4;
  write_point_n = 0;
  read_point_n  = 0;

  while(1) {
    
    while(1) {
      if( neu_buf_filled[ch] == 0 ) { // race condition on this array at all?
	  ch += 1; 
	  ch = ch%2; 
	  usleep(5); // 5 microsecond wait until recheck
	  continue; 
	}
      
      if(VDEBUG) printf("\e[1;31m[pt_trig_copythread] ==> Buffer ch: %d is filled!\e[0m\n",ch);
      // buffer is filled!
      break;
    }   
    
    if      (ch == 0) { buffp_rec32_n = pbuf_rec_n1; }
    else if (ch == 1) { buffp_rec32_n = pbuf_rec_n2; }
    /* else if (ch == 2) { buffp_rec32_n = pbuf_rec_n3; } */
    /* else if (ch == 3) { buffp_rec32_n = pbuf_rec_n4; } */
    
    //vic doesn't completely undetstand following commented code, lets 
    //read one out at a time?
    /* printf("[pt_sn_copythread] ==> check write and read point -- get lock\n"); */
    pthread_mutex_lock (&mutexlock);
    if(write_point_n >= read_point_n) dis =  jbuf_ev_size - (write_point_n - read_point_n);
    else dis = read_point_n - write_point_n;
    pthread_mutex_unlock (&mutexlock);
    /* printf("[pt_sn_copythread] ==> release lock\n");*/
  
    while (dis < nwrite) {
      usleep(100);
      pthread_mutex_lock (&mutexlock);
      /* printf("[pt_sn_copythread] ==> have to wait for space -- get lock\n"); */
      if(write_point_n >= read_point_n) dis =  jbuf_ev_size - (write_point_n - read_point_n);
      else dis = read_point_n - write_point_n;
      pthread_mutex_unlock (&mutexlock);
      /* printf("[pt_sn_copythread] ==> release lock\n"); */
    }
    
    if(VDEBUG) printf("\e[1;31m[pt_trig_copythread] ==> enter array copy --NU : ch: %d wps: %d rps: %d\n\e[0m", ch, write_point_n, read_point_n);
    read_point_n_tmp =  read_point_n;
    if(write_point_n >= read_point_n_tmp) {
      if((jbuf_ev_size - write_point_n) >= nwrite) {
    	for (ik=0; ik< nwrite; ik++) {
    	  buffer_ev_n[write_point_n+ik] = *buffp_rec32_n++;
    	}
    	write_point_n = write_point_n + nwrite;
      }
      else {
    	for (ik=0; ik< (jbuf_ev_size-write_point_n); ik++) {
    	  buffer_ev_n[write_point_n+ik] = *buffp_rec32_n++;
    	}
    	for (ik=0; ik< (nwrite-(jbuf_ev_size-write_point_n)); ik++) {
    	  buffer_ev_n[ik] = *buffp_rec32_n++;
    	}
    	write_point_n = write_point_n + nwrite- jbuf_ev_size;
      }
    }
    else {
      for (ik=0; ik< nwrite; ik++) {
    	buffer_ev_n[write_point_n+ik] = *buffp_rec32_n++;
      }
      write_point_n = write_point_n+ nwrite;
    }

    /* printf("[pt_sn_copythread] ==> finished copy\n"); */
    
    neu_buf_filled[ch] = 0;
    total_used_n -= 1;
    if(VDEBUG) printf("\e[1;31m[pt_trig_copythread] ==> total_used: %d\e[0m\n",total_used_n);
    //vic
  }  

}

void *pt_sn_filewrite(void *nword_write)
{
  static int file_buf[250000];
  static int w_t1,r_t1,nwrite,dis,is, index, n_write,ik;
  static int read_point_tmp;
  static UINT32 send_array[2];
  
  //xxx
  dwDMABufSize = 200000;
  if(VDEBUG) printf("\e[1;34m{pt_sn_filewrite} ==> sn file write thread started\e[0m\n");

  //ch=0;
  while (1) {
    //vic
    w_t1 = write_point_s;
    r_t1 = read_point_s;
    nwrite = dwDMABufSize/4;
    dis =w_t1 - r_t1;
    while (dis < nwrite) {
      w_t1 = write_point_s;
      r_t1 = read_point_s;
      dis  = w_t1 - r_t1;
      if (dis < 0) dis = jbuf_ev_size + dis;
      //usleep(300);
      usleep(100);
      /* printf("[pt_sn_filewrite] ==> dis: %d nwrite: %d \n",dis,nwrite); */
      /* printf("[pt_sn_filewrite] ==> wps: %d rps: %d \n",write_point_s,read_point_s); */
    
    }
    
    /* printf("\t==> in pt_sn_write ... wp, rp %d, %d \n",write_point_s, read_point_s); */
    if((w_t1 > r_t1) | ((jbuf_ev_size -r_t1)>nwrite)) {
      for (is=0; is<nwrite; is++) {
	file_buf[is] = buffer_ev_s[is+r_t1];
      }
      read_point_tmp = read_point_s+nwrite;
    }
    else {
      for (is=0; is<(jbuf_ev_size-r_t1); is++) {
	file_buf[is] = buffer_ev_s[is+r_t1];
      }
      index =0;
      for (is=(jbuf_ev_size-r_t1); is< (nwrite- (jbuf_ev_size-r_t1)); is++) {
	file_buf[is] = buffer_ev_s[index];
	index = index+1;
      }
      //     read_point_s = index;
      read_point_tmp = index;
    }
    send_array[0] = nwrite;
    n_write = write(fd_sn_pt,send_array,4);
    n_write = write(fd_sn_pt,file_buf,(nwrite*4));
    read_point_s = read_point_tmp;
    if(VDEBUG) printf("\e[1;34m[pt_sn_filewrite] ==> SuperNova write point = %d, read point %d\e[0m\n", write_point_s, read_point_s);
  }
}



static int pcie_send(WDC_DEVICE_HANDLE hDev, int mode, int nword, UINT32 *buff_send)
{
  /* imode =0 single word transfer, imode =1 DMA */
#include "wdc_defs.h"
  static DWORD dwAddrSpace;
  static DWORD dwDMABufSize;

  static UINT32 *buf_send;
  static WD_DMA *pDma_send;
  static DWORD dwStatus;
  static DWORD dwOptions_send = DMA_TO_DEVICE;
  static DWORD dwOffset;
  static UINT32 u32Data;
  static PVOID pbuf_send;
  int nwrite,i,j, iprint;
  static int ifr=0;

  if (ifr == 0) {
    ifr=1;
    dwDMABufSize = 140000;
    dwStatus = WDC_DMAContigBufLock(hDev, &pbuf_send, dwOptions_send, dwDMABufSize, &pDma_send);
    if (WD_STATUS_SUCCESS != dwStatus) {
      printf("Failed locking a send Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
    }
    buf_send = pbuf_send;
  }
  iprint =0;
  if(mode ==1 ) {
    for (i=0; i< nword; i++) {
      *(buf_send+i) = *buff_send++;
      /*	printf("%d \n",*(buf_send+i));   */
    }
  }
  if(mode == 0) {
    nwrite = nword*4;
    /*setup transmiiter */
    dwAddrSpace =2;
    u32Data = 0x20000000;
    dwOffset = 0x18;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    dwAddrSpace =2;
    u32Data = 0x40000000+nwrite;
    dwOffset = 0x18;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    for (j=0; j< nword; j++) {
      dwAddrSpace =0;
      dwOffset = 0x0;
      u32Data = *buff_send++;
      WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    }
    for (i=0; i<20000; i++) {
      dwAddrSpace =2;
      dwOffset = 0xC;
      WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
      if(iprint ==1) printf(" status reed %d %X \n", i, u32Data);
      if(((u32Data & 0x80000000) == 0) && iprint == 1) printf(" Data Transfer complete %d \n", i);
      if((u32Data & 0x80000000) == 0) break;
    }
  }
  if( mode ==1 ){
    nwrite = nword*4;
    WDC_DMASyncCpu(pDma_send);
    /*
      printf(" nwrite = %d \n", nwrite);
      printf(" pcie_send hDev = %d\n", hDev);
      printf(" buf_send = %X\n",*buf_send);
    */
    /*setup transmiiter */
    dwAddrSpace =2;
    u32Data = 0x20000000;
    dwOffset = 0x18;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    dwAddrSpace =2;
    u32Data = 0x40000000+nwrite;
    dwOffset = 0x18;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    /* set up sending DMA starting address */

    dwAddrSpace =2;
    u32Data = 0x20000000;
    dwOffset = 0x0;
    u32Data = pDma_send->Page->pPhysicalAddr & 0xffffffff;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    dwAddrSpace =2;
    u32Data = 0x20000000;
    dwOffset = 0x4;
    u32Data = (pDma_send->Page->pPhysicalAddr >> 32) & 0xffffffff;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    /* byte count */
    dwAddrSpace =2;
    dwOffset = 0x8;
    u32Data = nwrite;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    /* write this will start DMA */
    dwAddrSpace =2;
    dwOffset = 0xc;
    u32Data = 0x00100000;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    for (i=0; i<20000; i++) {
      dwAddrSpace =2;
      dwOffset = 0xC;
      WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
      if(iprint ==1) printf(" DMA status reed %d %X \n", i, u32Data);
      if(((u32Data & 0x80000000) == 0) && iprint == 1) printf(" DMA complete %d \n", i);
      if((u32Data & 0x80000000) == 0) break;
    }
    WDC_DMASyncIo(pDma_send);
  }
  return i;
}

static int pcie_send_1(WDC_DEVICE_HANDLE hDev, int mode, int nword, UINT32 *buff_send)
{
  /* imode =0 single word transfer, imode =1 DMA */
#include "wdc_defs.h"
  static DWORD dwAddrSpace;
  static DWORD dwDMABufSize;

  static UINT32 *buf_send;
  static WD_DMA *pDma_send;
  static DWORD dwStatus;
  static DWORD dwOptions_send = DMA_TO_DEVICE;
  static DWORD dwOffset;
  static UINT32 u32Data;
  static PVOID pbuf_send;
  int nwrite,i,j, iprint;
  static int ifr=0;

  if (ifr == 0) {
    ifr=1;
    dwDMABufSize = 140000;
    dwStatus = WDC_DMAContigBufLock(hDev, &pbuf_send, dwOptions_send, dwDMABufSize, &pDma_send);
    if (WD_STATUS_SUCCESS != dwStatus) {
      printf("Failed locking a send Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
    }
    buf_send = pbuf_send;
  }
  iprint =0;
  if(mode ==1 ) {
    for (i=0; i< nword; i++) {
      *(buf_send+i) = *buff_send++;
      /*	printf("%d \n",*(buf_send+i));   */
    }
  }
  if(mode == 0) {
    nwrite = nword*4;
    /*setup transmiiter */
    dwAddrSpace =2;
    u32Data = 0x20000000;
    dwOffset = 0x20;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    dwAddrSpace =2;
    u32Data = 0x40000000+nwrite;
    dwOffset = 0x20;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    for (j=0; j< nword; j++) {
      dwAddrSpace =4;
      dwOffset = 0x0;
      u32Data = *buff_send++;
      WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    }
    for (i=0; i<20000; i++) {
      dwAddrSpace =2;
      dwOffset = 0xC;
      WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
      if(iprint ==1) printf(" status reed %d %X \n", i, u32Data);
      if(((u32Data & 0x80000000) == 0) && iprint == 1) printf(" Data Transfer complete %d \n", i);
      if((u32Data & 0x80000000) == 0) break;
    }
  }
  if( mode ==1 ){
    nwrite = nword*4;
    WDC_DMASyncCpu(pDma_send);
    /*
      printf(" nwrite = %d \n", nwrite);
      printf(" pcie_send hDev = %d\n", hDev);
      printf(" buf_send = %X\n",*buf_send);
    */
    /*setup transmiiter */
    dwAddrSpace =2;
    u32Data = 0x20000000;
    dwOffset = 0x20;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    dwAddrSpace =2;
    u32Data = 0x40000000+nwrite;
    dwOffset = 0x20;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    /* set up sending DMA starting address */

    dwAddrSpace =2;
    u32Data = 0x20000000;
    dwOffset = 0x0;
    u32Data = pDma_send->Page->pPhysicalAddr & 0xffffffff;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    dwAddrSpace =2;
    u32Data = 0x20000000;
    dwOffset = 0x4;
    u32Data = (pDma_send->Page->pPhysicalAddr >> 32) & 0xffffffff;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    /* byte count */
    dwAddrSpace =2;
    dwOffset = 0x8;
    u32Data = nwrite;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    /* write this will start DMA */
    dwAddrSpace =2;
    dwOffset = 0xc;
    u32Data = 0x00230000;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    for (i=0; i<20000; i++) {
      dwAddrSpace =2;
      dwOffset = 0xC;
      WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
      if(iprint ==1) printf(" DMA status reed %d %X \n", i, u32Data);
      if(((u32Data & 0x80000000) == 0) && iprint == 1) printf(" DMA complete %d \n", i);
      if((u32Data & 0x80000000) == 0) break;
    }
    WDC_DMASyncIo(pDma_send);
  }
  return i;
}



static int pcie_rec(WDC_DEVICE_HANDLE hDev, int mode, int istart, int nword, int ipr_status, UINT32 *buff_rec)
{
  /* imode =0 single word transfer, imode =1 DMA */
#include "wdc_defs.h"
  static DWORD dwAddrSpace;
  static DWORD dwDMABufSize;

  static UINT32 *buf_rec;
  static WD_DMA *pDma_rec;
  static DWORD dwStatus;
  static DWORD dwOptions_rec = DMA_FROM_DEVICE;
  static DWORD dwOffset;
  static UINT32 u32Data;
  static UINT64 u64Data;
  static PVOID pbuf_rec;
  int nread,i,j, iprint,icomp;
  static int ifr=0;

  if (ifr == 0) {
    ifr=1;
    dwDMABufSize = 140000;
    dwStatus = WDC_DMAContigBufLock(hDev, &pbuf_rec, dwOptions_rec, dwDMABufSize, &pDma_rec);
    if (WD_STATUS_SUCCESS != dwStatus) {
      printf("Failed locking a send Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
    }
    buf_rec = pbuf_rec;
  }
  iprint =0;
  //    printf(" istart = %d\n", istart);
  //   printf(" mode   = %d\n", mode);
  /** set up the receiver **/
  if((istart == 1) | (istart == 3)) {
    // initalize transmitter mode register...
    //     printf(" nword = %d \n",nword);
    /*
      if(ipr_status ==1) {
      dwAddrSpace =2;
      u64Data =0;
      dwOffset = 0x18;
      WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
      printf (" status word before set = %x, %x \n",(u64Data>>32), (u64Data &0xffff));
      }
    */
    dwAddrSpace =2;
    u32Data = 0xf0000008;
    dwOffset = 0x28;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    /*initialize the receiver */
    dwAddrSpace =2;
    u32Data = 0x20000000;
    dwOffset = 0x1c;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    /* write byte count **/
    dwAddrSpace =2;
    u32Data = 0x40000000+nword*4;
    dwOffset = 0x1c;
    WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    if(ipr_status ==1) {
      dwAddrSpace =2;
      u64Data =0;
      dwOffset = 0x18;
      WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
      printf (" status word before read = %x, %x \n",(u64Data>>32), (u64Data &0xffff));
    }

    return 0;
  }
  if ((istart == 2) | (istart == 3)) {
    //     if(ipr_status ==1) {
    //      dwAddrSpace =2;
    //      u64Data =0;
    //      dwOffset = 0x18;
    //      WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
    //      printf (" status word before read = %x, %x \n",(u64Data>>32), (u64Data &0xffff));
    //     }
    if(mode == 0) {
      nread = nword/2+1;
      if(nword%2 == 0) nread = nword/2;
      for (j=0; j< nread; j++) {
	dwAddrSpace =0;
	dwOffset = 0x0;
	u64Data =0xbad;
	WDC_ReadAddr64(hDev,dwAddrSpace, dwOffset, &u64Data);
	//       printf("u64Data = %16X\n",u64Data);
	*buff_rec++ = (u64Data &0xffffffff);
	*buff_rec++ = u64Data >>32;
	//       printf("%x \n",(u64Data &0xffffffff));
	//       printf("%x \n",(u64Data >>32 ));
	//       if(j*2+1 > nword) *buff_rec++ = (u64Data)>>32;
	//       *buff_rec++ = 0x0;
      }
      if(ipr_status ==1) {
	dwAddrSpace =2;
	u64Data =0;
	dwOffset = 0x18;
	WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
	printf (" status word after read = %x, %x \n",(u64Data>>32), (u64Data &0xffff));
      }
      return 0;
    }
    if( mode ==1 ){
      nread = nword*4;
      WDC_DMASyncCpu(pDma_rec);
      /*
	printf(" nwrite = %d \n", nwrite);
	printf(" pcie_send hDev = %d\n", hDev);
	printf(" buf_send = %X\n",*buf_send);
      */
      /*setup receiver
	dwAddrSpace =2;
	u32Data = 0x20000000;
	dwOffset = 0x1c;
	WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
	dwAddrSpace =2;
	u32Data = 0x40000000+nread;
	dwOffset = 0x1c;
	WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
      */
      /* set up sending DMA starting address */

      dwAddrSpace =2;
      u32Data = 0x20000000;
      dwOffset = 0x0;
      u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
      WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

      dwAddrSpace =2;
      u32Data = 0x20000000;
      dwOffset = 0x4;
      u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
      WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

      /* byte count */
      dwAddrSpace =2;
      dwOffset = 0x8;
      u32Data = nread;
      WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

      /* write this will start DMA */
      dwAddrSpace =2;
      dwOffset = 0xc;
      u32Data = 0x00100040;
      WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
      icomp=0;
      for (i=0; i<20000; i++) {
        dwAddrSpace =2;
	dwOffset = 0xC;
        WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
	if(iprint ==1) printf(" DMA status read %d %X \n", i, u32Data);
	if(((u32Data & 0x80000000) == 0)) {
          icomp=1;
          if(iprint == 1) printf(" DMA complete %d \n", i);
        }
	if((u32Data & 0x80000000) == 0) break;
      }
      if(icomp == 0) {
        printf("DMA timeout\n");
        return 1;
      }
      WDC_DMASyncIo(pDma_rec);
      for (i=0; i< nword; i++) {
        *buff_rec++ = *(buf_rec+i);
	/*	printf("%d \n",*(buf_send+i));   */
      }
    }
  }
  return 0;
}


int tpc_adc_setup(WDC_DEVICE_HANDLE hDev, int imod_fem, int iframe, int itpc_adc, int ihuff, int icom_factor, int timesize)
{
  
#include "wdc_defs.h"
#define poweroff      0x0
#define poweron       0x1
#define configure_s30 0x2
#define configure_s60 0x3
#define configure_cont 0x20
#define rdstatus      0x80
#define loopback        0x04

#define dcm2_run_off  254
#define dcm2_run_on   255

#define dcm2_online   2
#define dcm2_setmask  3
#define dcm2_offline_busy 4
#define dcm2_load_packet_a 10
#define dcm2_load_packet_b 11
#define dcm2_offline_load 9
#define dcm2_status_read 20
#define dcm2_led_sel     29
#define dcm2_buffer_status_read 30
#define dcm2_status_read_inbuf 21
#define dcm2_status_read_evbuf 22
#define dcm2_status_read_noevnt 23
#define dcm2_zero 12
#define dcm2_compressor_hold 31

#define dcm2_5_readdata 4
#define dcm2_5_firstdcm 8
#define dcm2_5_lastdcm  9
#define dcm2_5_status_read 5
#define dcm2_5_source_id 25
#define dcm2_5_lastchnl 24

#define dcm2_packet_id_a 25
#define dcm2_packet_id_b 26
#define dcm2_hitformat_a 27
#define dcm2_hitformat_b 28

#define part_run_off  254
#define part_run_on   255
#define part_online   2
#define part_offline_busy 3
#define part_offline_hold 4
#define part_status_read 20
#define part_source_id 25


#define  t1_tr_bar 0
#define  t2_tr_bar 4
#define  cs_bar 2

  /**  command register location **/

#define  tx_mode_reg 0x28
#define  t1_cs_reg 0x18
#define  r1_cs_reg 0x1c
#define  t2_cs_reg 0x20
#define  r2_cs_reg 0x24

#define  tx_md_reg 0x28

#define  cs_dma_add_low_reg 0x0
#define  cs_dma_add_high_reg  0x4
#define  cs_dma_by_cnt 0x8
#define  cs_dma_cntrl 0xc
#define  cs_dma_msi_abort 0x10

  /** define status bits **/

#define  cs_init  0x20000000
#define  cs_mode_p 0x8000000
#define  cs_mode_n 0x0
#define  cs_start 0x40000000
#define  cs_done  0x80000000

#define  dma_tr1  0x100000
#define  dma_tr2  0x200000
#define  dma_tr12 0x300000
#define  dma_3dw_trans 0x0
#define  dma_4dw_trans 0x0
#define  dma_3dw_rec   0x40
#define  dma_4dw_rec   0x60
#define  dma_in_progress 0x80000000

#define  dma_abort 0x2

#define  mb_cntrl_add     0x1
#define  mb_cntrl_test_on 0x1
#define  mb_cntrl_test_off 0x0
#define  mb_cntrl_set_run_on 0x2
#define  mb_cntrl_set_run_off 0x3
#define  mb_cntrl_set_trig1 0x4
#define  mb_cntrl_set_trig2 0x5
#define  mb_cntrl_load_frame 0x6
#define  mb_cntrl_load_trig_pos 0x7

#define  mb_feb_power_add 0x1
#define  mb_feb_conf_add 0x2
#define  mb_feb_pass_add 0x3

#define  mb_feb_lst_on          1
#define  mb_feb_lst_off         0
#define  mb_feb_rxreset         2
#define  mb_feb_align           3
#define  mb_feb_pll_reset       5


#define  mb_feb_adc_align       1
#define  mb_feb_a_nocomp        2
#define  mb_feb_b_nocomp        3
#define  mb_feb_blocksize       4
#define  mb_feb_timesize        5
#define  mb_feb_mod_number      6
#define  mb_feb_a_id            7
#define  mb_feb_b_id            8
#define  mb_feb_max             9

#define  mb_feb_test_source    10
#define  mb_feb_test_sample    11
#define  mb_feb_test_frame     12
#define  mb_feb_test_channel   13
#define  mb_feb_test_ph        14
#define  mb_feb_test_base      15
#define  mb_feb_test_ram_data  16

#define  mb_feb_a_test         17
#define  mb_feb_b_test         18

#define  mb_feb_rd_status      20

#define  mb_feb_a_rdhed        21
#define  mb_feb_a_rdbuf        22
#define  mb_feb_b_rdhed        23
#define  mb_feb_b_rdbuf        24

#define  mb_feb_read_probe     30
#define  mb_feb_dram_reset     31
#define  mb_feb_adc_reset      33

#define  mb_a_buf_status       34
#define  mb_b_buf_status       35
#define  mb_a_ham_status       36
#define  mb_b_ham_status       37

#define  mb_feb_a_maxwords     40
#define  mb_feb_b_maxwords     41

#define  mb_feb_hold_enable    42

#define  mb_pmt_adc_reset       1
#define  mb_pmt_spi_add         2
#define  mb_pmt_adc_data_load   3

#define  mb_xmit_conf_add 0x2
#define  mb_xmit_pass_add 0x3

#define  mb_xmit_modcount 0x1
#define  mb_xmit_enable_1 0x2
#define  mb_xmit_enable_2 0x3
#define  mb_xmit_test1 0x4
#define  mb_xmit_test2 0x5

#define   mb_xmit_testdata  10

#define  mb_xmit_rdstatus 20
#define  mb_xmit_rdcounters 21
#define  mb_xmit_link_reset    22
#define  mb_opt_dig_reset   23
#define  mb_xmit_dpa_fifo_reset    24
#define  mb_xmit_dpa_word_align    25

#define  mb_trig_run                1
#define  mb_trig_frame_size         2
#define  mb_trig_deadtime_size      3
#define  mb_trig_active_size        4
#define  mb_trig_delay1_size        5
#define  mb_trig_delay2_size        6

#define  mb_trig_calib_delay        8

#define  mb_trig_prescale0         10
#define  mb_trig_prescale1         11
#define  mb_trig_prescale2         12
#define  mb_trig_prescale3         13
#define  mb_trig_prescale4         14
#define  mb_trig_prescale5         15
#define  mb_trig_prescale6         16
#define  mb_trig_prescale7         17
#define  mb_trig_prescale8         18

#define  mb_trig_mask0             20
#define  mb_trig_mask1             21
#define  mb_trig_mask2             22
#define  mb_trig_mask3             23
#define  mb_trig_mask4             24
#define  mb_trig_mask5             25
#define  mb_trig_mask6             26
#define  mb_trig_mask7             27
#define  mb_trig_mask8             28

#define  mb_trig_rd_param          30
#define  mb_trig_pctrig            31
#define  mb_trig_rd_status         32
#define  mb_trig_reset             33
#define  mb_trig_calib             34
#define  mb_trig_rd_gps            35

#define  mb_trig_g1_allow_min      36
#define  mb_trig_g1_allow_max      37
#define  mb_trig_g2_allow_min      38
#define  mb_trig_g2_allow_max      39

#define  mb_trig_sel1              40
#define  mb_trig_sel2              41
#define  mb_trig_sel3              42
#define  mb_trig_sel4              43

#define  mb_trig_g1_width          45
#define  mb_trig_g2_width          46

#define  mb_trig_p1_delay          50
#define  mb_trig_p1_width          51
#define  mb_trig_p2_delay          52
#define  mb_trig_p2_width          53
#define  mb_trig_p3_delay          54
#define  mb_trig_p3_width          55
#define  mb_trig_pulse_delay       58

#define  mb_trig_pulse1            60
#define  mb_trig_pulse2            61
#define  mb_trig_pulse3            62

#define  mb_shaper_pulsetime        1
#define  mb_shaper_dac              2
#define  mb_shaper_pattern          3
#define  mb_shaper_write            4
#define  mb_shaper_pulse            5
#define  mb_shaper_entrig           6

#define  mb_feb_pmt_gate_size      47
#define  mb_feb_pmt_beam_delay     48
#define  mb_feb_pmt_beam_size      49
#define  mb_feb_pmt_trig_delay     87

#define  mb_feb_pmt_gate1_size     88
#define  mb_feb_pmt_beam1_delay    89
#define  mb_feb_pmt_beam1_size     90
#define  mb_feb_pmt_trig1_delay    91

#define  mb_feb_pmt_ch_set         50
#define  mb_feb_pmt_delay0         51
#define  mb_feb_pmt_delay1         52
#define  mb_feb_pmt_precount       53
#define  mb_feb_pmt_thresh0        54
#define  mb_feb_pmt_thresh1        55
#define  mb_feb_pmt_thresh2        56
#define  mb_feb_pmt_thresh3        57
#define  mb_feb_pmt_width          58
#define  mb_feb_pmt_deadtime       59
#define  mb_feb_pmt_window         60
#define  mb_feb_pmt_words          61
#define  mb_feb_pmt_cos_mul        62
#define  mb_feb_pmt_cos_thres      63
#define  mb_feb_pmt_mich_mul       64
#define  mb_feb_pmt_mich_thres     65
#define  mb_feb_pmt_beam_mul       66
#define  mb_feb_pmt_beam_thres     67
#define  mb_feb_pmt_en_top         68
#define  mb_feb_pmt_en_upper       69
#define  mb_feb_pmt_en_lower       70
#define  mb_feb_pmt_blocksize      71

#define  mb_feb_pmt_test           80
#define  mb_feb_pmt_clear          81
#define  mb_feb_pmt_test_data      82
#define  mb_feb_pmt_pulse          83

#define  mb_feb_pmt_rxreset        84
#define  mb_feb_pmt_align_pulse    85
#define  mb_feb_pmt_rd_counters    86

#define  dma_buffer_size        10000000


  static UINT32 u32Data;
  static unsigned short u16Data;

  static long imod,ichip;
  unsigned short *buffp;


  static UINT32 i,j,k,ifr,nread,iprint,iwrite,ik,il,is,checksum;

  static UINT32 send_array[40000],read_array[1000];

  static UINT32 nmask,index,itmp,nword_tot,nevent,iv,ijk,islow_read;
  static UINT32 imod_p,imod_trig,imod_shaper;

  static int icomp_l,comp_s,ia,ic;

  DWORD dwStatus;
  DWORD dwOptions = DMA_FROM_DEVICE;
  UINT32 iread,icheck,izero;
  UINT32 buf_send[40000];
  static int   count,num,counta,nword,ireadback,nloop,ierror;
  static int   ij,nsend,iloop,inew,idma_readback,iadd,jevent;
  static int   itest,irun,ichip_c,dummy1,itrig_c;
  static int   idup,ihold,idouble,ihold_set,istatus_read;
  static int   idone,tr_bar,t_cs_reg,r_cs_reg,dma_tr;
  static int   ipulse,ibase,a_id,itrig_delay;
  static int   iset,ncount,nsend_f,nwrite,itrig_ext;
  static int   imod_xmit,idiv,isample,irand;
  static int   iframe_length, itrig,idrift_time,ijtrig;
  static int   idelay0, idelay1, threshold0, threshold1, pmt_words;
  static int   cos_mult, cos_thres, en_top, en_upper, en_lower;
  static int   irise, ifall, istart_time, use_pmt, pmt_testpulse;
  static int   ich_head, ich_sample, ich_frm,idebug,ntot_rec,nred;
  static int   ineu,ibusy_send,ibusy_test,ihold_word,ndma_loop;
  static int   irawprint,ifem_fst,ifem_lst,ifem_loop;
  static int   pmt_deadtime,pmt_mich_window;
  static int   oframe,osample,odiv,cframe,csample,cdiv;
  static int   idac_shaper, pmt_dac_scan, pmt_precount;
  unsigned char    charchannel;
  unsigned char    carray[4000];
  struct timespec tim, tim2;
  tim.tv_sec = 0;
  tim.tv_nsec =128000;


  PVOID pbuf_rec;
  WD_DMA *pDma_rec;
  DWORD dwOptions_send = DMA_TO_DEVICE | DMA_ALLOW_CACHE;
  //    DWORD dwOptions_rec = DMA_FROM_DEVICE | DMA_ALLOW_CACHE | DMA_ALLOW_64BIT_ADDRESS;
  DWORD dwOptions_rec = DMA_FROM_DEVICE | DMA_ALLOW_64BIT_ADDRESS;


  UINT32 *px, *py, *py1;

  FILE *outf,*inpf;
  //
  //
  //
  px = &buf_send;
  py = &read_array;

  irand =1;
  imod = imod_fem;
  ichip =1;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_power_add+(0x0<<16); //turn module 11 power on
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);

  usleep(200000);  // wait for 200 ms
  //
  usleep(10000);   // wait for 10ms

  inpf = fopen("/home/ub/module1x_140820_deb_3_21_2016.rbf","r"); // Chi's new-est FPGA code (Mar 21, 2016)
  //inpf = fopen("../../feb_tpc_fpga_default","r");
  //inpf = fopen("../../feb_tpc_fpga_test","r");
  printf(" start booting FEM %d\n", imod);
  ichip=mb_feb_conf_add;
  buf_send[0]=(imod<<11)+(ichip<<8)+0x0+(0x0<<16);  // turn conf to be on
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  //      for (i=0; i<100000; i++) {
  //          ik= i%2;
  //          dummy1= (ik+i)*(ik+i);
  //      }


  /* read data as characters (28941) */
  usleep(1000);   // wait fior a while
  nsend = 500;
  count = 0;
  counta= 0;
  ichip_c = 7; // set ichip_c to stay away from any other command in the
  dummy1 =0;
  while (fread(&charchannel,sizeof(char),1,inpf)==1) {
    carray[count] = charchannel;
    count++;
    counta++;
    if((count%(nsend*2)) == 0) {
      //printf("counta = %d\n",counta);
      buf_send[0] = (imod <<11) +(ichip_c <<8)+ (carray[0]<<16);
      send_array[0] =buf_send[0];
      if(dummy1 <= 5 ) printf(" counta = %d, first word = %x, %x, %x %x %x \n",counta,buf_send[0], carray[0], carray[1]
			      ,carray[2], carray[3]);
      for (ij=0; ij< nsend; ij++) {
	if(ij== (nsend-1)) buf_send[ij+1] = carray[2*ij+1]+(0x0<<16);
	else buf_send[ij+1] = carray[2*ij+1]+ (carray[2*ij+2]<<16);
	//      buf_send[ij+1] = carray[2*ij+1]+ (carray[2*ij+2]<<16);
	send_array[ij+1] = buf_send[ij+1];
      }
      nword =nsend+1;
      i=1;
      //       if(dummy1 == 0)
      ij = pcie_send(hDev, i, nword, px);
      nanosleep(&tim , &tim2);
      dummy1 = dummy1+1;
      count =0;
    }
  }
  if(feof(inpf)) {
    printf("You have reached the end-of-file word count= %d %d\n", counta, count);
    buf_send[0] = (imod <<11) +(ichip_c <<8)+ (carray[0]<<16);
    if ( count > 1) {
      if( ((count-1)%2) ==0) {
	ik =(count-1)/2;
      }
      else {
	ik =(count-1)/2+1;
      }
      ik=ik+2;   // add one more for safety
      printf("ik= %d\n",ik);
      for (ij=0; ij<ik; ij++){
	if(ij == (ik-1)) buf_send[ij+1] = carray[(2*ij)+1]+(((imod<<11)+(ichip<<8)+0x0)<<16);
	else buf_send[ij+1] = carray[(2*ij)+1]+ (carray[(2*ij)+2]<<16);
	send_array[ij+1] = buf_send[ij+1];
      }
    }
    else ik=1;

    for (ij=ik-10; ij< ik+1; ij++) {
      printf("Last data = %d, %x\n",ij,buf_send[ij]);
    }
    nword =ik+1;
    i=1;
    i = pcie_send(hDev, i, nword, px);
  }
  usleep(2000);    // wait for 2ms to cover the packet time plus fpga init time
  fclose(inpf);
  //
  //
  //

  //printf(" enter 1 to continue \n");
  //scanf("%d",&ik);

  //
  //
  //
  ichip=3;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_dram_reset+(0x1<<16);  // turm the DRAM reset on
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  //        imod=11;
  ichip=3;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_dram_reset+(0x0<<16);  // turm the DRAM reset off
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);

  usleep(5000);    // wait for 5 ms for DRAM to be initialized

  //         imod=11;
  ichip=3;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_mod_number+(imod<<16);  // set module number
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  //
  //
  nword =1;

  i = pcie_rec(hDev,0,1,nword,iprint,py);     // init the receiver

  //         imod=11;
  ichip=3;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_rd_status+(0x0<<16);  // read out status
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  py = &read_array;
  i = pcie_rec(hDev,0,2,nword,iprint,py);     // read out 2 32 bits words
  printf("receive data word = %x, %x \n", read_array[0], read_array[1]);

  if(itpc_adc == 1) {
    //
    //      readback status
    //
    i = pcie_rec(hDev,0,1,nword,iprint,py);     // init the receiver

    imod=imod_fem;
    ichip=3;
    buf_send[0]=(imod<<11)+(ichip<<8)+20+(0x0<<16);  // read out status
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);
    py = &read_array;
    i = pcie_rec(hDev,0,2,nword,iprint,py);     // read out 2 32 bits words
    //
    //
    printf("receiv e data word -- after reset = %x, %x \n", read_array[0], read_array[1]);
    printf(" module = %d, command = %d \n", ((read_array[0]>>11) & 0x1f), (read_array[0] &0xff));
    printf(" ADC right dpa lock     %d \n", ((read_array[0]>>17) & 0x1));
    printf(" ADC left  dpa lock     %d \n", ((read_array[0]>>18) & 0x1));
    printf(" block error 2          %d \n", ((read_array[0]>>19) & 0x1));
    printf(" block error 1          %d \n", ((read_array[0]>>20) & 0x1));
    printf(" pll lcoked             %d \n", ((read_array[0]>>21) & 0x1));
    printf(" superNova mem ready    %d \n", ((read_array[0]>>22) & 0x1));
    printf(" beam      mem ready    %d \n", ((read_array[0]>>23) & 0x1));
    printf(" ADC right PLL locked   %d \n", ((read_array[0]>>24) & 0x1));
    printf(" ADC left PLL locked    %d \n", ((read_array[0]>>25) & 0x1));
    printf(" ADC align cmd right    %d \n", ((read_array[0]>>26) & 0x1));
    printf(" ADC align cmd left     %d \n", ((read_array[0]>>27) & 0x1));
    printf(" ADC align done right   %d \n", ((read_array[0]>>28) & 0x1));
    printf(" ADC align done left    %d \n", ((read_array[0]>>29) & 0x1));
    printf(" Neutrino data empty    %d \n", ((read_array[0]>>30) & 0x1));
    printf(" Neutrino Header empty  %d \n", ((read_array[0]>>31) & 0x1));

    //
    //    the ADC spi stream.. The 16 bits data before the last word is r/w, w1, w2 and 13 bits address
    //                         the last 16 bits.upper byte = 8 bits data and lower 8 bits ignored.
    //
    for (is=0; is<8; is++) {
      //      imod=11;
      ichip=5;
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_pmt_spi_add)+((is & 0xf)<<16); //set spi address
      i=1;
      k=1;
      i = pcie_send(hDev, i, k, px);
      usleep(2000);   // sleep for 2ms
      printf(" spi port %d \n",is);
      //       scanf("%d",&ik);

      //      imod=11;
      ichip=5;
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_pmt_adc_data_load)+(0x0300<<16); //1st next word will be overwrite by the next next word
      buf_send[1]=(((0x0)<<13)+(0xd))+((0xc)<<24)+((0x0)<<16);
      //        buf_send[1]=(((0x0)<<13)+(0xd))+((0x0)<<24)+((0x0)<<16);
      //
      //  set /w =0, w1,w2 =0, a12-a0 = 0xd, data =0xb;
      //

      i=1;
      k=2;
      i = pcie_send(hDev, i, k, px);
      usleep(2000);   // sleep for 2ms
      printf(" spi port 2nd command %d \n",is);
      //       scanf("%d",&ik);

      //      imod=11;
      ichip=5;
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_pmt_adc_data_load)+(0x0300<<16); //1st next word will be overwrite by the next next word
      buf_send[1]=(((0x0)<<13)+(0xff))+((0x1)<<24)+((0x0)<<16);
      //
      //  set /w =0, w1,w2 =0, a12-a0 = 0xff, data =0x1;
      //  write to transfer register
      //

      i=1;
      k=2;
      i = pcie_send(hDev, i, k, px);
      usleep(2000);   // sleep for 2ms

    }

    //       printf(" enter 1 to continue FPGA ADC receiver reset\n");
    //       scanf("%d",&ik);
    //
    //    send FPGA ADC receiver reset
    //
    //      imod=11;
    ichip=3;
    buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_adc_reset+(0x1<<16);  // FPGA ADC receiver reset on
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);
  

    //       printf(" enter 1 to continue FPGA ADC receiver align\n");
    //       scanf("%d",&ik);
    //
    //    send FPGA ADC align
    //
    //      imod=11;
    ichip=3;
    buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_adc_align+(0x0<<16);  // FPGA ADC receiver reset off
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);
    usleep(1000);
    //
    //
    //
    //
    //     scanf("%d", &i);
    //
    nword = 2;
    i = pcie_rec(hDev,0,1,nword,iprint,py);     // init the receiver
    //
    imod=imod_fem;
    ichip=3;
    buf_send[0]=(imod<<11)+(ichip<<8)+20+(0x0<<16);  // read out status
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);
    //     scanf("%d", &i);
    usleep(10);
    py = &read_array;
    read_array[0] =0;
    i = pcie_rec(hDev,0,2,nword,iprint,py);     // read out 2 32 bits words
    usleep(10);
    //
    printf("receiv e data word -- after reset = %x, %x \n", read_array[0], read_array[1]);
    printf(" module = %d, command = %d \n", ((read_array[0]>>11) & 0x1f), (read_array[0] &0xff));
    printf(" ADC right dpa lock     %d \n", ((read_array[0]>>17) & 0x1));
    printf(" ADC left  dpa lock     %d \n", ((read_array[0]>>18) & 0x1));
    printf(" block error 2          %d \n", ((read_array[0]>>19) & 0x1));
    printf(" block error 1          %d \n", ((read_array[0]>>20) & 0x1));
    printf(" pll lcoked             %d \n", ((read_array[0]>>21) & 0x1));
    printf(" superNova mem ready    %d \n", ((read_array[0]>>22) & 0x1));
    printf(" beam      mem ready    %d \n", ((read_array[0]>>23) & 0x1));
    printf(" ADC right PLL locked   %d \n", ((read_array[0]>>24) & 0x1));
    printf(" ADC left PLL locked    %d \n", ((read_array[0]>>25) & 0x1));
    printf(" ADC align cmd right    %d \n", ((read_array[0]>>26) & 0x1));
    printf(" ADC align cmd left     %d \n", ((read_array[0]>>27) & 0x1));
    printf(" ADC align done right   %d \n", ((read_array[0]>>28) & 0x1));
    printf(" ADC align done left    %d \n", ((read_array[0]>>29) & 0x1));
    printf(" Neutrino data empty    %d \n", ((read_array[0]>>30) & 0x1));
    printf(" Neutrino Header empty  %d \n", ((read_array[0]>>31) & 0x1));

    //       printf(" finish align \n");
    //       scanf("%d",&ik);
    //
    //    the ADC spi stream.. The 16 bits data before the last word is r/w, w1, w2 and 13 bits address
    //                         the last 16 bits.upper byte = 8 bits data and lower 8 bits ignored.
    //
    for (is=0; is<8; is++) {
      //        imod=11;
      ichip=5;
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_pmt_spi_add)+((is & 0xf)<<16); //set spi address
      i=1;
      k=1;
      i = pcie_send(hDev, i, k, px);
      usleep(2000);   // sleep for 2ms
      printf(" spi port %d \n",is);
      //       scanf("%d",&ik);

      //        imod=11;
      ichip=5;
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_pmt_adc_data_load)+(0x0300<<16); //1st next word will be overwrite by the next next word
      //        buf_send[1]=(((0x0)<<13)+(0xd))+((0x9)<<24)+((0x0)<<16);
      buf_send[1]=(((0x0)<<13)+(0xd))+((0x0)<<24)+((0x0)<<16);
      //
      //  set /w =0, w1,w2 =0, a12-a0 = 0xd, data =0xb;
      //

      i=1;
      k=2;
      i = pcie_send(hDev, i, k, px);
      usleep(2000);   // sleep for 2ms
      printf(" spi port 2nd command %d \n",is);
      //       scanf("%d",&ik);

      //       imod=11;
      ichip=5;
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_pmt_adc_data_load)+(0x0300<<16); //1st next word will be overwrite by the next next word
      buf_send[1]=(((0x0)<<13)+(0xff))+((0x1)<<24)+((0x0)<<16);
      //
      //  set /w =0, w1,w2 =0, a12-a0 = 0xff, data =0x1;
      //  write to transfer register
      //

      i=1;
      k=2;
      i = pcie_send(hDev, i, k, px);
      usleep(2000);   // sleep for 2ms

    }
  }
  else {
    nword =1;
    //
    // set to use test generator 2, set test =2
    //
    //       imod=11;
    ichip=mb_feb_pass_add;
    buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_test_source+(0x2<<16);  // set test source to 2
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);


    //
    //    start loading the test 2 data memory
    //
    //       imod =11;
    ichip=3;
    printf(" loading channels w/ test data... ");

    //
    // Rand method
    //
    //srand(time(NULL));
    //int rand_seed = rand() & 0xfff;
    int rand_seed = 100;
    srand(rand_seed);
    printf("\nrandom seed: %d\n",rand_seed);
    ibase=2000;
    ijk=ibase;
    /*
    for(ik=0; ik<256; ik++) {
      if((ik%icom_factor)==0) ijk = rand() & 0xfff;
      else {
	int tmp_adc = 0;
	while( tmp_adc == 0)
	  tmp_adc = rand() & 0x7;
	ijk += tmp_adc;
	ijk -= 4;
      }
      ijk = ijk & 0xfff;
      if(ik%4 == 0) printf("\n");
      printf("%-4d ",(ijk & 0xfff));
      send_array[ik]=ijk;
    }
    */

    for(is=0; is<64; is++) {
      printf("Channel %-2d\n",is);
      for(ik=0; ik<256; ik++) {
	if((ik%icom_factor)==0) ijk = rand() & 0xfff;
	else {
	  int tmp_adc = 0;
	  while( tmp_adc == 0)
	    tmp_adc = rand() & 0x7;
	  ijk += tmp_adc;
	  ijk -= 4;
	}
	ijk = ijk & 0xfff;

	if(ik%4 == 0) printf("\n");
	printf("%-4d ",(ijk & 0xfff));
	send_array[is*256+ik] = ijk;
      }
      printf("\n");
    }
    printf("\n");

    for(is=0; is<64; is++) {
      ik = 0x4000+is;                        // load channel address
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_test_ram_data)+((ik & 0xffff)<<16); // load channe address
      i = pcie_send(hDev, 1, 1, px);
      for(ik=0; ik<256; ik++) {
	//k = 0x8000 + send_array[ik];        // make sure bit 15-12 is clear for the data
	k = 0x8000 + send_array[is*256+ik];
	buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_test_ram_data)+((k & 0xffff)<<16); // load test data
	i = pcie_send(hDev, 1, 1, px);
	/*
	if(is==0){
	  k = 0x8000 + send_array[is*256+ik];        // make sure bit 15-12 is clear for the data
	  buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_test_ram_data)+((k & 0xffff)<<16); // load test data
	  i = pcie_send(hDev, 1, 1, px);
	}else{
	  k = 0x8000;
	  buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_test_ram_data)+((k & 0xffff)<<16); // load test data
	  i = pcie_send(hDev, 1, 1, px);
	}
	*/
      }
    }

    //
    // Orig method
    //
    /*
    for (is=0; is<64; is++) {
      ik = 0x4000+is;                        // load channel address
      buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_test_ram_data)+((ik & 0xffff)<<16); // load channe address
      i = pcie_send(hDev, 1, 1, px);
      //ibase = 32*is;
      ibase = 2000;
      il = is%8;
      //if(il == 0) printf("%d ",is);
      if(il==0) printf("Channel: %d\n",is);
      for (ik=0; ik< 256; ik++) {                 // loop over all possible address
	if(irand ==1) ijk = rand() & 0xfff ;        // use random number
	else ijk= (ibase+ik*8) & 0xfff;
	//          else {
	//            if(ik ==0) ijk =0x111;
	//            else ijk= (ibase+ik*8) & 0xfff;
	//           }

	if((ik%icom_factor) ==0) ic =ijk;        // set data to repeat for 4 samples....
	ijk = ic;

	k = 0x8000 + ijk;        // make sure bit 15-12 is clear for the data
	buf_send[0]=(imod<<11)+(ichip<<8)+(mb_feb_test_ram_data)+((k & 0xffff)<<16); // load test data
	i = pcie_send(hDev, 1, 1, px);
	send_array[is*256+ik]=ijk;           //load up data map

	if(is==0) {
	  if(ik%8 == 0) printf("\n");
	  printf("%-4d ",(k & 0xfff));
	}

      }
    }
    */

    printf(" ... done!\n");

    /*
    ichip=3;
    if(ihuff == 1) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_nocomp+(0x0<<16);  // turn the compression
    else buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_nocomp+(0x1<<16);  // set b channel no compression
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);
    */
  }
  //
  //
  //    set compression or not
  //
  ichip =3;
  if(ihuff == 1)buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_a_nocomp+(0x0<<16);  // turn the compression
  else buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_a_nocomp+(0x1<<16);  // set b channel no compression
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  //    printf(" type 1 to continue, ihuff = %d\n", ihuff);
  //    scanf("%d", &i);
  if(ihuff == 1) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_nocomp+(0x0<<16);  // turn the compression
  else buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_nocomp+(0x1<<16);  // set b channel no compression
  //  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_nocomp+(0x0<<16); // always turn on supernova compression
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
    
  //         timesize =4;
  //         imod=11;
  ichip=3;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_timesize+(timesize<<16);  // set drift time size
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);

  a_id =0xf;
  //         imod=11;
  ichip=3;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_id+(a_id<<16);  // set a_id
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);

  //       imod=11;
  ichip=4;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_b_id+(a_id<<16);  // set b_id
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  //
  //     set max word in the pre-buffer memory
  //
  ik=8000;
  //         imod=11;
  ichip=3;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_max+(ik<<16);  // set pre-buffer max word
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  //
  //
  //
  //     enable hold
  //
  //         imod=11;
  imod = imod_fem;
  ichip=3;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_hold_enable+(0x1<<16);  // enable the hold
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  return i;

}
