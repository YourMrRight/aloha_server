#include "Aloha_server.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QFileDialog>
#include <QDebug>
const int tcpPort = 8888;
const int gUdpPort = 6666;
Aloha_server::Aloha_server(QWidget *parent)
    : QDialog(parent)
	, m_pixPath("")
{
    ui.setupUi(this);

	if (!connectMySql())
	{
		QMessageBox::warning(NULL,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("�������ݿ�ʧ�ܣ�"));
		qDebug() << "available drivers:";
		QStringList drivers = QSqlDatabase::drivers();
		foreach(QString driver, drivers)
			qDebug() << driver;

		close();
		return;
	}

	//setDepNameMap();
	setStatusMap();
	setOnlineMap();

	m_queryInfoModel.setQuery("SELECT * FROM tab_user");
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//���ֻ��
	updateTableData();

	
	m_employeeID = 0;

	m_timer = new QTimer(this);
	m_timer->setInterval(200);
	m_timer->start();
	connect(m_timer, &QTimer::timeout, this, &Aloha_server::onRefresh);


	initTcpSocket();
	initUdpSocket();
}



void Aloha_server::initTcpSocket()
{
	m_tcpServer = new TcpServer(tcpPort);
	m_tcpServer->run();

	//�յ�tcp�ͻ��˷�������Ϣ�����udp�㲥
	connect(m_tcpServer, &TcpServer::signalTcpMsgComes,
		this, &Aloha_server::onUDPbroadMsg);
}

void Aloha_server::initUdpSocket()
{
	m_udpSender = new QUdpSocket(this);



}

bool Aloha_server::connectMySql()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setDatabaseName("aloha");	//���ݿ�����
	db.setHostName("124.71.111.16");//������
	db.setUserName("root");		//�û���
	db.setPassword("123456");	//����
	db.setPort(3306);			//�˿�

	if (db.open())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Aloha_server::setStatusMap()
{
	m_statusMap.insert(QStringLiteral("1"), QStringLiteral("��Ч"));
	m_statusMap.insert(QStringLiteral("0"), QStringLiteral("��ע��"));
}

void Aloha_server::setOnlineMap()
{
	m_onlineMap.insert(QStringLiteral("1"), QStringLiteral("����"));
	m_onlineMap.insert(QStringLiteral("2"), QStringLiteral("����"));
	m_onlineMap.insert(QStringLiteral("3"), QStringLiteral("����"));
}

void Aloha_server::updateTableData(int employeeID)
{
	ui.tableWidget->clear();

	//if (depID && depID != m_compDepID)//��ѯ����
	//{
	//	m_queryInfoModel.setQuery(QString("SELECT * FROM tab_employees WHERE departmentID = %1").arg(depID));
	//}
	if (employeeID)//��ȷ����
	{
		m_queryInfoModel.setQuery(QString("SELECT * FROM tab_user WHERE userAccount = %1").arg(employeeID));
	}
	else//��ѯ����
	{
		m_queryInfoModel.setQuery(QString("SELECT * FROM tab_user"));
	}

	int rows = m_queryInfoModel.rowCount();			//���������ܼ�¼����
	int columns = m_queryInfoModel.columnCount();	//�����������ֶ�����

	QModelIndex index;//ģ������

	//���ñ�����������
	ui.tableWidget->setRowCount(rows);
	ui.tableWidget->setColumnCount(columns);

	//���ñ�ͷ
	QStringList headers;
	headers 
		<< QStringLiteral("�û��˺�")
		<< QStringLiteral("�û��ǳ�")
		<< QStringLiteral("�û�ǩ��")
		<< QStringLiteral("�û�״̬")
		<< QStringLiteral("�û�ͷ��")
		<< QStringLiteral("����״̬");
	ui.tableWidget->setHorizontalHeaderLabels(headers);

	//�����еȿ�
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			index = m_queryInfoModel.index(i, j);//�С���
			QString strData = m_queryInfoModel.data(index).toString();//��ȡi��j�е�����

			//��ȡ�ֶ�����
			QSqlRecord record = m_queryInfoModel.record(i);//��ǰ�еļ�¼
			QString strRecordName = record.fieldName(j);//��

			//if (strRecordName == QLatin1String("departmentID"))
			//{
			//	ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_depNameMap.value(strData)));
			//	continue;
			//}
			if (strRecordName == QLatin1String("status"))
			{
				ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_statusMap.value(strData)));
				continue;
			}
			else if (strRecordName == QLatin1String("online"))
			{
				ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_onlineMap.value(strData)));
				continue;
			}


			ui.tableWidget->setItem(i, j, new QTableWidgetItem(strData));
		}
	}
}

void Aloha_server::onRefresh()
{
	updateTableData(m_employeeID);
}

