#ifndef RIGCTL_H
#define RIGCTL_H

#include <QtCore/QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

class RigCtl {
	public:
		RigCtl(){};
		~RigCtl(){};
		double getFreq();
		void setFreq(double freq);
		int getMode();
		void setMode(int);

	private:
		double m_freq;
		int m_mode;
};

class RigCtlSocket : public QObject {
        Q_OBJECT

        public:
                RigCtlSocket(QObject *parent = 0, RigCtl *rig = 0, QTcpSocket *conn = 0);
		~RigCtlSocket(){};

        public slots:
                void disconnected(void);
                void readyRead(void);

        private:
                QTcpSocket *conn;
		RigCtl *m_rig;
};

class RigCtlServer : public QObject {
        Q_OBJECT

        public:
                RigCtlServer(QObject *parent = 0, unsigned short rigctl_port = RIGCTL_PORT);
		~RigCtlServer(){};
		RigCtl* getRig();
                static const unsigned short RIGCTL_PORT;

        public slots:
                void newConnection(void);

        private:
                QTcpServer *server;
		RigCtl *m_rig;
};
#endif // RIGCTL_H
