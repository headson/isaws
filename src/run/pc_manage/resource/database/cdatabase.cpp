#include "stdafx.h"
#include "cdatabase.h"

#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

CMyQueryModel::CMyQueryModel(QObject *parent)
: QSqlQueryModel(parent)
{
}

CMyQueryModel::~CMyQueryModel()
{
}

QVariant CMyQueryModel::data(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item, role);
    if (role == Qt::TextAlignmentRole)
    {
        value = (Qt::AlignCenter);
        return value;
    }
    return value;
}

//////////////////////////////////////////////////////////////////////////
CMyTableModel::CMyTableModel(QObject *parent)
: QSqlTableModel(parent)
{
}

CMyTableModel::~CMyTableModel()
{
}

QVariant CMyTableModel::data(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item, role);
    if (role == Qt::TextAlignmentRole)
    {
        value = Qt::AlignCenter;
        return value;
    }
    return value;
}

//////////////////////////////////////////////////////////////////////////
CDataBase::CDataBase(QObject *parent)
: QObject(parent)
{
    if (m_cDataBase.isOpen()) {
        m_cDataBase.close();
    }
}

CDataBase::~CDataBase()
{
    if (m_cDataBase.isOpen()) {
        m_cDataBase.close();
    }
}

bool CDataBase::Init(QString sPath)
{
    m_cDataBase = QSqlDatabase::addDatabase("QSQLITE");
    m_cDataBase.setDatabaseName(sPath);
    if (!m_cDataBase.open())
    {
        qDebug() << m_cDataBase.lastError() << " failed to connect." << endl;
        return false;
    }

    CreateTableAdmin();     // 管理员表
    CreateTableDiary();     // 日志
    CreateTableEvent();     // 事件
    CreateTableDevice();    // 设备列表
    CreateTableEventType(); // 事件类型
    CreateTablePCnt();      // 人数统计

    UpdateTableDeviceOffline();
    return true;
}

bool CDataBase::Init(QString sHost, int32_t nPort/*=3306*/)
{
    m_cDataBase = QSqlDatabase::addDatabase("QMYSQL");
    m_cDataBase.setHostName(sHost);
    m_cDataBase.setPort(nPort);
    m_cDataBase.setDatabaseName(tr("whs_service"));
    m_cDataBase.setUserName("thinkwatch");
    m_cDataBase.setPassword("201920202021");

    if (!m_cDataBase.open())
    {
        qDebug() << m_cDataBase.lastError() << " failed to connect." << endl;
        return false;
    }
    return true;
}

// 事务开始
bool CDataBase::Transaction()
{
    if (m_cDataBase.isOpen()) {
        return m_cDataBase.transaction();
    }

    return false;
}

// 事务结束
bool CDataBase::Commit()
{
    if (m_cDataBase.isOpen()) {
        return m_cDataBase.commit();
    }
    return false;
}

QSqlQueryModel* CDataBase::GetQueryModel()
{
    CMyQueryModel* model = new CMyQueryModel();
    if (!model) {
        return NULL;
    }

    return model;
}

QSqlTableModel* CDataBase::GetTableModel()
{
    //CMyTableModel* model = new CMyTableModel();
    QSqlTableModel* model = new QSqlTableModel();
    if (!model) {
        return NULL;
    }

    return model;
}

