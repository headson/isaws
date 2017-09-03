#include "cdatabase.h"

#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

CMyQueryModel::CMyQueryModel(QObject *parent)
  : QSqlQueryModel(parent) {
}

CMyQueryModel::~CMyQueryModel() {
}

QVariant CMyQueryModel::data(const QModelIndex &item, int role) const {
  QVariant value = QSqlQueryModel::data(item, role);
  if (role == Qt::TextAlignmentRole) {
    value = (Qt::AlignCenter);
    return value;
  }
  return value;
}

//////////////////////////////////////////////////////////////////////////
CMyTableModel::CMyTableModel(QObject *parent)
  : QSqlTableModel(parent) {
}

CMyTableModel::~CMyTableModel() {
}

QVariant CMyTableModel::data(const QModelIndex &item, int role) const {
  QVariant value = QSqlQueryModel::data(item, role);
  if (role == Qt::TextAlignmentRole) {
    value = Qt::AlignCenter;
    return value;
  }
  return value;
}

//////////////////////////////////////////////////////////////////////////
CDataBase::CDataBase(QObject *parent)
  : QObject(parent) {
  if (m_cDataBase.isOpen()) {
    m_cDataBase.close();
  }
}

CDataBase::~CDataBase() {
  if (m_cDataBase.isOpen()) {
    m_cDataBase.close();
  }
}

bool CDataBase::Init(QString sPath) {
  m_cDataBase = QSqlDatabase::addDatabase("QSQLITE");
  m_cDataBase.setDatabaseName(sPath);
  if (!m_cDataBase.open()) {
    qDebug() << m_cDataBase.lastError() << " failed to connect." << endl;
    return false;
  }

  CreateTableAdmin();     // 管理员表
  CreateTableDiary();     // 日志
  CreateTableDevice();    // 设备列表
  
  UpdateTableDeviceOffline();
  return true;
}

bool CDataBase::Init(QString sHost, int32_t nPort/*=3306*/) {
  m_cDataBase = QSqlDatabase::addDatabase("QMYSQL");
  m_cDataBase.setHostName(sHost);
  m_cDataBase.setPort(nPort);
  m_cDataBase.setDatabaseName(tr("whs_service"));
  m_cDataBase.setUserName("thinkwatch");
  m_cDataBase.setPassword("201920202021");

  if (!m_cDataBase.open()) {
    qDebug() << m_cDataBase.lastError() << " failed to connect." << endl;
    return false;
  }
  return true;
}

// 事务开始
bool CDataBase::Transaction() {
  if (m_cDataBase.isOpen()) {
    return m_cDataBase.transaction();
  }

  return false;
}

// 事务结束
bool CDataBase::Commit() {
  if (m_cDataBase.isOpen()) {
    return m_cDataBase.commit();
  }
  return false;
}

QSqlQueryModel* CDataBase::GetQueryModel() {
  CMyQueryModel* model = new CMyQueryModel();
  if (!model) {
    return NULL;
  }

  return model;
}

QSqlTableModel* CDataBase::GetTableModel() {
  //CMyTableModel* model = new CMyTableModel();
  QSqlTableModel* model = new QSqlTableModel();
  if (!model) {
    return NULL;
  }

  return model;
}

///管理员//////////////////////////////////////////////////////////////////////
bool CDataBase::CreateTableAdmin() {
  bool bRet = false;
  QString sSql = "CREATE TABLE IF NOT EXISTS admin ("
                 "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "USERNAME varchar(32) NOT NULL,"
                 "PASSWORD varchar(256) NOT NULL,"
                 "RIGHT_ID int(10) NOT NULL DEFAULT '1000',"
                 "NOTE varchar(255) DEFAULT NULL"
                 ")";
  QSqlQuery query;
  bRet = query.exec(sSql);
  if (bRet == true) {
    if (query.exec("SELECT * FROM admin")) {
      while (query.next()) {
        return true;
      }

      CUsrInfo cAdm;
      cAdm.username_ = "admin";
      cAdm.password_ = "12345678";
      InsertTableAdmin(cAdm);
    }

    bRet = true;
  } else {
    qDebug() << "CreateTableAdmin failed " << query.lastError() << endl;
  }
  return bRet;
}

