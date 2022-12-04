/*
obs-auto-subtitle
 Copyright (C) 2019-2020 Yibai Zhang

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

#include "src/yitu_streaming.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <thread>

class SpeechRecognitionClient {
public:
    SpeechRecognitionClient(std::string endpoint, std::string x_api_key);
    void start();
    static void readerLoop(SpeechRecognitionClient *that);
    void sendBuffer(const char *buffer, int size);
    void stop();
    void setLang(std::string _lang);
    void setEnablePunc(bool enable);
    void setEnableNOA(bool enable);
    void setCallback(std::function<void(std::string, bool)> cb);
    ~SpeechRecognitionClient();
private:
    std::unique_ptr<SpeechRecognition::Stub> stub_;
    std::unique_ptr< ::grpc::ClientReaderWriter< ::StreamingSpeechRequest, ::StreamingSpeechResponse>> readwriter_;
    std::string apiKey ;
    std::thread readthread;
    std::string lang = "";
    std::function<void(std::string, bool)> callback;
    bool running = true;
    bool punc = true;
    bool noa = true;
};