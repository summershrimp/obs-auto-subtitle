//
// Created by Yibai Zhang on 2020/6/13.
//

#ifndef OBS_AUTO_SUBTITLE_ASRBASE_H
#define OBS_AUTO_SUBTITLE_ASRBASE_H
#include <functional>
#include <QString>
#include <QObject>

typedef std::function<void(QString, int)> ResultCallback;

enum ResultType {
    ResultType_End = 0,
    ResultType_Middle

};

class ASRBase : public QObject {
    Q_OBJECT
public:
    ASRBase(QObject *parent);
    void setResultCallback(ResultCallback cb);
    virtual ~ASRBase(){};

protected:
    ResultCallback getCallback();
signals:
    void sendAudioMessage(const void *, unsigned long);
    void start();
    void stop();

public slots:
    virtual void onStart() = 0;
    virtual void onStop() = 0;
private:
    ResultCallback cb;
};


#endif //OBS_AUTO_SUBTITLE_ASRBASE_H
