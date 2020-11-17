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
    void initUdpSocket();		//��ʼ��UDP
    bool connectMySql();
    //void setDepNameMap();
    void setStatusMap();
    void setOnlineMap();
   // int getCompDepID();	//��ȡȺ��
    void updateTableData(int employeeID = 0);
    QTimer* m_timer;	//��ʱˢ������
private slots:
    void onUDPbroadMsg(QByteArray& btData);
    void onRefresh();
    void on_queryIDBtn_clicked();	//�����û��˺�ɸѡ
    void on_logoutBtn_clicked();	//ע���û��˺�
    void on_selectPictureBtn_clicked();	//ѡ��ͼƬ��Ա���Ĵ��գ�
    void on_addBtn_clicked();	//����Ա��
private:
    Ui::Aloha_server ui;
    int m_employeeID;	//�˺�
    QMap<QString, QString> m_statusMap;	//״̬
    QString m_pixPath;	//ͷ��·��
    QMap<QString, QString>m_depNameMap;	//����
    QMap<QString, QString>m_onlineMap;	//����

    QSqlQueryModel m_queryInfoModel;//��ѯ�����û�����Ϣģ��
    TcpServer* m_tcpServer;
    QUdpSocket* m_udpSender;//udp�㲥
};
