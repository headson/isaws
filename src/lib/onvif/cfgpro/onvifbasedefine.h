#ifndef __ONVIF_SERVER_BASEDEFINE_H__
#define __ONVIF_SERVER_BASEDEFINE_H__

#define HW_INFO_WRITTED 0x27052011
#define OEM_INFO_SIZE 7
#define HW_VERSION_SIZE 4

//share mem 里保存这个，cpu序
typedef struct hw_info {
	unsigned int write_status;
	unsigned char mac[8];
	unsigned int serialno[2];
	unsigned int device_type;
	char exdata_checker;//='.'表示后面有东西
	unsigned char oem_info[OEM_INFO_SIZE];//前3位厂商的编号，后4位厂商简称
	//硬件版本号,1001是老版，新版第2字节高4位表示支持最大通道路数，低4位表示最大算法通道数，
	//第3字节高4位表示支持模拟最大数，低4位表示支持高清最大通道数，最后一位表示hwflag
	unsigned char hw_version[HW_VERSION_SIZE];
	unsigned int hw_flag;//硬件标识,新版改为保存HwType
	unsigned int exdataSize;
  unsigned int board_version;//板子版本
}hw_info_t;

#endif
