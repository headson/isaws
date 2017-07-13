#ifndef CDATABASE_H
#define CDATABASE_H

#include "appinc.h"
#include <stdint.h>

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlTableModel>

#include "cusrinfo.h"
#include "cdevinfo.h"

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
    bool CheckLogin(QString usr, QString pwd);
    bool DeleteTableAdmin(QString& usr);
    bool InsertTableAdmin(CUsrInfo& info);
    bool UpdateTableAdmin(CUsrInfo& info);

    // �豸��
    bool    CreateTableDevice();
    int32_t DeleteTableDevice(QString& sUuid);
    int32_t InsertTableDevice(const CDevInfo& cDev);
    int32_t UpdateTableDeviceOffline();
    int32_t UpdateTableDevice(const CDevInfo& cDev);
    int32_t SelectTableDeviceCount(const QString& sFilter);
    int32_t SelectTableDevice(CDevInfo& cDev, const QString& sUuid);
    int32_t SelectTableDevice(QVector<CDevInfo>* aDev, const QString& sFilter);

    bool    CreateTableDiary();        // ��־
    bool    InsertTableDiary(QString sName, QString sLog);

private:
    QSqlDatabase        m_cDataBase;
};

#endif // CDATABASE_H
