#include <stdio.h>

#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_16BIT_REG     0x0709  /* 16BIT REG WIDTH */
#define I2C_16BIT_DATA    0x070a  /* 16BIT DATA WIDTH */

#define PCF8563_REG_ST1   0x00 /* status */
#define PCF8563_REG_ST2   0x01

#define PCF8563_REG_SC    0x02 /* datetime */
#define PCF8563_REG_MN    0x03
#define PCF8563_REG_HR    0x04
#define PCF8563_REG_DM    0x05
#define PCF8563_REG_DW    0x06
#define PCF8563_REG_MO    0x07
#define PCF8563_REG_YR    0x08

#define PCF8563_REG_AMN   0x09 /* alarm */
#define PCF8563_REG_AHR   0x0A
#define PCF8563_REG_ADM   0x0B
#define PCF8563_REG_ADW   0x0C

#define PCF8563_REG_CLKO  0x0D /* clock out */
#define PCF8563_REG_TMRC  0x0E /* timer control */
#define PCF8563_REG_TMR   0x0F /* timer */

#define PCF8563_SC_LV     0x80 /* low voltage */
#define PCF8563_MO_C      0x80 /* century */

static int          g_fd = -1;
const unsigned char i2c_addr = 0xA2;
const unsigned int  addr_byte = 1;
const unsigned int  data_byte = 1;
static unsigned int c_polarity = 0;

int i2c_init(void) {
  int ret = 0;
  if (g_fd >= 0) {
    return 0;
  }

  g_fd = open("/dev/i2c-2", O_RDWR);
  if(g_fd < 0) {
    printf("Open /dev/i2c-2 error!\n");
    return -1;
  }

  ret = ioctl(g_fd, I2C_SLAVE_FORCE, i2c_addr);
  if (ret < 0) {
    printf("CMD_SET_DEV error!\n");
    return ret;
  }
  return 0;
}

int i2c_exit(void) {
  if (g_fd >= 0) {
    close(g_fd);
    g_fd = -1;
    return 0;
  }
  return -1;
}

int i2c_read(int addr) {
  int ret;
  char buf[8];
  int idx = 0;

  buf[idx++] = addr & 0xFF;
  if (addr_byte == 2) {
    ret = ioctl(g_fd, I2C_16BIT_REG, 1);
    buf[idx++] = addr >> 8;
  } else {
    ret = ioctl(g_fd, I2C_16BIT_REG, 0);
  }
  if (ret < 0) {
    printf("CMD_SET_REG_WIDTH error!\n");
    return -1;
  }

  if (data_byte == 2) {
    ret = ioctl(g_fd, I2C_16BIT_DATA, 1);
  } else {
    ret = ioctl(g_fd, I2C_16BIT_DATA, 0);
  }

  if (ret) {
    printf("hi_i2c read faild!\n");
    return -1;
  }

  ret = read(g_fd, buf, idx);
  if(ret < 0) {
    printf("I2C_WRITE error!\n");
    return -1;
  }

  int data = 0;
  if (data_byte == 2) {
    data = buf[0] | (buf[1] << 8);
  } else {
    data = buf[0];
  }
  return data;
}

