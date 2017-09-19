/************************************************************************
*Author      : Sober.Peng 17-09-20
*Description :
************************************************************************/
#include "cgpioevent.h"

extern "C" {
#include "include/mpi_sys.h"
#include "include/mpi_venc.h"
#include "include/hi_comm_venc.h"
};

void CGpioEvent::GpioInit() {
  // GPIO8_0 output
  HI_MPI_SYS_SetReg(0x200f0100, 0x1);

  HI_U32 nval = 0;
  HI_MPI_SYS_GetReg(0x201C0400, &nval);
  nval |= 0x1;
  HI_MPI_SYS_SetReg(0x201C0400, nval);
  HI_MPI_SYS_SetReg(0x201c0004, 0x0);   // default value

  // GPIO0_3 output
  HI_MPI_SYS_SetReg(0x200F0030, 0x0);

  nval = 0;
  HI_MPI_SYS_GetReg(0x20140400, &nval);
  nval |= 0x8;
  HI_MPI_SYS_SetReg(0x20140400, nval);
  HI_MPI_SYS_SetReg(0x20140020, 0x0);   // default value
}

void CGpioEvent::IRCutOpen() {
  HI_MPI_SYS_SetReg(0x201c0004, 1);
  HI_MPI_SYS_SetReg(0x20140020, 0x8);

  //VENC_COLOR2GREY_S vcs;
  //vcs.bColor2Grey = HI_TRUE;
  //HI_MPI_VENC_SetColor2Grey(0, &vcs);
  //HI_MPI_VENC_SetColor2Grey(1, &vcs);
}

void CGpioEvent::IRCutClose() {
  HI_MPI_SYS_SetReg(0x201c0004, 0);
  HI_MPI_SYS_SetReg(0x20140020, 0);

  //VENC_COLOR2GREY_S vcs;
  //vcs.bColor2Grey = HI_FALSE;
  //HI_MPI_VENC_SetColor2Grey(0, &vcs);
  //HI_MPI_VENC_SetColor2Grey(1, &vcs);
}

