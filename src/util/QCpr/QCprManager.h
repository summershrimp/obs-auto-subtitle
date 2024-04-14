/*
QCprManager
 Copyright (C) 2019-2024 Yibai Zhang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; If not, see <https://www.gnu.org/licenses/>
*/
#ifndef _QCPR_MANAGER_H_
#define _QCPR_MANAGER_H_

#include <QThread>
#include <cpr/cpr.h>

class QCprManager : public QThread {
	Q_OBJECT

public:
	QCprManager(cpr::AsyncResponse &&fr, QObject *parent = nullptr);

private:
	void run() override;
	cpr::AsyncResponse fr;

signals:
	void resultReady(const cpr::Response r);
};

#endif