///管理员//////////////////////////////////////////////////////////////////////
bool CDataBase::CreateTableAdmin()
{
    bool bRet = false;
    QString sSql = "CREATE TABLE IF NOT EXISTS admin ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "USERNAME varchar(31) NOT NULL,"
        "PASSWORD1 varchar(255) NOT NULL,"
        "PASSWORD2 varchar(255) NOT NULL,"
        "COUNTRY varchar(12) DEFAULT NULL,"
        "PHONE varchar(20) NOT NULL,"
        "LOGIN_IP varchar(255) DEFAULT NULL,"
        "LOGIN_MAC varchar(255) DEFAULT NULL,"
        "RIGHT_ID int(10) NOT NULL DEFAULT '1000',"
        "NOTE varchar(255) DEFAULT NULL"
        ")";
    QSqlQuery query;
    bRet = query.exec(sSql);
    if (bRet == true)
    {
        if (query.exec("SELECT * FROM admin"))
        {
            while (query.next()) { return true; }

            CAdmInfo cAdm;
            cAdm.username_ = "admin";
            cAdm.password1_ = "12345678";
            cAdm.password2_ = "123456789";
            cAdm.login_mac_ = "";
            InsertTableAdmin(cAdm);
        }

        bRet = true;
    }
    else
    {
        qDebug() << "CreateTableAdmin failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::CheckLogin(QString sUsr, QString sPwd1, QString sPwd2, QString sMac)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "SELECT USERNAME,PASSWORD1,PASSWORD2,"
        "COUNTRY,PHONE,RIGHT_ID,NOTE,LOGIN_IP,LOGIN_MAC "
        "FROM admin "
        "WHERE USERNAME='" + sUsr + "' AND PASSWORD1='" + sPwd1 + "'"
        " AND PASSWORD2='" + sPwd2 + "'";
    //sSql + " AND LOGIN_MAC='"+sMac+"'";
    bRet = query.exec(sSql);
    if (bRet)
    {
        bRet = false;
        while (query.next())
        {
            bRet = true;
            break;
        }
    }
    if (!bRet)
    {
        qDebug() << "CheckLogin failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::DeleteTableAdmin(QString& sName)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "DELETE FROM admin WHERE USERNAME='" + sName + "'";
    bRet = query.exec(sSql);

    if (!bRet)
    {
        qDebug() << "DeleteTableAdmin failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::InsertTableAdmin(CAdmInfo& cAdm)
{
    if (cAdm.username_.isEmpty()
        || cAdm.password1_.isEmpty()
        || cAdm.password2_.isEmpty()) {
        return false;
    }

    QSqlQuery query;
    bool bRet = false;
    QString sSql = "INSERT INTO admin(USERNAME,PASSWORD1,PASSWORD2,"
        "COUNTRY,PHONE,LOGIN_IP,LOGIN_MAC,RIGHT_ID,NOTE) "
        " VALUES ('" + cAdm.username_ + "','" + cAdm.password1_ + "'"
        ",'" + cAdm.password2_ + "', '" + cAdm.country_ + "','" + cAdm.phone_ + "'"
        ",'" + cAdm.login_ip_ + "','" + cAdm.login_mac_ + "','" + QString::number(cAdm.right_id_) + "'"
        ",'" + cAdm.note_ + "')";

    bRet = query.exec(sSql);
    if (bRet == false) {
        qDebug() << "ReplaceTableAdmin failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::UpdateTableAdmin(CAdmInfo& cAdm)
{
    if (cAdm.username_.isEmpty()
        || cAdm.password1_.isEmpty()
        || cAdm.password2_.isEmpty()) {
        return false;
    }

    QSqlQuery query;
    bool bRet = false;
    QString sSql = "UPDATE admin SET USERNAME='" + cAdm.username_ + "',"
        "PASSWORD1='" + cAdm.password1_ + "',PASSWORD2='" + cAdm.password2_ + "',"
        "COUNTRY='" + cAdm.country_ + "',PHONE='" + cAdm.phone_ + "',"
        "LOGIN_IP='" + cAdm.login_ip_ + "',LOGIN_MAC='" + cAdm.login_mac_ + "',"
        "RIGHT_ID=" + QString::number(cAdm.right_id_) + ",NOTE='" + cAdm.note_ + "'"
        " WHERE USERNAME='" + cAdm.username_ + "'";

    bRet = query.exec(sSql);
    if (bRet == false) {
        qDebug() << "ReplaceTableAdmin failed " << query.lastError() << endl;
    }
    return bRet;
}

///用户////////////////////////////////////////////////////////////////////////
// 通过设备ID，查找管理员
bool CDataBase::SelectTableUser(QVector<CUsrInfo>& aUsr, QString sDevID, bool bAdm)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "select u.ID,u.USERNAME,u.PASSWORD,ud.USER_RIGHT_ID "
        "from user u "
        "LEFT JOIN user_device ud on ud.USER_ID = u.ID "
        "WHERE ud.DEVICE_ID = '" + sDevID + "' ";
    if (bAdm) {
        sSql += " and ud.USER_RIGHT_ID=1000 ";
    }

    bRet = query.exec(sSql);
    if (bRet)
    {
        if (query.next())
        {
            CUsrInfo cUsr;

            int32_t nIndex = 0;
            cUsr.id_ = query.value(nIndex++).toString();
            cUsr.username_ = query.value(nIndex++).toString();
            cUsr.password_ = query.value(nIndex++).toString();
            cUsr.user_right_id_ = query.value(nIndex++).toInt();

            aUsr.push_back(cUsr);
        }
    }
    if (!bRet)
    {
        qDebug() << "SelectTableUser failed " << query.lastError() << endl;
    }
    return bRet;
}

int32_t CDataBase::SelectTableUserCount(const QString& sFilter)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "SELECT count(u.ID) FROM user u " + sFilter;
    bRet = query.exec(sSql);
    if (bRet)
    {
        if (query.next())  {
            return query.value(0).toInt();
        }
    }
    return -1;
}

bool CDataBase::SelectTableUser(CUsrInfo& cUsr, const QString& sUsername)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "SELECT u.ID,u.USERNAME,u.PASSWORD,"
        "u.COUNTRYCODE,u.PHONE,u.REG_TIME,u.IS_ONLINE,"
        "u.ADDRESS,u.PUSH_ID,u.DEVICE_TYPE_ID "
        "FROM user u WHERE u.USERNAME='" + sUsername + "'";
    bRet = query.exec(sSql);
    if (bRet)
    {
        while (query.next()) {
            //int32_t nIndex = 0;
            //cDev.id_            = query.value(nIndex++).toString();
            //cDev.check_code_    = query.value(nIndex++).toString();
            //cDev.device_type_id_= query.value(nIndex++).toInt();
        }
    }
    if (!bRet) {
        qDebug() << "SelectTableUser failed " << query.lastError() << endl;
    }
    return bRet;
}

///设备////////////////////////////////////////////////////////////////////////
bool CDataBase::CreateTableDevice()
{
    bool bRet = false;
    QString sSql = "CREATE TABLE IF NOT EXISTS device ("
        "ID varchar(31) PRIMARY KEY NOT NULL,"
        "CHECK_CODE varchar(31) NOT NULL DEFAULT '12345678',"
        "DEVICE_TYPE_ID int(10) DEFAULT '1100',"
        "NAME varchar(255) DEFAULT NULL,"
        "MODULE varchar(64) DEFAULT NULL,"
        "ISLOCK int(10) DEFAULT '0',"
        "VERSION int(10) DEFAULT '0',"
        "HARDWARE int(10) DEFAULT '0',"
        "TIMEZONE int(10)  DEFAULT '0',"
        "IS_ONLINE int(1) DEFAULT '0',"
        "IVA_STATUS varchar(64) DEFAULT NULL,"
        "IP varchar(19) DEFAULT NULL,"
        "MSG_PORT int(10) DEFAULT '0',"
        "VDO_PORT int(10) DEFAULT '0',"
        "ADO_PORT int(10) DEFAULT '0',"
        "WEB_PORT int(10) DEFAULT '0',"
        "UPDATE_PORT int(10) DEFAULT '0',"
        "ACCESS_IP varchar(19) DEFAULT NULL,"
        "STORE_ID varchar(31) DEFAULT 'STORE00000000000001',"
        "DISPATCH_ID varchar(31) DEFAULT 'DISPATCH00000000000001',"
        "ORGAN varchar(31) DEFAULT NULL,"
        "HEART_CYCLE int(10) DEFAULT '120',"
        "INSTALL_ADDR varchar(255) DEFAULT NULL,"
        "NOTE varchar(255) DEFAULT NULL,"
        "PHOTO mediumtext"
        ")";

    QSqlQuery query;
    bRet = query.exec(sSql);
    if (!bRet) {
        qDebug() << "CreateTableDevice failed " << query.lastError() << endl;
    }
    query.exec("UPDATE device SET IS_ONLINE=0");
    return bRet;
}

int32_t CDataBase::DeleteTableDevice(QString& sUuid)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "DELETE FROM device WHERE ID='" + sUuid + "'";
    bRet = query.exec(sSql);

    if (!bRet) {
        qDebug() << "DeleteTableDevice failed " << query.lastError() << endl;
        return -1;
    }
    return 0;
}

int32_t CDataBase::InsertTableDevice(const CDevInfo& cDev)
{
    if (cDev.id_.isEmpty()) {
        return -1;
    }

    QSqlQuery query;
    QString sSql = "INSERT INTO device(ID,CHECK_CODE,DEVICE_TYPE_ID,"
        "NAME,MODULE,ISLOCK,STORE_ID,DISPATCH_ID";
    if (!cDev.install_addr_.isEmpty()
        && cDev.install_addr_.compare("NULL"))
    {
        sSql += ",INSTALL_ADDR";
    }
    sSql += ") ";
    sSql += "VALUES ('" + cDev.id_ + "','" + cDev.check_code_ + "',"
        "" + QString::number(cDev.device_type_id_) + ",'" + cDev.name_ + "',"
        "'" + cDev.module_ + "'," + QString::number(cDev.islock_) + ","
        "'" + cDev.store_id_ + "','" + cDev.dispatch_id_ + "'";
    if (!cDev.install_addr_.isEmpty()
        && cDev.install_addr_.compare("NULL"))
    {
        sSql += ",'" + cDev.install_addr_ + "'";
    }
    sSql += ")";

    bool bRet = query.exec(sSql);
    if (bRet == false) {
        qDebug() << "InsertTableDevice failed " << query.lastError() << endl;
        return -1;
    }
    return 0;
}

int32_t CDataBase::UpdateTableDeviceOffline()
{
    QString sSql = "UPDATE device SET IS_ONLINE=0";

    QSqlQuery query;
    bool bRet = query.exec(sSql);
    if (!bRet) {
        qDebug() << "UpdateTableDevice failed " << query.lastError() << endl;
        return -1;
    }
    qDebug("%s.\n", sSql.toStdString().c_str());
    return 0;
}

int32_t CDataBase::UpdateTableDevice(const CDevInfo& cDev)
{
    if (cDev.id_.isEmpty()) {
        return -1;
    }

    QSqlQuery query;
    bool bRet = false;

    QString sSql = "UPDATE device SET IS_ONLINE=" + QString::number(cDev.is_online_) + "";
    if (cDev.check_code_ != "NULL") {
        sSql += ", CHECK_CODE='" + cDev.check_code_ + "'";
    }
    if (cDev.device_type_id_ != -1) {
        sSql += ", DEVICE_TYPE_ID=" + QString::number(cDev.device_type_id_) + "";
    }
    if (cDev.name_ != "NULL") {
        sSql += ", NAME='" + cDev.name_ + "'";
    }
    if (cDev.module_ != "NULL") {
        sSql += ", MODULE='" + cDev.module_ + "'";
    }
    if (cDev.iva_status_ != "NULL") {
        sSql += ", IVA_STATUS='" + cDev.iva_status_ + "'";
    }
    if (cDev.ip_ != "NULL") {
        sSql += ", IP='" + cDev.ip_ + "'";
    }
    if (cDev.vdo_port_ != -1) {
        sSql += ", VDO_PORT='" + QString::number(cDev.vdo_port_) + "'";
    }
    if (cDev.islock_ != -1) {
        sSql += ", ISLOCK='" + QString::number(cDev.islock_) + "'";
    }
    if (cDev.store_id_ != "NULL") {
        sSql += ", STORE_ID='" + cDev.store_id_ + "'";
    }
    if (cDev.dispatch_id_ != "NULL") {
        sSql += ", DISPATCH_ID='" + cDev.dispatch_id_ + "'";
    }
    if (cDev.install_addr_ != "NULL") {
        sSql += ", INSTALL_ADDR='" + cDev.install_addr_ + "'";
    }
    if (cDev.organ_ != "NULL") {
        sSql += ", ORGAN='" + cDev.organ_ + "'";
    }
    if (cDev.note_ != "NULL") {
        sSql += ", NOTE='" + cDev.note_ + "'";
    }
    if (cDev.photo_ != "NULL" && !cDev.photo_.isEmpty()) {
        sSql += ", PHOTO='" + cDev.photo_ + "'";
    }
    sSql += " WHERE ID='" + cDev.id_ + "'";

    bRet = query.exec(sSql);
    if (!bRet) {
        qDebug() << "UpdateTableDevice failed " << query.lastError() << endl;
        return -1;
    }
    //qDebug("%s.\n", sSql.toStdString().c_str());
    return 0;
}

int32_t CDataBase::SelectTableDevice(CDevInfo& cDev, const QString& sUuid)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "SELECT d.ID,d.CHECK_CODE,d.DEVICE_TYPE_ID,d.NAME,d.MODULE,"
        "d.ISLOCK,d.VERSION,d.HARDWARE,d.TIMEZONE,d.IS_ONLINE,d.IVA_STATUS,d.IP,d.MSG_PORT,"
        "d.VDO_PORT,d.ADO_PORT,d.WEB_PORT,d.UPDATE_PORT,d.ACCESS_IP,d.STORE_ID,d.DISPATCH_ID,"
        "d.ORGAN,d.HEART_CYCLE,d.INSTALL_ADDR,d.NOTE,d.PHOTO "
        "FROM device d WHERE d.ID='" + sUuid + "'";
    bRet = query.exec(sSql);
    if (bRet)
    {
        if (query.next())
        {
            int32_t nIndex = 0;
            cDev.id_ = query.value(nIndex++).toString();
            cDev.check_code_ = query.value(nIndex++).toString();
            cDev.device_type_id_ = query.value(nIndex++).toInt();
            cDev.name_ = query.value(nIndex++).toString();
            cDev.module_ = query.value(nIndex++).toString();
            cDev.islock_ = query.value(nIndex++).toInt();
            cDev.version_ = query.value(nIndex++).toInt();
            cDev.hardware_ = query.value(nIndex++).toInt();
            cDev.timezone_ = query.value(nIndex++).toInt();
            cDev.is_online_ = query.value(nIndex++).toInt();
            cDev.iva_status_ = query.value(nIndex++).toString();
            cDev.ip_ = query.value(nIndex++).toString();
            cDev.msg_port_ = query.value(nIndex++).toInt();
            cDev.vdo_port_ = query.value(nIndex++).toInt();
            cDev.ado_port_ = query.value(nIndex++).toInt();
            cDev.web_port_ = query.value(nIndex++).toInt();
            cDev.update_port_ = query.value(nIndex++).toInt();
            cDev.access_ip_ = query.value(nIndex++).toString();
            cDev.store_id_ = query.value(nIndex++).toString();
            cDev.dispatch_id_ = query.value(nIndex++).toString();
            cDev.organ_ = query.value(nIndex++).toString();
            cDev.heart_cycle_ = query.value(nIndex++).toInt();
            cDev.install_addr_ = query.value(nIndex++).toString();
            cDev.note_ = query.value(nIndex++).toString();
            cDev.photo_ = query.value(nIndex++).toString();
        }
    }
    if (!bRet){
        qDebug() << "SelectTableDevice failed " << query.lastError() << endl;
        return -1;
    }
    return 0;
}

int32_t CDataBase::SelectTableDeviceCount(const QString& sFilter)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "SELECT count(d.ID) FROM device d " + sFilter;
    bRet = query.exec(sSql);
    if (bRet)
    {
        if (query.next())  {
            return query.value(0).toInt();
        }
    }
    return -1;
}

