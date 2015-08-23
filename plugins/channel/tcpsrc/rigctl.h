#ifndef RIGCTL_H
#define RIGCTL_H

#include <QtCore/QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include "dsp/channelmarker.h"

class RigCtl {
	public:
		RigCtl(ChannelMarker* channel = 0);
		~RigCtl(){};
		qint64 getFreq();
		void setFreq(qint64 freq);
		void requestFreq(qint64 freq);
		int getMode();
		void setMode(int mode);
		void setTunerFreq(qint64 freq) {m_tunerFreq = freq;}
		void setTunerSamples(int samples) {m_samplerate2 = samples / 2;}
	private:
		ChannelMarker* m_channel;
		qint64 m_tunerFreq;
		int m_samplerate2;
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
                RigCtlServer(QObject *parent = 0, RigCtl *rig = 0, unsigned short rigctl_port = RIGCTL_PORT);
		~RigCtlServer(){};
                static const unsigned short RIGCTL_PORT;

        public slots:
                void newConnection(void);

        private:
                QTcpServer *server;
		RigCtl *m_rig;
};
#endif // RIGCTL_H
