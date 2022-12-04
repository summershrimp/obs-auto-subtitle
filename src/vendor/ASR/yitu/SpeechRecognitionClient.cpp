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

#include "SpeechRecognitionClient.h"


SpeechRecognitionClient::SpeechRecognitionClient(std::string endpoint, std::string x_api_key) {
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
    stub_ = SpeechRecognition::NewStub(channel);
    apiKey = x_api_key;
}

void SpeechRecognitionClient::start(){
    grpc::ClientContext context;
    context.AddMetadata("x-api-key", apiKey);
    readwriter_ = stub_->RecognizeStream(&context);
    readthread = std::thread(SpeechRecognitionClient::readerLoop, this);
    StreamingSpeechRequest req;

    auto config = req.mutable_streamingspeechconfig();

    config->mutable_audioconfig()->set_aue(AudioConfig_AudioEncoding_PCM);
    config->mutable_audioconfig()->set_samplerate(16000);
   
    config->mutable_speechconfig()->set_scene(SpeechConfig_Scene_GENERALSCENE);
    if(lang == "cn") {
        config->mutable_speechconfig()->set_lang(SpeechConfig_Language_MANDARIN);
    } else if (lang == "en") {
        config->mutable_speechconfig()->set_lang(SpeechConfig_Language_ENGLISH);
    } else {
        config->mutable_speechconfig()->set_lang(SpeechConfig_Language_UNSPECIFIED);
    }
    config->mutable_speechconfig()->set_disablepunctuation(!punc);
    config->mutable_speechconfig()->set_disableconvertnumber(!noa);

    readwriter_->Write(req);
    running = true;
}

void SpeechRecognitionClient::readerLoop(SpeechRecognitionClient *that) {
    StreamingSpeechResponse resp;
    while (that->readwriter_->Read(&resp)) {
        if(resp.has_status()){
            continue;
        } else {
            auto result = resp.result();
            auto text = result.besttranscription().transcribedtext();
            auto isFinal = result.isfinal();
            if(that->callback){
                that->callback(text, isFinal);
            }
        }
    }
}

void SpeechRecognitionClient::sendBuffer(const char *buffer, int size) {
    StreamingSpeechRequest req;
    req.set_audiodata(buffer, size);
    readwriter_->Write(req);
}

void SpeechRecognitionClient::stop() {
    if(running) {
        running = false;
        readwriter_->WritesDone();
        readthread.join();
    }
}

void SpeechRecognitionClient::setLang(std::string _lang) {
    lang = _lang;
}

void SpeechRecognitionClient::setEnablePunc(bool enable) {
    punc = enable;
}

void SpeechRecognitionClient::setEnableNOA(bool enable) {
    noa = enable;
}

void SpeechRecognitionClient::setCallback(std::function<void(std::string, bool)> cb) {
    callback = cb;
}

SpeechRecognitionClient::~SpeechRecognitionClient(){
    stop();
}