int32_t CDataBase::SelectTableDevice(QVector<CDevInfo>* aDev, const QString& sFilter)
{
    QSqlQuery query;
    bool bRet = false;
    if (aDev == NULL) {
        return -1;
    }

    QString sSql = "SELECT d.ID,d.CHECK_CODE,d.DEVICE_TYPE_ID,d.NAME,d.MODULE,"
        "d.ISLOCK,d.VERSION,d.HARDWARE,d.TIMEZONE,d.IS_ONLINE,d.IVA_STATUS,d.IP,d.MSG_PORT,"
        "d.VDO_PORT,d.ADO_PORT,d.WEB_PORT,d.UPDATE_PORT,d.ACCESS_IP,d.STORE_ID,d.DISPATCH_ID,"
        "d.ORGAN,d.HEART_CYCLE,d.INSTALL_ADDR,d.NOTE,d.PHOTO "
        "FROM device d" + sFilter;
    bRet = query.exec(sSql);
    if (bRet)
    {
        while (query.next())
        {
            CDevInfo cDev;
            int32_t nIndex = 0;

            cDev.id_ = query.value(nIndex++).toString();
            cDev.check_code_ = query.value(nIndex++).toString();
            cDev.device_type_id_ = query.value(nIndex++).toInt();
            cDev.name_ = query.value(nIndex++).toString();
            cDev.module_ = query.value(nIndex++).toString();
            cDev.islock_ = query.value(nIndex++).toInt();
            cDev.version_ = query.value(nIndex++).toInt();
            cDev.hardware_ = query.value(nIndex++).toInt();
            cDev.timezone_ = query.value(nIndex++).toInt();
            cDev.is_online_ = query.value(nIndex++).toInt();
            cDev.iva_status_ = query.value(nIndex++).toString();
            cDev.ip_ = query.value(nIndex++).toString();
            cDev.msg_port_ = query.value(nIndex++).toInt();
            cDev.vdo_port_ = query.value(nIndex++).toInt();
            cDev.ado_port_ = query.value(nIndex++).toInt();
            cDev.web_port_ = query.value(nIndex++).toInt();
            cDev.update_port_ = query.value(nIndex++).toInt();
            cDev.access_ip_ = query.value(nIndex++).toString();
            cDev.store_id_ = query.value(nIndex++).toString();
            cDev.dispatch_id_ = query.value(nIndex++).toString();
            cDev.organ_ = query.value(nIndex++).toString();
            cDev.heart_cycle_ = query.value(nIndex++).toInt();
            cDev.install_addr_ = query.value(nIndex++).toString();
            cDev.note_ = query.value(nIndex++).toString();
            cDev.photo_= query.value(nIndex++).toString();
            aDev->push_back(cDev);
        }
    }
    if (!bRet) {
        qDebug() << "SelectTableDevice failed " << query.lastError() << endl;
        return -1;
    }
    return 0;
}

