#ifndef __ONVIF_SERVER_BASEDEFINE_H__
#define __ONVIF_SERVER_BASEDEFINE_H__

#define HW_INFO_WRITTED 0x27052011
#define OEM_INFO_SIZE 7
#define HW_VERSION_SIZE 4

//share mem �ﱣ�������cpu��
typedef struct hw_info {
	unsigned int write_status;
	unsigned char mac[8];
	unsigned int serialno[2];
	unsigned int device_type;
	char exdata_checker;//='.'��ʾ�����ж���
	unsigned char oem_info[OEM_INFO_SIZE];//ǰ3λ���̵ı�ţ���4λ���̼��
	//Ӳ���汾��,1001���ϰ棬�°��2�ֽڸ�4λ��ʾ֧�����ͨ��·������4λ��ʾ����㷨ͨ������
	//��3�ֽڸ�4λ��ʾ֧��ģ�����������4λ��ʾ֧�ָ������ͨ���������һλ��ʾhwflag
	unsigned char hw_version[HW_VERSION_SIZE];
	unsigned int hw_flag;//Ӳ����ʶ,�°��Ϊ����HwType
	unsigned int exdataSize;
  unsigned int board_version;//���Ӱ汾
}hw_info_t;

#endif