void Aloha_server::onUDPbroadMsg(QByteArray& btData)
{
	for (quint16 port = gUdpPort; port < gUdpPort + 200; ++port)
	{
		m_udpSender->writeDatagram(btData, btData.size(), QHostAddress::Broadcast, port);
	}
}
void Aloha_server::on_queryIDBtn_clicked()
{
;
	if (!ui.queryIDLineEdit->text().length())
	{
		QMessageBox::information(this,
			QStringLiteral("��ʾ"),
			QStringLiteral("�������û��˺�"));
		ui.queryIDLineEdit->setFocus();
		return;
	}

	//��ȡ�û������Ա��QQ��
	int employeeID = ui.queryIDLineEdit->text().toInt();
	QSqlQuery queryInfo(QString("SELECT * FROM tab_user WHERE userAccount = %1").arg(employeeID));
	queryInfo.exec();
	if (!queryInfo.next())
	{
		QMessageBox::information(this,
			QStringLiteral("��ʾ"),
			QStringLiteral("��������ȷ���û��˺ţ�"));
		ui.queryIDLineEdit->setFocus();
		return;
	}
	else
	{
		m_employeeID = employeeID;
	}
}

void Aloha_server::on_logoutBtn_clicked()
{
	ui.queryIDLineEdit->clear();
	
	//���Ա��QQ���Ƿ�����
	if (!ui.logoutIDLineEdit->text().length())
	{
		QMessageBox::information(this,
			QStringLiteral("��ʾ"),
			QStringLiteral("�������û��˺�"));
		ui.logoutIDLineEdit->setFocus();
		return;
	}

	//��ȡ�û������Ա��QQ��
	int employeeID = ui.logoutIDLineEdit->text().toInt();

	//��������Ա��QQ�źϷ���
	QSqlQuery queryInfo(QString("SELECT userName FROM tab_user WHERE userAccount = %1").arg(employeeID));
	queryInfo.exec();
	if (!queryInfo.next())
	{
		QMessageBox::information(this,
			QStringLiteral("��ʾ"),
			QStringLiteral("��������ȷ���û��˺ţ�"));
		ui.logoutIDLineEdit->setFocus();
		return;
	}
	else
	{
		//ע���������������ݿ����ݣ���Ա����status����Ϊ0
		QSqlQuery sqlUpdate(QString("UPDATE tab_user SET status = 0 WHERE userAccount = %1").arg(employeeID));
		sqlUpdate.exec();

		//��ȡע��Ա��������
		QString strName = queryInfo.value(0).toString();

		QMessageBox::information(this,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("�û� %1 ���˺�:%2 �ѱ�ע����")
			.arg(strName)
			.arg(employeeID));

		ui.logoutIDLineEdit->clear();
	}


}

void Aloha_server::on_selectPictureBtn_clicked()
{
	//��ȡѡ���ͷ��·�� 
	m_pixPath = QFileDialog::getOpenFileName(
		this,
		QString::fromLocal8Bit("ѡ��ͷ��"),
		".",
		"*.png;;*.jpg"
	);

	if (!m_pixPath.size())
	{
		return;
	}

	//��ͷ����ʾ����ǩ 
	QPixmap pixmap;
	pixmap.load(m_pixPath);

	qreal widthRatio = (qreal)ui.headLabel->width() / (qreal)pixmap.width();
	qreal heightRatio = (qreal)ui.headLabel->height() / (qreal)pixmap.height();

	QSize size(pixmap.width() * widthRatio, pixmap.height() * heightRatio);
	ui.headLabel->setPixmap(pixmap.scaled(size));
}

void Aloha_server::on_addBtn_clicked()
{
	//����û�����������
	QString strName = ui.nameLineEdit->text();
	if (!strName.size())
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("�������û�������"));
		ui.nameLineEdit->setFocus();
		return;
	}

	//���Ա��ѡ��ͷ��
	if (!m_pixPath.size())
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("��ѡ���û�ͷ��·����"));
		return;
	}

	//���ݿ�����µ�Ա������
	//��ȡԱ��QQ��
	QSqlQuery maxEmployeeID("SELECT MAX(userAccount) FROM tab_user");
	maxEmployeeID.exec();
	maxEmployeeID.next();

	int employeeID = maxEmployeeID.value(0).toInt() + 1;



	//ͼƬ·����ʽ����Ϊ ��/���滻Ϊ��\��xxx\xxx\xxx.png
	m_pixPath.replace("/", "\\\\");


	QSqlQuery insertSql(QString("INSERT INTO tab_user(userAccount,userName,picture) \
		VALUES(%1,'%2','%3')")
		.arg(employeeID)
		.arg(strName)
		.arg(m_pixPath));

	insertSql.exec();
	QMessageBox::information(this,
		QString::fromLocal8Bit("��ʾ"),
		QString::fromLocal8Bit("�����û��ɹ���"));
	m_pixPath = "";
	ui.nameLineEdit->clear();
	ui.headLabel->setText(QStringLiteral("  Ա������  "));
}