///事件///////////////////////////////////////////////////////////////////////
bool CDataBase::CreateTableEvent()
{
    bool bRet = false;

    QSqlQuery query;
    QString sSql = "CREATE TABLE IF NOT EXISTS event ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "TIME_POINT datetime NOT NULL DEFAULT '0000-00-00 00:00:00',"
        "TIME_END datetime DEFAULT NULL,"
        "EVENT int(10) NOT NULL DEFAULT '0',"
        "DEVICE_ID varchar(31) NOT NULL DEFAULT '',"
        "IDENT_USER varchar(31) DEFAULT NULL,"
        "NUMBER int(10) DEFAULT '0',"
        "LEVEL int(10) NOT NULL DEFAULT '0',"
        "USER_ID varchar(31) DEFAULT NULL,"
        "PROCESS int(10) DEFAULT '0',"
        "PHOTO mediumtext"
        ")";
    bRet = query.exec(sSql);
    if (!bRet) {
        qDebug() << "CreateTableEvent failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::InsertTableEvent(CEvtInfo& cEvt)
{
    if (cEvt.event_ == 0
        || cEvt.time_point_.isEmpty())
    {
        return false;
    }

    bool bRet = false;
    CEvtInfo cTEvt = cEvt;
    if (SelectTableEvent(cTEvt)) {
        if (cTEvt.level_ < cEvt.level_) {
            if (UpdateTableEvent(cEvt) == 0) {
                bRet = true;
            }
        }
    }
    else {
        QSqlQuery query;
        QString sSql = "INSERT INTO event(TIME_POINT,DEVICE_ID,EVENT,NUMBER,LEVEL";
        if (!cEvt.time_end_.isEmpty()) { sSql += ",TIME_END"; }
        if (!cEvt.ident_user_.isEmpty()) { sSql += ",IDENT_USER"; }
        if (!cEvt.user_id_.isEmpty()) { sSql += ",USER_ID"; }
        if (!cEvt.photo_.isEmpty()) { sSql += ",PHOTO"; }
        sSql += ") ";
        sSql += "VALUES ('" + cEvt.time_point_ + "','" + cEvt.device_id_ + "'"
            "," + QString::number(cEvt.event_) + "," + QString::number(cEvt.number_) + ""
            "," + QString::number(cEvt.level_) + "";
        if (!cEvt.time_end_.isEmpty()) { sSql += ",'" + cEvt.time_end_ + "'"; }
        if (!cEvt.ident_user_.isEmpty()) { sSql += ",'" + cEvt.ident_user_ + "'"; }
        if (!cEvt.user_id_.isEmpty()) { sSql += ",'" + cEvt.user_id_ + "'"; }
        if (!cEvt.photo_.isEmpty()) { sSql += ",'" + cEvt.photo_ + "'"; }
        sSql += ")";

        bRet = query.exec(sSql);
        if (bRet == false) {
            qDebug() << "InsertTableEvent failed " << query.lastError() << endl;
        }
    }
    return bRet;
}

int32_t CDataBase::UpdateTableEvent(CEvtInfo& cEvt)
{
    if (cEvt.event_ == 0
        || cEvt.device_id_.isEmpty()
        || cEvt.time_point_.isEmpty()) {
        return -1;
    }

    QSqlQuery query;
    bool bRet = false;

    QString sSql = "UPDATE event SET EVENT=" + QString::number(cEvt.event_) + ""
        ", LEVEL=" + QString::number(cEvt.level_) + "";

    if (cEvt.photo_ != "NULL"
        && !cEvt.photo_.isEmpty()) {
        sSql += ", PHOTO='" + cEvt.photo_ + "'";
    }
    if (cEvt.ident_user_ != "NULL"
        && !cEvt.ident_user_.isEmpty()) {
        sSql += ", IDENT_USER='" + cEvt.ident_user_ + "'";
    }
    if (cEvt.user_id_ != "NULL"
        && !cEvt.user_id_.isEmpty()) {
        sSql += ", USER_ID='" + cEvt.user_id_ + "'";
    }
    sSql += " WHERE TIME_POINT='" + cEvt.time_point_ + "'"
        " AND DEVICE_ID='" + cEvt.device_id_ + "'";
    //" AND EVENT="+QString::number(cEvt.event_)+""

    bRet = query.exec(sSql);
    if (!bRet) {
        qDebug() << "UpdateTableEvent failed " << query.lastError() << endl;
        return -2;
    }
    return 0;
}

bool CDataBase::SelectTableEvent(CEvtInfo& cEvt)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "select "
        " d.name as DEVICE_NAME, d.DEVICE_TYPE_ID,"
        " e.TIME_POINT, e.EVENT, e.LEVEL, e.DEVICE_ID,"
        " e.NUMBER, e.IDENT_USER, e.USER_ID,e.PROCESS,"
        " e.PHOTO"
        " from event e "
        " left join device d on d.id = e.DEVICE_ID ";

    sSql += "WHERE TIME_POINT='" + cEvt.time_point_ + "'"
        " AND DEVICE_ID = '" + cEvt.device_id_ + "'";
    //if (bMatchEvent) {
    //    sSql += " AND EVENT = "+QString::number(cEvt.event_)+"";
    //}

    bRet = query.exec(sSql);
    if (bRet)
    {
        bRet = false;
        while (query.next())
        {
            int32_t nIndex = 0;
            cEvt.device_name_   = query.value(nIndex++).toString();
            cEvt.device_type_   = query.value(nIndex++).toInt();
            cEvt.time_point_    = query.value(nIndex++).toDateTime().toString(DEF_TIME_STR);
            cEvt.event_         = query.value(nIndex++).toInt();
            cEvt.level_         = query.value(nIndex++).toInt();
            cEvt.device_id_     = query.value(nIndex++).toString();
            cEvt.number_        = query.value(nIndex++).toInt();
            cEvt.ident_user_    = query.value(nIndex++).toString();
            cEvt.user_id_       = query.value(nIndex++).toString();
            cEvt.process_       = query.value(nIndex++).toInt();
            cEvt.photo_         = query.value(nIndex++).toString();
            bRet = true;
        }
    }
    if (!bRet){
        qDebug() << "SelectTableEvent failed " << query.lastError() << endl;
    }
    return bRet;
}

int32_t CDataBase::SelectTableEventCount(const QString& sFilter)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "select COUNT(e.ID)"
        " from event e "
        " left join device d on d.id = e.DEVICE_ID " + sFilter;
    bRet = query.exec(sSql);
    if (bRet)
    {
        if (query.next())  {
            return query.value(0).toInt();
        }
    }
    return -1;
}

