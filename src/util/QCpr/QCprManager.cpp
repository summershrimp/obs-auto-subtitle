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
#include "QCprManager.h"

QCprManager::QCprManager(cpr::AsyncResponse &&fr, QObject *parent)
	: QThread(parent), fr(std::move(fr))
{
}

void QCprManager::run()
{
	fr.wait(); // This waits until the request is complete
	cpr::Response r =
		fr.get(); // Since the request is complete, this returns immediately
	emit resultReady(r);
}
