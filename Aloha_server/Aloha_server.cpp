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
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("连接数据库失败！"));
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
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//表格只读
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

	//收到tcp客户端发来的信息后进行udp广播
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
	db.setDatabaseName("aloha");	//数据库名称
	db.setHostName("124.71.111.16");//主机名
	db.setUserName("root");		//用户名
	db.setPassword("123456");	//密码
	db.setPort(3306);			//端口

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
	m_statusMap.insert(QStringLiteral("1"), QStringLiteral("有效"));
	m_statusMap.insert(QStringLiteral("0"), QStringLiteral("已注销"));
}

void Aloha_server::setOnlineMap()
{
	m_onlineMap.insert(QStringLiteral("1"), QStringLiteral("离线"));
	m_onlineMap.insert(QStringLiteral("2"), QStringLiteral("在线"));
	m_onlineMap.insert(QStringLiteral("3"), QStringLiteral("隐身"));
}

void Aloha_server::updateTableData(int employeeID)
{
	ui.tableWidget->clear();

	//if (depID && depID != m_compDepID)//查询部门
	//{
	//	m_queryInfoModel.setQuery(QString("SELECT * FROM tab_employees WHERE departmentID = %1").arg(depID));
	//}
	if (employeeID)//精确查找
	{
		m_queryInfoModel.setQuery(QString("SELECT * FROM tab_user WHERE userAccount = %1").arg(employeeID));
	}
	else//查询所有
	{
		m_queryInfoModel.setQuery(QString("SELECT * FROM tab_user"));
	}

	int rows = m_queryInfoModel.rowCount();			//总行数（总记录数）
	int columns = m_queryInfoModel.columnCount();	//总列数（总字段数）

	QModelIndex index;//模型索引

	//设置表格的行数、列
	ui.tableWidget->setRowCount(rows);
	ui.tableWidget->setColumnCount(columns);

	//设置表头
	QStringList headers;
	headers 
		<< QStringLiteral("用户账号")
		<< QStringLiteral("用户昵称")
		<< QStringLiteral("用户签名")
		<< QStringLiteral("用户状态")
		<< QStringLiteral("用户头像")
		<< QStringLiteral("在线状态");
	ui.tableWidget->setHorizontalHeaderLabels(headers);

	//设置列等宽
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			index = m_queryInfoModel.index(i, j);//行、列
			QString strData = m_queryInfoModel.data(index).toString();//获取i行j列的数据

			//获取字段名称
			QSqlRecord record = m_queryInfoModel.record(i);//当前行的记录
			QString strRecordName = record.fieldName(j);//列

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
			QStringLiteral("提示"),
			QStringLiteral("请输入用户账号"));
		ui.queryIDLineEdit->setFocus();
		return;
	}

	//获取用户输入的员工QQ号
	int employeeID = ui.queryIDLineEdit->text().toInt();
	QSqlQuery queryInfo(QString("SELECT * FROM tab_user WHERE userAccount = %1").arg(employeeID));
	queryInfo.exec();
	if (!queryInfo.next())
	{
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("请输入正确的用户账号！"));
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
	
	//检测员工QQ号是否输入
	if (!ui.logoutIDLineEdit->text().length())
	{
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("请输入用户账号"));
		ui.logoutIDLineEdit->setFocus();
		return;
	}

	//获取用户输入的员工QQ号
	int employeeID = ui.logoutIDLineEdit->text().toInt();

	//检测输入的员工QQ号合法性
	QSqlQuery queryInfo(QString("SELECT userName FROM tab_user WHERE userAccount = %1").arg(employeeID));
	queryInfo.exec();
	if (!queryInfo.next())
	{
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("请输入正确的用户账号！"));
		ui.logoutIDLineEdit->setFocus();
		return;
	}
	else
	{
		//注销操作，更新数据库数据，将员工的status设置为0
		QSqlQuery sqlUpdate(QString("UPDATE tab_user SET status = 0 WHERE userAccount = %1").arg(employeeID));
		sqlUpdate.exec();

		//获取注销员工的姓名
		QString strName = queryInfo.value(0).toString();

		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("用户 %1 的账号:%2 已被注销！")
			.arg(strName)
			.arg(employeeID));

		ui.logoutIDLineEdit->clear();
	}


}

void Aloha_server::on_selectPictureBtn_clicked()
{
	//获取选择的头像路径 
	m_pixPath = QFileDialog::getOpenFileName(
		this,
		QString::fromLocal8Bit("选择头像"),
		".",
		"*.png;;*.jpg"
	);

	if (!m_pixPath.size())
	{
		return;
	}

	//将头像显示到标签 
	QPixmap pixmap;
	pixmap.load(m_pixPath);

	qreal widthRatio = (qreal)ui.headLabel->width() / (qreal)pixmap.width();
	qreal heightRatio = (qreal)ui.headLabel->height() / (qreal)pixmap.height();

	QSize size(pixmap.width() * widthRatio, pixmap.height() * heightRatio);
	ui.headLabel->setPixmap(pixmap.scaled(size));
}

void Aloha_server::on_addBtn_clicked()
{
	//检测用户姓名的输入
	QString strName = ui.nameLineEdit->text();
	if (!strName.size())
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("请输入用户姓名！"));
		ui.nameLineEdit->setFocus();
		return;
	}

	//检测员工选择头像
	if (!m_pixPath.size())
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("请选择用户头像路径！"));
		return;
	}

	//数据库插入新的员工数据
	//获取员工QQ号
	QSqlQuery maxEmployeeID("SELECT MAX(userAccount) FROM tab_user");
	maxEmployeeID.exec();
	maxEmployeeID.next();

	int employeeID = maxEmployeeID.value(0).toInt() + 1;



	//图片路径格式设置为 “/”替换为“\”xxx\xxx\xxx.png
	m_pixPath.replace("/", "\\\\");


	QSqlQuery insertSql(QString("INSERT INTO tab_user(userAccount,userName,picture) \
		VALUES(%1,'%2','%3')")
		.arg(employeeID)
		.arg(strName)
		.arg(m_pixPath));

	insertSql.exec();
	QMessageBox::information(this,
		QString::fromLocal8Bit("提示"),
		QString::fromLocal8Bit("新增用户成功！"));
	m_pixPath = "";
	ui.nameLineEdit->clear();
	ui.headLabel->setText(QStringLiteral("  员工寸照  "));
}