int32_t CDataBase::SelectTableEvent(QVector<CEvtInfo>* aEvt, const QString& sFilter)
{
    QSqlQuery query;
    bool bRet = false;
    if (aEvt == NULL) {
        return -1;
    }

    QString sSql = "select "
        " d.name as DEVICE_NAME, d.DEVICE_TYPE_ID,"
        " e.TIME_POINT, e.EVENT, e.DEVICE_ID, e.NUMBER,"
        " e.IDENT_USER, e.USER_ID,e.PROCESS,e.PHOTO"
        " from event e "
        " left join device d on d.id = e.DEVICE_ID ";
    sSql += sFilter;

    bRet = query.exec(sSql);
    if (bRet)
    {
        while (query.next())
        {
            CEvtInfo cEvt;

            int32_t nIndex = 0;
            cEvt.device_name_ = query.value(nIndex++).toString();
            cEvt.device_type_ = query.value(nIndex++).toInt();
            cEvt.time_point_ = query.value(nIndex++).toDateTime().toString(DEF_TIME_STR);
            cEvt.event_ = query.value(nIndex++).toInt();
            cEvt.device_id_ = query.value(nIndex++).toString();
            cEvt.number_ = query.value(nIndex++).toInt();
            cEvt.ident_user_ = query.value(nIndex++).toString();
            cEvt.user_id_ = query.value(nIndex++).toString();
            cEvt.process_ = query.value(nIndex++).toInt();
            cEvt.photo_ = query.value(nIndex++).toString();

            aEvt->push_back(cEvt);
        }
    }
    if (!bRet){
        qDebug() << "SelectTableEvent failed " << query.lastError() << endl;
        return -1;
    }
    return 0;
}

