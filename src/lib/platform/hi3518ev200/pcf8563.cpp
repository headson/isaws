#include <stdio.h>

static unsigned bcd2bin(unsigned char val) {
  return (val & 0x0f) + (val >> 4) * 10;
}

static unsigned char bin2bcd(unsigned val) {
  return ((val / 10) << 4) + val % 10;
}

struct rtc_time {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
};

#define PCF8563_REG_ST1		0x00 /* status */
#define PCF8563_REG_ST2		0x01

#define PCF8563_REG_SC		0x02 /* datetime */
#define PCF8563_REG_MN		0x03
#define PCF8563_REG_HR		0x04
#define PCF8563_REG_DM		0x05
#define PCF8563_REG_DW		0x06
#define PCF8563_REG_MO		0x07
#define PCF8563_REG_YR		0x08

#define PCF8563_REG_AMN		0x09 /* alarm */
#define PCF8563_REG_AHR		0x0A
#define PCF8563_REG_ADM		0x0B
#define PCF8563_REG_ADW		0x0C

#define PCF8563_REG_CLKO	0x0D /* clock out */
#define PCF8563_REG_TMRC	0x0E /* timer control */
#define PCF8563_REG_TMR		0x0F /* timer */

#define PCF8563_SC_LV		0x80 /* low voltage */
#define PCF8563_MO_C		0x80 /* century */

/*
 * In the routines that deal directly with the pcf8563 hardware, we use
 * rtc_time -- month 0-11, hour 0-23, yr = calendar year-epoch.
 */
static int pcf8563_get_datetime(struct i2c_client *client, struct rtc_time *tm) {
  struct pcf8563 *pcf8563 = i2c_get_clientdata(client);
  unsigned char buf[13] = { PCF8563_REG_ST1 };

  struct i2c_msg msgs[] = {
    { client->addr, 0, 1, buf },	/* setup read ptr */
    { client->addr, I2C_M_RD, 13, buf },	/* read status + date */
  };

  /* read registers */
  if ((i2c_transfer(client->adapter, msgs, 2)) != 2) {
    dev_err(&client->dev, "%s: read error\n", __func__);
    return -EIO;
  }

  if (buf[PCF8563_REG_SC] & PCF8563_SC_LV)
    dev_info(&client->dev,
             "low voltage detected, date/time is not reliable.\n");

  dev_dbg(&client->dev,
          "%s: raw data is st1=%02x, st2=%02x, sec=%02x, min=%02x, hr=%02x, "
          "mday=%02x, wday=%02x, mon=%02x, year=%02x\n",
          __func__,
          buf[0], buf[1], buf[2], buf[3],
          buf[4], buf[5], buf[6], buf[7],
          buf[8]);

  tm->tm_sec = bcd2bin(buf[PCF8563_REG_SC] & 0x7F);
  tm->tm_min = bcd2bin(buf[PCF8563_REG_MN] & 0x7F);
  tm->tm_hour = bcd2bin(buf[PCF8563_REG_HR] & 0x3F); /* rtc hr 0-23 */
  tm->tm_mday = bcd2bin(buf[PCF8563_REG_DM] & 0x3F);
  tm->tm_wday = buf[PCF8563_REG_DW] & 0x07;
  tm->tm_mon = bcd2bin(buf[PCF8563_REG_MO] & 0x1F) - 1; /* rtc mn 1-12 */
  tm->tm_year = bcd2bin(buf[PCF8563_REG_YR]);
  if (tm->tm_year < 70)
    tm->tm_year += 100;	/* assume we are in 1970...2069 */
  /* detect the polarity heuristically. see note above. */
  pcf8563->c_polarity = (buf[PCF8563_REG_MO] & PCF8563_MO_C) ?
                        (tm->tm_year >= 100) : (tm->tm_year < 100);

  dev_dbg(&client->dev, "%s: tm is secs=%d, mins=%d, hours=%d, "
          "mday=%d, mon=%d, year=%d, wday=%d\n",
          __func__,
          tm->tm_sec, tm->tm_min, tm->tm_hour,
          tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

  /* the clock can give out invalid datetime, but we cannot return
   * -EINVAL otherwise hwclock will refuse to set the time on bootup.
   */
  if (rtc_valid_tm(tm) < 0)
    dev_err(&client->dev, "retrieved date/time is not valid.\n");

  return 0;
}

static int pcf8563_set_datetime(struct i2c_client *client, struct rtc_time *tm) {
  struct pcf8563 *pcf8563 = i2c_get_clientdata(client);
  int i, err;
  unsigned char buf[9];

  dev_dbg(&client->dev, "%s: secs=%d, mins=%d, hours=%d, "
          "mday=%d, mon=%d, year=%d, wday=%d\n",
          __func__,
          tm->tm_sec, tm->tm_min, tm->tm_hour,
          tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

  /* hours, minutes and seconds */
  buf[PCF8563_REG_SC] = bin2bcd(tm->tm_sec);
  buf[PCF8563_REG_MN] = bin2bcd(tm->tm_min);
  buf[PCF8563_REG_HR] = bin2bcd(tm->tm_hour);

  buf[PCF8563_REG_DM] = bin2bcd(tm->tm_mday);

  /* month, 1 - 12 */
  buf[PCF8563_REG_MO] = bin2bcd(tm->tm_mon + 1);

  /* year and century */
  buf[PCF8563_REG_YR] = bin2bcd(tm->tm_year % 100);
  if (pcf8563->c_polarity ? (tm->tm_year >= 100) : (tm->tm_year < 100))
    buf[PCF8563_REG_MO] |= PCF8563_MO_C;

  buf[PCF8563_REG_DW] = tm->tm_wday & 0x07;

  /* write register's data */
  for (i = 0; i < 7; i++) {
    unsigned char data[2] = { PCF8563_REG_SC + i,
                              buf[PCF8563_REG_SC + i]
                            };

    err = i2c_master_send(client, data, sizeof(data));
    if (err != sizeof(data)) {
      dev_err(&client->dev,
              "%s: err=%d addr=%02x, data=%02x\n",
              __func__, err, data[0], data[1]);
      return -EIO;
    }
  };

  return 0;
}