bool CDataBase::CheckLogin(QString sUsr, QString sPwd) {
  QSqlQuery query;
  bool bRet = false;

  QString sSql = "SELECT USERNAME,PASSWORD1,PASSWORD2,"
                 "COUNTRY,PHONE,RIGHT_ID,NOTE,LOGIN_IP,LOGIN_MAC "
                 "FROM admin "
                 "WHERE USERNAME='" + sUsr + "' AND PASSWORD1='" + sPwd + "'";
  //sSql + " AND LOGIN_MAC='"+sMac+"'";
  bRet = query.exec(sSql);
  if (bRet) {
    bRet = false;
    while (query.next()) {
      bRet = true;
      break;
    }
  }
  if (!bRet) {
    qDebug() << "CheckLogin failed " << query.lastError() << endl;
  }
  return bRet;
}

bool CDataBase::DeleteTableAdmin(QString& sName) {
  QSqlQuery query;
  bool bRet = false;

  QString sSql = "DELETE FROM admin WHERE USERNAME='" + sName + "'";
  bRet = query.exec(sSql);

  if (!bRet) {
    qDebug() << "DeleteTableAdmin failed " << query.lastError() << endl;
  }
  return bRet;
}

bool CDataBase::InsertTableAdmin(CUsrInfo& cAdm) {
  if (cAdm.username_.isEmpty()
      || cAdm.password_.isEmpty()) {
    return false;
  }

  QSqlQuery query;
  bool bRet = false;
  QString sSql = "INSERT INTO admin(USERNAME,PASSWORD,RIGHT_ID,NOTE) "
                 " VALUES ('" + cAdm.username_ + "','" + cAdm.password_ + "'"
                 ",'" + QString::number(cAdm.right_id_) + "'"
                 ",'" + cAdm.note_ + "')";

  bRet = query.exec(sSql);
  if (bRet == false) {
    qDebug() << "ReplaceTableAdmin failed " << query.lastError() << endl;
  }
  return bRet;
}

bool CDataBase::UpdateTableAdmin(CUsrInfo& cAdm) {
  if (cAdm.username_.isEmpty()
      || cAdm.password_.isEmpty()) {
    return false;
  }

  QSqlQuery query;
  bool bRet = false;
  QString sSql = "UPDATE admin SET USERNAME='" + cAdm.username_ + "',"
                 "PASSWORD1='" + cAdm.password_ + "',"
                 "RIGHT_ID=" + QString::number(cAdm.right_id_) +
                 ",NOTE='" + cAdm.note_ + "' WHERE USERNAME='" + cAdm.username_ + "'";

  bRet = query.exec(sSql);
  if (bRet == false) {
    qDebug() << "ReplaceTableAdmin failed " << query.lastError() << endl;
  }
  return bRet;
}