///事件类型///////////////////////////////////////////////////////////////////////
bool CDataBase::CreateTableEventType()
{
    bool bRet = false;

    QSqlQuery query;
    QString sSql = "CREATE TABLE IF NOT EXISTS event_type ("
        "ID INTEGER PRIMARY KEY,"
        "NAME varchar(255) NOT NULL DEFAULT '',"
        "BVIEW int(1) DEFAULT 0,"
        "BSEND int(1) DEFAULT 0,"
        "BPOPUP int(1) DEFAULT 0"
        ")";
    bRet = query.exec(sSql);
    if (bRet) {
        CEvtType cEt;
        cEt.nId_ = 10; cEt.sName_ = tr("抓拍");             cEt.bView_ = 0; cEt.bSend_ = 0; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 52; cEt.sName_ = tr("门户异常报警");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 53; cEt.sName_ = tr("门户异常预警");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 54; cEt.sName_ = tr("人员劫持报警");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 55; cEt.sName_ = tr("忘记反锁门提醒");   cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 56; cEt.sName_ = tr("正常进门");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 57; cEt.sName_ = tr("正常出门");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 58; cEt.sName_ = tr("人员路过");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 59; cEt.sName_ = tr("认证进门");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 60; cEt.sName_ = tr("认证出门");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 62; cEt.sName_ = tr("视线异常提醒");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 63; cEt.sName_ = tr("视线异常报警");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 64; cEt.sName_ = tr("撤防录像");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 65; cEt.sName_ = tr("人员劫持预警");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 66; cEt.sName_ = tr("门铃");             cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 67; cEt.sName_ = tr("非法闯入预警");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 68; cEt.sName_ = tr("非法闯入报警");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 69; cEt.sName_ = tr("远程开门");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 70; cEt.sName_ = tr("防拆报警");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 71; cEt.sName_ = tr("布防");             cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 72; cEt.sName_ = tr("撤防");             cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 73; cEt.sName_ = tr("设置日常模式");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 74; cEt.sName_ = tr("设置为外出模式");   cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 75; cEt.sName_ = tr("门未关报警");       cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 76; cEt.sName_ = tr("开门报警");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 77; cEt.sName_ = tr("开门事件");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 78; cEt.sName_ = tr("关门事件");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 79; cEt.sName_ = tr("门磁故障");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 80; cEt.sName_ = tr("锁孔视频丢失");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 81; cEt.sName_ = tr("人像视频丢失");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 83; cEt.sName_ = tr("窗户视频丢失");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 84; cEt.sName_ = tr("拾音器故障");       cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 85; cEt.sName_ = tr("扬声器故障");       cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 86; cEt.sName_ = tr("红外补光灯故障");   cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 87; cEt.sName_ = tr("人脸注册");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 88; cEt.sName_ = tr("环境视频丢失");     cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 1; InsertTableEventType(cEt);
        cEt.nId_ = 90; cEt.sName_ = tr("其他进门");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 98; cEt.sName_ = tr("中断事件");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
        cEt.nId_ = 99; cEt.sName_ = tr("其他事件");         cEt.bView_ = 1; cEt.bSend_ = 1; cEt.bPopup_ = 0; InsertTableEventType(cEt);
    }
    else {
        qDebug() << "CreateTableEventType failed " << query.lastError() << endl;
        return false;
    }
    return bRet;
}

