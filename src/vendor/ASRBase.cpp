//
// Created by Yibai Zhang on 2020/6/13.
//

#include "ASRBase.h"

ASRBase::ASRBase(QObject *parent) : QObject(parent){
    qRegisterMetaType<ResultCallback>("ResultCallback");
    connect(this, &ASRBase::start, this, &ASRBase::onStart);
    connect(this, &ASRBase::stop, this, &ASRBase::onStop);
}

void ASRBase::setResultCallback(ResultCallback cb) {
    this->cb = std::move(cb);
}

ResultCallback ASRBase::getCallback() {
    return cb;
}