///设备////////////////////////////////////////////////////////////////////////
bool CDataBase::CreateTableDevice() {
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

int32_t CDataBase::DeleteTableDevice(QString& sUuid) {
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

int32_t CDataBase::InsertTableDevice(const CDevInfo& cDev) {
  /*if (cDev.id_.isEmpty()) {
    return -1;
  }

  QSqlQuery query;
  QString sSql = "INSERT INTO device(ID,CHECK_CODE,DEVICE_TYPE_ID,"
                 "NAME,MODULE,ISLOCK,STORE_ID,DISPATCH_ID";
  if (!cDev.install_addr_.isEmpty()
      && cDev.install_addr_.compare("NULL")) {
    sSql += ",INSTALL_ADDR";
  }
  sSql += ") ";
  sSql += "VALUES ('" + cDev.id_ + "','" + cDev.check_code_ + "',"
          "" + QString::number(cDev.device_type_id_) + ",'" + cDev.name_ + "',"
          "'" + cDev.module_ + "'," + QString::number(cDev.islock_) + ","
          "'" + cDev.store_id_ + "','" + cDev.dispatch_id_ + "'";
  if (!cDev.install_addr_.isEmpty()
      && cDev.install_addr_.compare("NULL")) {
    sSql += ",'" + cDev.install_addr_ + "'";
  }
  sSql += ")";

  bool bRet = query.exec(sSql);
  if (bRet == false) {
    qDebug() << "InsertTableDevice failed " << query.lastError() << endl;
    return -1;
  }*/
  return 0;
}

int32_t CDataBase::UpdateTableDeviceOffline() {
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

int32_t CDataBase::UpdateTableDevice(const CDevInfo& cDev) {
  /*if (cDev.id_.isEmpty()) {
    return -1;
  }*/

  QSqlQuery query;
  bool bRet = false;

  /*QString sSql = "UPDATE device SET IS_ONLINE=" + QString::number(cDev.is_online_) + "";
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
  }*/
  //qDebug("%s.\n", sSql.toStdString().c_str());
  return 0;
}

int32_t CDataBase::SelectTableDevice(CDevInfo& cDev, const QString& sUuid) {
  QSqlQuery query;
  bool bRet = false;

  QString sSql = "SELECT d.ID,d.CHECK_CODE,d.DEVICE_TYPE_ID,d.NAME,d.MODULE,"
                 "d.ISLOCK,d.VERSION,d.HARDWARE,d.TIMEZONE,d.IS_ONLINE,d.IVA_STATUS,d.IP,d.MSG_PORT,"
                 "d.VDO_PORT,d.ADO_PORT,d.WEB_PORT,d.UPDATE_PORT,d.ACCESS_IP,d.STORE_ID,d.DISPATCH_ID,"
                 "d.ORGAN,d.HEART_CYCLE,d.INSTALL_ADDR,d.NOTE,d.PHOTO "
                 "FROM device d WHERE d.ID='" + sUuid + "'";
  bRet = query.exec(sSql);
  if (bRet) {
    if (query.next()) {
      int32_t nIndex = 0;
      /*cDev.id_ = query.value(nIndex++).toString();
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
      cDev.photo_ = query.value(nIndex++).toString();*/
    }
  }
  if (!bRet) {
    qDebug() << "SelectTableDevice failed " << query.lastError() << endl;
    return -1;
  }
  return 0;
}

int32_t CDataBase::SelectTableDeviceCount(const QString& sFilter) {
  QSqlQuery query;
  bool bRet = false;

  QString sSql = "SELECT count(d.ID) FROM device d " + sFilter;
  bRet = query.exec(sSql);
  if (bRet) {
    if (query.next())  {
      return query.value(0).toInt();
    }
  }
  return -1;
}

int32_t CDataBase::SelectTableDevice(QVector<CDevInfo>* aDev, const QString& sFilter) {
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
  if (bRet) {
    while (query.next()) {
      CDevInfo cDev;
      int32_t nIndex = 0;

      /*cDev.id_ = query.value(nIndex++).toString();
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
      cDev.photo_= query.value(nIndex++).toString();*/
      aDev->push_back(cDev);
    }
  }
  if (!bRet) {
    qDebug() << "SelectTableDevice failed " << query.lastError() << endl;
    return -1;
  }
  return 0;
}

///日志//////////////////////////////////////////////////////////////////////////
bool CDataBase::CreateTableDiary() {
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
  } else {
    qDebug() << "CreateTableDiary failed " << query.lastError() << endl;
  }
  return bRet;
}

bool CDataBase::InsertTableDiary(QString sName, QString sLog) {
  if (sName.isEmpty()
      || sLog.isEmpty()) {
    return false;
  }

  QSqlQuery query;
  bool bRet = false;

  QString sSql = "INSERT INTO diary(USERNAME, TIME, LOG) VALUES ("
                 "'" + sName + "',"
                 "'" + QDateTime::currentDateTime().toString() + "','" + sLog + "')";

  bRet = query.exec(sSql);
  if (bRet == false) {
    qDebug() << "InsertTableDiary failed " << query.lastError() << endl;
  }
  return bRet;
}