bool CDataBase::InsertTableEventType(CEvtType& cET)
{
    if (cET.nId_ <= 0 || cET.sName_.isEmpty()) {
        return false;
    }

    QSqlQuery query;
    QString sSql = "INSERT INTO event_type(ID,NAME,BVIEW,"
        "BSEND,BPOPUP) VALUES (" + QString::number(cET.nId_) + ","
        "'" + cET.sName_ + "'," + QString::number(cET.bView_) + ","
        "" + QString::number(cET.bSend_) + "," + QString::number(cET.bPopup_) + ")";

    bool bRet = query.exec(sSql);
    if (bRet == false) {
        //qDebug() << "InsertTableEventType failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::DeleteTableEventType(int32_t nEvent)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "DELETE FROM event_type WHERE ID=" + QString::number(nEvent) + "";

    bRet = query.exec(sSql);
    if (!bRet) {
        qDebug() << "DeleteTableDevice failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::UpdateTableEventType(CEvtType& cEt)
{
    if (cEt.nId_ <= 0
        || cEt.sName_.isEmpty()) {
        return false;
    }

    QSqlQuery query;
    bool bRet = false;

    QString sSql = "UPDATE event_type SET ";
    sSql += " NAME='" + cEt.sName_ + "'";
    sSql += ", BVIEW=" + QString::number(cEt.bView_) + "";
    sSql += ", BSEND=" + QString::number(cEt.bSend_) + "";
    sSql += ", BPOPUP=" + QString::number(cEt.bPopup_) + "";
    sSql += " WHERE ID=" + QString::number(cEt.nId_) + "";
    bRet = query.exec(sSql);
    if (!bRet) {
        qDebug() << "UpdateTableEvent failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::SelectTableEventType(QVector<CEvtType>& cVET)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "SELECT ID,NAME,BVIEW,BSEND,BPOPUP"
        " FROM event_type";
    bRet = query.exec(sSql);
    if (bRet)
    {
        while (query.next()) {
            CEvtType cEt;
            int32_t nIndex = 0;
            cEt.nId_ = query.value(nIndex++).toInt();
            cEt.sName_ = query.value(nIndex++).toString();
            cEt.bView_ = ((query.value(nIndex++).toInt() > 0) ? 1 : 0);
            cEt.bSend_ = ((query.value(nIndex++).toInt() > 0) ? 1 : 0);
            cEt.bPopup_ = ((query.value(nIndex++).toInt() > 0) ? 1 : 0);
            cVET.push_back(cEt);
        }
    }
    if (!bRet){
        qDebug() << "SelectTableAccess failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::CreateTablePCnt()
{
    bool bRet = false;

    QSqlQuery query;
    if (query.exec("CREATE TABLE IF NOT EXISTS people_count ( "
        "DEVICE_ID  VARCHAR(32) NOT NULL,"
        "DIST_TIME  DATETIME NOT NULL,"
        "IN_REL_CNT   INTEGER,"
        "IN_ABS_CNT   INTEGER,"
        "OUT_REL_CNT  INTEGER,"
        "OUT_ABS_CNT  INTEGER"
        ")"))
    {
        bRet = true;
    }
    else
    {
        qDebug() << "CreateTablePCnt failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::InsertTablePCnt(CEvtInfo& cEvt)
{
    if (cEvt.device_id_.isEmpty() 
        || cEvt.time_point_.isEmpty()) {
        return false;
    }

    QSqlQuery query;
    QString sSql = "INSERT INTO people_count(device_id,dist_time,in_rel_cnt,in_abs_cnt,out_rel_cnt,out_abs_cnt) "
        "VALUES ('"+cEvt.device_id_+"', '"+cEvt.time_point_+"', "
        +QString::number(cEvt.nInRelVal)+", "+QString::number(cEvt.nInAbsVal)+","
        +QString::number(cEvt.nOutRelVal)+", "+QString::number(cEvt.nOutAbsVal)+""
        ")";

    bool bRet = query.exec(sSql);
    if (bRet == false) {
        //qDebug() << "InsertTableEventType failed " << query.lastError() << endl;
    }

    emit RefreshPeopleCount();
    return bRet;
}

// 正：绝对，相对；反：绝对，相对
int32_t CDataBase::SelectTablePCntSum(const QString& sFilter)
{
    QSqlQuery query;
    bool bRet = false;
    QString sSql = "select sum(IN_ABS_CNT)+sum(OUT_ABS_CNT) "
        "from people_count " + sFilter;

    int32_t nRet = 0;
    bRet = query.exec(sSql);
    if (bRet) {
        if (query.next())  {
            nRet = query.value(0).toInt();
        }
    }
    return nRet;
}


///服务器/////////////////////////////////////////////////////////////////////////
bool CDataBase::SelectTableAccess(QVector<CAesInfo>& aAes)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "SELECT ID,NAME,ISLOCK,WAN_IP,LAN_IP,"
        "CLIENT_PORT,DISPATCH_PORT,TERM_PORT,"
        "ORGAN_ID,LOGIN_TIME,INSTALL_ADDR,NOTE "
        "FROM access";
    bRet = query.exec(sSql);
    if (bRet)
    {
        while (query.next())
        {
            CAesInfo cAes;
            int32_t nIndex = 0;

            cAes.id_ = query.value(nIndex++).toString();
            cAes.name_ = query.value(nIndex++).toString();
            cAes.islock_ = query.value(nIndex++).toInt();
            cAes.wan_ip_ = query.value(nIndex++).toString();
            cAes.lan_ip_ = query.value(nIndex++).toString();
            cAes.client_port_ = query.value(nIndex++).toInt();
            cAes.dispatch_port_ = query.value(nIndex++).toInt();
            cAes.term_port_ = query.value(nIndex++).toInt();
            cAes.organ_id_ = query.value(nIndex++).toString();
            cAes.install_addr_ = query.value(nIndex++).toString();
            cAes.note_ = query.value(nIndex++).toString();
            aAes.push_back(cAes);
        }
    }
    if (!bRet){
        qDebug() << "SelectTableAccess failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::SelectTableDispatch(QVector<CDispInfo>& aDisp)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "SELECT ID,NAME,WAN_IP,LAN_IP,VDO_PORT,"
        "MSG_PORT,OWNER_SHIP,LINK_TYPE,HEART_CYCLE,"
        "MAX_INPUT_CNT,MAX_OUTPUT_CNT,RESVD_OUTPUT_CNT,"
        "ORGAN_ID,INSTALL_ADDR,NOTE "
        "FROM dispatch";
    bRet = query.exec(sSql);
    if (bRet)
    {
        while (query.next())
        {
            CDispInfo cDisp;
            int32_t nIndex = 0;

            cDisp.id_ = query.value(nIndex++).toString();
            cDisp.name_ = query.value(nIndex++).toString();
            cDisp.wan_ip_ = query.value(nIndex++).toString();
            cDisp.lan_ip_ = query.value(nIndex++).toString();
            cDisp.vdo_port_ = query.value(nIndex++).toInt();
            cDisp.msg_port_ = query.value(nIndex++).toInt();
            cDisp.owner_ship_ = query.value(nIndex++).toString();
            cDisp.link_type_ = query.value(nIndex++).toString();
            cDisp.heart_cycle_ = query.value(nIndex++).toInt();
            cDisp.max_input_cnt_ = query.value(nIndex++).toInt();
            cDisp.max_output_cnt_ = query.value(nIndex++).toInt();
            cDisp.resvd_output_cnt_ = query.value(nIndex++).toInt();
            cDisp.organ_id_ = query.value(nIndex++).toString();
            cDisp.install_addr_ = query.value(nIndex++).toString();
            cDisp.note_ = query.value(nIndex++).toString();
            aDisp.push_back(cDisp);
        }
    }
    if (!bRet){
        qDebug() << "SelectTableAccess failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::SelectTableStorage(QVector<CStoreInfo>& aStore, QString sDevID)
{
    QSqlQuery query;
    bool bRet = false;

    QString sSql = "SELECT st.ID,st.NAME,st.WAN_IP,st.LAN_IP,st.VDO_PORT, "
        "st.MSG_PORT,st.OWNER_SHIP,st.LINK_TYPE,st.HEART_CYCLE, "
        "st.MAX_INPUT_CNT,st.MAX_OUTPUT_CNT,st.RESVD_OUTPUT_CNT, "
        "st.ORGAN_ID,st.INSTALL_ADDR,st.NOTE "
        "FROM store st ";
    if (!sDevID.isEmpty()) {
        sSql += "LEFT JOIN device d ON d.STORE_ID = st.ID ";
        sSql += "WHERE d.ID = '" + sDevID + "'";
    }
    bRet = query.exec(sSql);
    if (bRet)
    {
        while (query.next())
        {
            CStoreInfo cStore;
            int32_t nIndex = 0;

            cStore.id_ = query.value(nIndex++).toString();
            cStore.name_ = query.value(nIndex++).toString();
            cStore.wan_ip_ = query.value(nIndex++).toString();
            cStore.lan_ip_ = query.value(nIndex++).toString();
            cStore.vdo_port_ = query.value(nIndex++).toInt();
            cStore.msg_port_ = query.value(nIndex++).toInt();
            cStore.owner_ship_ = query.value(nIndex++).toString();
            cStore.link_type_ = query.value(nIndex++).toString();
            cStore.heart_cycle_ = query.value(nIndex++).toInt();
            cStore.max_input_cnt_ = query.value(nIndex++).toInt();
            cStore.max_output_cnt_ = query.value(nIndex++).toInt();
            cStore.resvd_output_cnt_ = query.value(nIndex++).toInt();
            cStore.organ_id_ = query.value(nIndex++).toString();
            cStore.install_addr_ = query.value(nIndex++).toString();
            cStore.note_ = query.value(nIndex++).toString();
            aStore.push_back(cStore);
        }
    }
    if (!bRet){
        qDebug() << "SelectTableAccess failed " << query.lastError() << endl;
    }
    return bRet;
}

///日志//////////////////////////////////////////////////////////////////////////
bool CDataBase::CreateTableDiary()
{
    bool bRet = false;

    QSqlQuery query;
    if (query.exec("CREATE TABLE IF NOT EXISTS diary ( "
        "ID         INTEGER         PRIMARY KEY AUTOINCREMENT NOT NULL, "   // 
        "USERNAME   VARCHAR(32)     NOT NULL, "                             // 用户名
        "TIME       DATETIME        NOT NULL, "                             // 时间
        "LOG        VARCHAR(255)    NULL"                                   // 日志
        ")"))
    {
        bRet = true;
    }
    else
    {
        qDebug() << "CreateTableDiary failed " << query.lastError() << endl;
    }
    return bRet;
}

bool CDataBase::InsertTableDiary(QString sName, QString sLog)
{
    if (sName.isEmpty()
        || sLog.isEmpty())
    {
        return false;
    }

    QSqlQuery query;
    bool bRet = false;

    QString sSql = "INSERT INTO diary(USERNAME, TIME, LOG) VALUES ("
        "'" + sName + "',"
        "'" + QDateTime::currentDateTime().toString(DEF_TIME_STR) + "','" + sLog + "')";

    bRet = query.exec(sSql);
    if (bRet == false) {
        qDebug() << "InsertTableDiary failed " << query.lastError() << endl;
    }
    return bRet;
}





