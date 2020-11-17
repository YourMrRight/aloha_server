#pragma once

#include <QtWidgets/QWidget>
#include "ui_Aloha_server.h"
#include "TcpServer.h"
#include <QSqlQueryModel>
#include <QTimer>
#include <QUdpSocket>
class Aloha_server : public QDialog
{
    Q_OBJECT

public:
    Aloha_server(QWidget *parent = Q_NULLPTR);
private:
    void initTcpSocket();
    void initUdpSocket();		//初始化UDP
    bool connectMySql();
    //void setDepNameMap();
    void setStatusMap();
    void setOnlineMap();
   // int getCompDepID();	//获取群号
    void updateTableData(int employeeID = 0);
    QTimer* m_timer;	//定时刷新数据
private slots:
    void onUDPbroadMsg(QByteArray& btData);
    void onRefresh();
    void on_queryIDBtn_clicked();	//根据用户账号筛选
    void on_logoutBtn_clicked();	//注销用户账号
    void on_selectPictureBtn_clicked();	//选择图片（员工的寸照）
    void on_addBtn_clicked();	//新增员工
private:
    Ui::Aloha_server ui;
    int m_employeeID;	//账号
    QMap<QString, QString> m_statusMap;	//状态
    QString m_pixPath;	//头像路径
    QMap<QString, QString>m_depNameMap;	//名称
    QMap<QString, QString>m_onlineMap;	//在线

    QSqlQueryModel m_queryInfoModel;//查询所有用户的信息模型
    TcpServer* m_tcpServer;
    QUdpSocket* m_udpSender;//udp广播
};