int i2c_write(int addr, int data) {
  int ret;
  char buf[8];
  int idx = 0;

  buf[idx++] = addr & 0xFF;
  if (addr_byte == 2) {
    ret = ioctl(g_fd, I2C_16BIT_REG, 1);
    buf[idx++] = addr >> 8;
  } else {
    ret = ioctl(g_fd, I2C_16BIT_REG, 0);
  }

  if (ret < 0) {
    printf("CMD_SET_REG_WIDTH error!\n");
    return -1;
  }

  buf[idx++] = data;
  if (data_byte == 2) {
    ret = ioctl(g_fd, I2C_16BIT_DATA, 1);
    buf[idx++] = data >> 8;
  } else {
    ret = ioctl(g_fd, I2C_16BIT_DATA, 0);
  }

  if (ret) {
    printf("hi_i2c write faild!\n");
    return -1;
  }

  ret = write(g_fd, buf, idx);
  if(ret < 0) {
    printf("I2C_WRITE error!\n");
    return -1;
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////
static unsigned bcd2bin(unsigned char val) {
  return (val & 0x0f) + (val >> 4) * 10;
}

static unsigned char bin2bcd(unsigned val) {
  return ((val / 10) << 4) + val % 10;
}

int GetTime(struct tm* ptm) {
  unsigned char buf[13] = {PCF8563_REG_ST1};
  for(char addr = 0x00; addr <= 0x0D; addr++) {
    buf[addr] = i2c_read(addr);
  }

  if (ptm) {
    ptm->tm_sec = bcd2bin(buf[PCF8563_REG_SC] & 0x7F);
    ptm->tm_min = bcd2bin(buf[PCF8563_REG_MN] & 0x7F);
    ptm->tm_hour = bcd2bin(buf[PCF8563_REG_HR] & 0x3F);    /* rtc hr 0-23 */
    ptm->tm_mday = bcd2bin(buf[PCF8563_REG_DM] & 0x3F);
    ptm->tm_wday = buf[PCF8563_REG_DW] & 0x07;
    ptm->tm_mon = bcd2bin(buf[PCF8563_REG_MO] & 0x1F) - 1; /* rtc mn 1-12 */
    ptm->tm_year = bcd2bin(buf[PCF8563_REG_YR]);
    if (ptm->tm_year < 70)
      ptm->tm_year += 100;                                 /* assume we are in 1970...2069 */
    /* detect the polarity heuristically. see note above. */
    c_polarity = (buf[PCF8563_REG_MO] & PCF8563_MO_C) ?
                 (ptm->tm_year >= 100) : (ptm->tm_year < 100);
    return 0;
  }
  return -1;
}

int SetTime(struct tm* ptm) {
  if (ptm) {
    unsigned char buf[9] = {0};
    /* hours, minutes and seconds */
    buf[PCF8563_REG_SC] = bin2bcd(ptm->tm_sec);
    buf[PCF8563_REG_MN] = bin2bcd(ptm->tm_min);
    buf[PCF8563_REG_HR] = bin2bcd(ptm->tm_hour);
    buf[PCF8563_REG_DM] = bin2bcd(ptm->tm_mday);
    buf[PCF8563_REG_MO] = bin2bcd(ptm->tm_mon + 1);    /* month, 1 - 12 */
    buf[PCF8563_REG_YR] = bin2bcd(ptm->tm_year % 100); /* year and century */
    if (c_polarity ? (ptm->tm_year >= 100) : (ptm->tm_year < 100))
      buf[PCF8563_REG_MO] |= PCF8563_MO_C;
    buf[PCF8563_REG_DW] = ptm->tm_wday & 0x07;

    int err = 0;
    /* write register's data */
    for (int i = 0; i < 7; i++) {
      err = i2c_write(PCF8563_REG_SC + i, buf[PCF8563_REG_SC + i]);
      if (err != 0) {
        printf("%s: err=%d addr=%02x, data=%02x\n",
               __func__, err, PCF8563_REG_SC + i, buf[PCF8563_REG_SC + i]);
        return -5;
      }
    };
    return 0;
  }
  return -1;
}

int main(int argc, char* argv[]) {
  int nData = 0;
  if (argc < 2)
    return 0;

  i2c_init();

  nData = i2c_read(0x00);
  if((nData & 0x80)) {
    i2c_write(0x00, 0x00);
    printf("set rtc pm control.\n");
  }

  if (strcmp(argv[1], "-w") == 0) {
    struct tm* ptm = NULL;
    time_t tt = time(NULL);

    ptm = localtime(&tt);
    SetTime(ptm);
  } else if (strcmp(argv[1], "-s") == 0) {
    struct tm ctm;
    GetTime(&ctm);
    printf("%4d-%02d-%02d %02d:%02d:%02d\n",
           ctm.tm_year+1900,
           ctm.tm_mon+1,
           ctm.tm_mday,
           ctm.tm_hour,
           ctm.tm_min,
           ctm.tm_sec);
    struct timeval tvval;
    tvval.tv_sec = mktime(&ctm);
    tvval.tv_usec = 0;
    settimeofday(&tvval, NULL);
  } else if(strcmp(argv[1], "-r") == 0) {
    struct tm ctm;
    GetTime(&ctm);

    char cmd[64] = {0};
    snprintf(cmd, 63,
             "date -s \"%4d-%02d-%02d %02d:%02d:%02d\"",
             ctm.tm_year+1900,
             ctm.tm_mon+1,
             ctm.tm_mday,
             ctm.tm_hour,
             ctm.tm_min,
             ctm.tm_sec);
    system(cmd);
  } else {
    printf("no this cmd.\n");
  }

  i2c_exit();
  return 0;
}
