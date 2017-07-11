#ifndef CDATABASE_H
#define CDATABASE_H

#include "appinc.h"
#include <stdint.h>

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlTableModel>

#include "cadminfo.h"
#include "cusrinfo.h"
#include "caesinfo.h"
#include "cdevinfo.h"
#include "cevtinfo.h"
#include "cevttype.h"

#include "cdispinfo.h"
#include "cstoreinfo.h"

class CMyQueryModel : public QSqlQueryModel
{
public:
    explicit CMyQueryModel(QObject *parent=0);
    virtual ~CMyQueryModel();

    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
};

class CMyTableModel : public QSqlTableModel
{
public:
    explicit CMyTableModel(QObject *parent=0);
    virtual ~CMyTableModel();

    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
};

//////////////////////////////////////////////////////////////////////////
class CDataBase : public QObject
{
    Q_OBJECT

public:
    CDataBase(QObject *parent=NULL);
    ~CDataBase();

signals:
    void    RefreshPeopleCount();

public:
    bool Init(QString sPath);                   // ������
    bool Init(QString sHost, int32_t nPort);    // ������

    bool Transaction();             // ����ʼ
    bool Commit();                  // �������
    QSqlQueryModel* GetQueryModel();
    QSqlTableModel* GetTableModel();

public:
    // ����Ա��
    bool CreateTableAdmin();
    bool CheckLogin(QString sUsr, QString sPwd1, QString sPwd2, QString sMac);
    bool DeleteTableAdmin(QString& sName);
    bool InsertTableAdmin(CAdmInfo& cAdm);
    bool UpdateTableAdmin(CAdmInfo& cAdm);

    // ͨ���豸ID�����ҹ���Ա
    bool SelectTableUser(QVector<CUsrInfo>& cUsr, QString sDevID, bool bAdm=false);

    int32_t SelectTableUserCount(const QString& sFilter);
    bool SelectTableUser(CUsrInfo& cUsr, const QString& sUsername);

    // �豸��
    bool    CreateTableDevice();
    int32_t DeleteTableDevice(QString& sUuid);
    int32_t InsertTableDevice(const CDevInfo& cDev);
    int32_t UpdateTableDeviceOffline();
    int32_t UpdateTableDevice(const CDevInfo& cDev);
    int32_t SelectTableDeviceCount(const QString& sFilter);
    int32_t SelectTableDevice(CDevInfo& cDev, const QString& sUuid);
    int32_t SelectTableDevice(QVector<CDevInfo>* aDev, const QString& sFilter);

    // �¼���
    bool    CreateTableEvent();
    bool    InsertTableEvent(CEvtInfo& cEvt);
    int32_t UpdateTableEvent(CEvtInfo& cEvt);
    bool    SelectTableEvent(CEvtInfo& cEvt);
    int32_t SelectTableEventCount(const QString& sFilter);
    int32_t SelectTableEvent(QVector<CEvtInfo>* aEvt, const QString& sFilter);

    // �¼�����
    bool    CreateTableEventType();
    bool    InsertTableEventType(CEvtType& cET);
    bool    DeleteTableEventType(int32_t nEvent);
    bool    UpdateTableEventType(CEvtType& cET);
    bool    SelectTableEventType(QVector<CEvtType>& cVET);

    // ����ͳ��
    bool    CreateTablePCnt();
    bool    InsertTablePCnt(CEvtInfo& cEvt);
    int32_t SelectTablePCntSum(const QString& sFilter);

    // ���������
    bool    SelectTableAccess(QVector<CAesInfo>& aAes);
    // ת��������
    bool    SelectTableDispatch(QVector<CDispInfo>& aDisp);
    // �洢������
    bool    SelectTableStorage(QVector<CStoreInfo>& aStore, QString sDevID="");

    bool    CreateTableDiary();        // ��־
    bool    InsertTableDiary(QString sName, QString sLog);

private:
    QSqlDatabase        m_cDataBase;
};

#endif // CDATABASE_H
