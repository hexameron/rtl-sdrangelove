/*
 * Server for hamlib's TCP rigctl protocol.
 * Copyright (C) 2010 Adam Sampson <ats@offog.org>
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *
 * Modified for QtRadio by Glenn VE9GJ Sept 29 2011
 * Taken from sdr-shell project
 *
 * (C) 2015 John Greb.
 *
 */

#include "rigctl.h"
#include <string>
#include <vector>

RigCtlSocket::RigCtlSocket(QObject *parent, RigCtl *rig, QTcpSocket *conn)
        : QObject(parent) {
	this->conn = conn;
	this->m_rig = rig;
}

void RigCtlSocket::disconnected() {
        deleteLater();
}

void RigCtlSocket::readyRead() {
        if (!conn->canReadLine()) {
                return;
        }

        QByteArray command(conn->readLine());
        // removed command.chop(1); because it is unable to cope with
        // with more than one character line terminations
        // i.e. telnet client uses a CR/LF sequence
        command = command.simplified();
        if (command.size() == 0) {
                command.append("?");
        }

        QString cmdstr = command.constData();
        QStringList cmdlist = cmdstr.split(QRegExp("\\s+"));
        int cmdlistcnt = cmdlist.count();
        bool output = false;
        int retcode = 0; //RIG_OK
        QTextStream out(conn);

        /* This isn't a full implementation of the rigctl protocol; it's
           essentially just enough to keep fldigi happy.  (And not very happy
           at that; you will need to up the delays to stop it sending a
           command, ignoring the reply owing to a timeout, then getting
           confused the next time it sends a command because the old reply is
           already in the buffer.)

           Not implemented but used by fldigi:
             v           get_vfo  -fixed
             F 1.234     set_freq -fixed
         */

        if (command[0] == 'f') { // get_freq
            out << QString::number((double)m_rig->getFreq(),'e', 8) << "\n";
            output = true;
        } else if(cmdlist[0].compare("F") == 0 && cmdlistcnt == 2) { // set_freq
            QString newf = cmdlist[1];
            m_rig->requestFreq((qint64)atof(newf.toUtf8()));
        } else if (command[0] == 'v') { // get_vfo
            out  << "VFOA\n";
	    output = true;
        } else if (command[0] == 'V') { // set_VFO
            QString cmd = command.constData();
            if ( cmd.contains("VFOA")){
               //main->rigctlSetVFOA();
            }
            if ( cmd.contains("VFOB")){
               //main->rigctlSetVFOB();
            }
        } else if (command[0] == 'j') { // get_rit
            out << "0" << "\n";
            output = true;
	} else if (cmdlist[0][0] == 'M') { // Set_mode
	    if (cmdlist[1][0]=='U')
		m_rig->setMode(0);
	    if (cmdlist[1][0]=='F')
		m_rig->setMode(1);
	} else if (command[0] == 'm') { // get_mode
	    if (m_rig->getMode())
		out << "FM\n";
	    else
		out << "USB\n";
	    out << "3300\n";
	    output = true;
        } else if (command[0] == 's') { // get_split_vfo
            // simple "we don't do split" response

            // TODO - if split is selected then VFOS will be returned
            // which is invalid. This needs to be '1\n<Tx-VFO>' if split is
            // enabled and the Tx-VFO probably needs to be VFOB
            out << "0" << "\n";
            // if split is to be supported then this needs to be the
            // Tx VFO and other split functions will probably need to
            // be implemented
            output = true;
	} else if (command[0] == '\\' || command[0] == '1') {
            // See dump_state in rigctl_parse.c for what this means.
            out << "0\n"; // protocol version
            out << "2" << "\n"; //RIG_MODEL_NETRIGCTL
            out << "1" << "\n"; //RIG_ITU_REGION1
            // Not sure exactly what to send here but this seems to work
            out << "150000.000000 30000000.000000  0x900af -1 -1 0x10000003 0x3\n"; //("%"FREQFMT" %"FREQFMT" 0x%x %d %d 0x%x 0x%x\n",start,end,modes,low_power,high_power,vfo,ant)
            out << "0 0 0 0 0 0 0\n";
            out << "150000.000000 30000000.000000  0x900af -1 -1 0x10000003 0x3\n"; //TX
            out << "0 0 0 0 0 0 0\n";
            out << "0 0\n";
            out << "0 0\n";
            out << "0\n";
            out << "0\n";
            out << "0\n";
            out << "0\n";
            out << "\n";
            out << "\n";
            out << "0x0\n";
            out << "0x0\n";
            out << "0x0\n";
            out << "0x0\n";
            out << "0x0\n";
            out << "0\n";
            output = true;
        } else {
            qDebug("rigctl: unknown command \"%s\"\n", command.constData());
            retcode = -11;
        }
            qDebug("rigctl:  command \"%s\"\n", command.constData());
        if (!output) {
                out << "RPRT " << retcode << "\n";
        }
}

const unsigned short RigCtlServer::RIGCTL_PORT(19999);

RigCtl::RigCtl(ChannelMarker* channel) {
	m_channel = channel;
	m_tunerFreq = 1.02e8;
	m_samplerate2 = 96000;
}

qint64 RigCtl::getFreq() {
	return m_freq;
}

void RigCtl::requestFreq(qint64 freq) {
	qint64 newfreq;

	newfreq = freq - m_tunerFreq;
	if (newfreq - m_samplerate2 > 0)
		newfreq = (qint64)m_samplerate2;
	else if (newfreq + m_samplerate2 < 0)
		newfreq = -(qint64)m_samplerate2;

	m_freq = newfreq + m_tunerFreq;
	m_channel->setCenterFrequency(newfreq);
}

void RigCtl::setFreq(qint64 freq) {
	m_freq = m_tunerFreq + freq;
}

int RigCtl::getMode() {
	return m_mode;
}

void RigCtl::setMode(int mode) {
	m_mode = mode;
}

void RigCtl::setTunerFreq(qint64 freq) {
	m_freq += freq - m_tunerFreq;
	m_tunerFreq = freq;
}

void RigCtl::setTunerSamples(int samples) {
	m_samplerate2 = samples / 2;
}

RigCtlServer::RigCtlServer(QObject *parent, RigCtl *rig,  unsigned short rigctl_port)
	: QObject(parent) {
	bool result;
	unsigned short port;

	port = rigctl_port;
	m_rig = rig;
	server = new QTcpServer(this);
	do {
		result = server->listen(QHostAddress::Any, port);
	} while (!result && (port-- > rigctl_port - 5));
	if (!result) {
		qCritical("rigctl: failed to bind socket\n");
		return;
	}
	qCritical("rigctl: Listening on port %d\n", port);
	connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

void RigCtlServer::newConnection() {
	QTcpSocket *conn = server->nextPendingConnection();
        RigCtlSocket *sock = new RigCtlSocket(this, m_rig, conn);
        connect(conn, SIGNAL(disconnected()), conn, SLOT(deleteLater()));
        connect(conn, SIGNAL(disconnected()), sock, SLOT(disconnected()));
        connect(conn, SIGNAL(readyRead()), sock, SLOT(readyRead()));
}

